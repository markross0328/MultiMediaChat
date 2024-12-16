#include "protocol.h"

struct Packet createMessagePacket(const char* message, const char* sender) {
    struct Packet packet;
    memset(&packet, 0, sizeof(struct Packet));
    
    strncpy(packet.header, MSG_TYPE_TEXT, HEADER_SIZE - 1);
    strncpy(packet.data, message, DATA_SIZE - 1);
    strncpy(packet.sender, sender, 31);
    
    return packet;
}

struct Packet createPrivatePacket(const char* message, const char* sender, const char* dest) {
    struct Packet packet;
    memset(&packet, 0, sizeof(struct Packet));
    
    strncpy(packet.header, MSG_TYPE_PRIVATE, HEADER_SIZE - 1);
    strncpy(packet.data, message, DATA_SIZE - 1);
    strncpy(packet.sender, sender, 31);
    strncpy(packet.dest, dest, 31);
    
    return packet;
}

void sendPacket(int socket_fd, const struct Packet* packet) {
    send(socket_fd, packet, sizeof(struct Packet), 0);
}

struct Packet receivePacket(int socket_fd) {
    struct Packet packet;
    memset(&packet, 0, sizeof(struct Packet));
    recv(socket_fd, &packet, sizeof(struct Packet), 0);
    return packet;
}