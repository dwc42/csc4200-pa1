
#ifndef SOCKET_UTILITIES_H_
#define SOCKET_UTILITIES_H_
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PROTOCOL_NUMBER 17
#define MESSAGE_TYPE 1
#define PAYLOAD_CHUNK_SIZE 12

typedef struct PacketHeader
{
	unsigned version;
	unsigned messageType;
	unsigned messageLength;
} PacketHeader;
typedef struct Packet
{
	PacketHeader header;
	char *payload;
} Packet;
int sendPacket(int socket, Packet *packet);
Packet receivePacket(int socket);
#endif
