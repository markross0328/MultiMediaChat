#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h> 
#include "../protocol/protocol.h"

#define PORT 8080
#define BUFFER_SIZE 4096
#define MESSAGE_SIZE 2048
#define USERNAME_SIZE 32

// Function declaration before main
void *receive_messages(void *socket_desc);

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char message[MESSAGE_SIZE] = {0};
    char username[USERNAME_SIZE] = {0};

    // Get username
    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        perror("Invalid address");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection failed");
        return -1;
    }

    printf("Connected to server. Type /msg <username> <message> for private messages\n");
    // Create a separate thread for receiving messages
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, (void *)&sock) < 0)
    {
        perror("Could not create receive thread");
        return -1;
    }

    while (1)
    {
        printf("You: ");
        fflush(stdout);

        if (!fgets(message, sizeof(message), stdin))
        {
            break;
        }

        message[strcspn(message, "\n")] = 0;

        if (strlen(message) == 0)
        {
            continue;
        }

        if (strncmp(message, "/msg", 4) == 0)
        {
            char dest[USERNAME_SIZE];
            char private_msg[MESSAGE_SIZE];
            if (sscanf(message + 4, " %s %[^\n]", dest, private_msg) == 2)
            {
                struct Packet packet = createPrivatePacket(private_msg, username, dest);
                sendPacket(sock, &packet);
            }
        }
        else
        {
            struct Packet packet = createMessagePacket(message, username);
            sendPacket(sock, &packet);
        }
    }

    close(sock);
    return 0;
}

void *receive_messages(void *socket_desc)
{
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE] = {0};
    struct Packet packet;

    while (1)
    {
        memset(&packet, 0, sizeof(struct Packet));
        int valread = recv(sock, &packet, sizeof(struct Packet), 0);

        if (valread > 0)
        {
            if (strcmp(packet.header, MSG_TYPE_PRIVATE) == 0)
            {
                printf("\rPrivate from %s: %s\nYou: ", packet.sender, packet.data);
            }
            else
            {
                printf("\r%s: %s\nYou: ", packet.sender, packet.data);
            }
            fflush(stdout);
        }
        else if (valread == 0)
        {
            printf("\nDisconnected from server\n");
            break;
        }
        else
        {
            perror("Read failed");
            break;
        }
    }

    close(sock);
    pthread_exit(NULL);
}