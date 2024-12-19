#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <curl/curl.h>
#include "../protocol/protocol.h"

#define PORT 8080
#define BUFFER_SIZE 4096
#define JSON_BUFFER_SIZE 4096
#define MAX_CLIENTS 10
#define USERNAME_SIZE 32

struct client_info
{
    int socket;
    char username[USERNAME_SIZE];
    int active;
};

void broadcast_message(struct Packet *packet, int sender_socket);
void send_private_message(struct Packet *packet);
void send_to_flask(const char *message);
void notify_flask_disconnect(const char *username);

struct client_info clients[MAX_CLIENTS] = {0}; // Initialize array of clients
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void send_to_flask(const char *message)
{
    CURL *curl = curl_easy_init();
    if (curl)
    {
        const char *flask_url = "http://127.0.0.1:5000/api/message";

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, flask_url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message);

        char error_buffer[CURL_ERROR_SIZE];
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "Flask request failed: %s\n", error_buffer);
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

void broadcast_message(struct Packet *packet, int sender_socket)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].socket != 0 &&
            clients[i].active &&
            clients[i].socket != sender_socket)
        {
            sendPacket(clients[i].socket, packet);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}
    
void send_private_message(struct Packet *packet) {
    pthread_mutex_lock(&clients_mutex);
    
    // Send to recipient
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0 && 
            clients[i].active &&
            (strcmp(clients[i].username, packet->dest) == 0 ||    // Send to recipient
             strcmp(clients[i].username, packet->sender) == 0))   // Send back to sender
        {
            sendPacket(clients[i].socket, packet);
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
    
    // Send to Flask server
    char json_buffer[JSON_BUFFER_SIZE];
    snprintf(json_buffer, JSON_BUFFER_SIZE,
        "{\"message\": \"%s\", \"username\": \"%s\", \"header\": \"%s\", \"dest\": \"%s\", \"isPrivate\": true}",
        packet->data,
        packet->sender,
        packet->header,
        packet->dest);
    
    send_to_flask(json_buffer);
}

void notify_flask_disconnect(const char *username)
{
    CURL *curl = curl_easy_init();
    if (curl)
    {
        const char *flask_url = "http://127.0.0.1:5000/api/disconnect";
        char json_payload[256];
        snprintf(json_payload, sizeof(json_payload), "{\"username\": \"%s\"}", username);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, flask_url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);

        curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

void *handle_client(void *arg)
{
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE] = {0};
    struct Packet packet;

    // Add client to tracking array
    pthread_mutex_lock(&clients_mutex);
    int client_index = -1;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].socket == 0)
        {
            clients[i].socket = client_socket;
            clients[i].active = 1;
            client_index = i;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    if (client_index == -1)
    {
        printf("No room for new client\n");
        close(client_socket);
        free(arg);
        return NULL;
    }

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(client_socket, buffer, BUFFER_SIZE);

        if (valread > 0)
        {
            if (strstr(buffer, "POST /api/message") == buffer)
            {
                char *body = strstr(buffer, "\r\n\r\n");
                if (body)
                {
                    body += 4;
                    printf("Received message: %s\n", body);

                    send_to_flask(body);

                    const char *response = "HTTP/1.1 200 OK\r\n"
                                           "Content-Type: application/json\r\n"
                                           "Connection: keep-alive\r\n"
                                           "Content-Length: 29\r\n"
                                           "\r\n"
                                           "{\"status\":\"message received\"}";

                    send(client_socket, response, strlen(response), 0);

                    pthread_mutex_lock(&clients_mutex);
                    for (int i = 0; i < MAX_CLIENTS; i++)
                    {
                        if (clients[i].socket != 0 &&
                            clients[i].active &&
                            clients[i].socket != client_socket)
                        {
                            char broadcast_response[BUFFER_SIZE];
                            snprintf(broadcast_response, sizeof(broadcast_response),
                                     "HTTP/1.1 200 OK\r\n"
                                     "Content-Type: application/json\r\n"
                                     "Content-Length: %zu\r\n"
                                     "\r\n"
                                     "%s",
                                     strlen(body), body);

                            send(clients[i].socket, broadcast_response, strlen(broadcast_response), 0);
                        }
                    }
                    pthread_mutex_unlock(&clients_mutex);
                }
            }
            if(memcpy(&packet, buffer, sizeof(struct Packet)) && (strcmp(packet.header, MSG_TYPE_PRIVATE) == 0)) {
                if (strcmp(packet.header, MSG_TYPE_PRIVATE) == 0)
                {
                    send_private_message(&packet);

                    // JSON for private message
                    char json_buffer[JSON_BUFFER_SIZE];
                    int json_len = snprintf(json_buffer, JSON_BUFFER_SIZE,
                                            "{\"message\": \"%s\", \"username\": \"%s\", \"header\": \"%s\", \"dest\": \"%s\", \"isPrivate\": true}",
                                            packet.data,
                                            packet.sender,
                                            packet.header,
                                            packet.dest);

                    if (json_len >= JSON_BUFFER_SIZE)
                    {
                        fprintf(stderr, "JSON buffer overflow\n");
                        continue;
                    }
                    send_to_flask(json_buffer);
            
            }
            else
            {
                broadcast_message(&packet, client_socket);

                // JSON for regular message
                char json_buffer[JSON_BUFFER_SIZE];
                int json_len = snprintf(json_buffer, JSON_BUFFER_SIZE,
                                        "{\"message\": \"%s\", \"username\": \"%s\", \"header\": \"%s\"}",
                                        packet.data,
                                        packet.sender,
                                        packet.header);

                if (json_len >= JSON_BUFFER_SIZE)
                {
                    fprintf(stderr, "JSON buffer overflow\n");
                    continue;
                }
                send_to_flask(json_buffer);
            }
        }
    }
    else if (valread == 0)
    {
        printf("Client disconnected.\n");
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i].socket == client_socket)
            {
                notify_flask_disconnect(clients[i].username);
                clients[i].socket = 0;
                clients[i].active = 0;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
        break;
    }
    else
    {
        perror("Read failed");
        break;
    }
}

close(client_socket);
free(arg);
return NULL;
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].socket = 0;
        clients[i].active = 0;
        memset(clients[i].username, 0, USERNAME_SIZE);
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is running...\n");

    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Accept failed");
            continue;
        }

        int *client_sock = malloc(sizeof(int));
        *client_sock = new_socket;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)client_sock) < 0)
        {
            perror("Could not create thread");
            free(client_sock);
            close(new_socket);
            continue;
        }

        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}