#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string.h>
#include <sys/socket.h>

#define HEADER_SIZE 8
#define DATA_SIZE 2048
#define MSG_TYPE_TEXT "TEXT"
#define MSG_TYPE_PRIVATE "PRIVATE"

struct Packet {
    char header[HEADER_SIZE];  // Message type (e.g., "TEXT", "PRIVATE")
    char data[DATA_SIZE];      // Message payload
    char sender[32];           // Added for sender identification
    char dest[32];            // Added for private message recipient
};


struct Packet createMessagePacket(const char* message, const char* sender);
struct Packet createPrivatePacket(const char* message, const char* sender, const char* dest);
void sendPacket(int socket_fd, const struct Packet* packet);
struct Packet receivePacket(int socket_fd);

#endif