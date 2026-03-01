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

#include "socket_utilities.h"

#define REMOTE_SERVER_IP "10.128.0.3"
#define REMOTE_SERVER_PORT 5000
#define BUFFER_SIZE 256
int main()
{
	int server_socket;
	struct sockaddr_in server_addr;
	struct sockaddr_in local_addr;
	socklen_t client_addr_len = sizeof(local_addr);
	char buffer[BUFFER_SIZE] = {0};
	// 1. create local socket
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	// configure server address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(REMOTE_SERVER_PORT);
	inet_pton(AF_INET, REMOTE_SERVER_IP, &server_addr.sin_addr);
	// connect to server
	if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("connection failed");
		exit(EXIT_FAILURE);
	}
	printf("Success: Connected to server at %s:%d\n", REMOTE_SERVER_IP, REMOTE_SERVER_PORT);
	char *message = "hello server";
	Packet packet;
	packet.header.version = PROTOCOL_NUMBER;
	packet.header.messageType = MESSAGE_TYPE;
	packet.header.messageLength = strlen(message);
	packet.payload = message;
	if (sendPacket(server_socket, &packet) < 0)
	{
		perror("return packet send failed");
		exit(EXIT_FAILURE);
	};
	printf("packet sent to server: (header: (version: %u, type: %u, length: %u), payload: %s)\n",
		   packet.header.version,
		   packet.header.messageType,
		   packet.header.messageLength,
		   packet.payload);
	Packet packetFromServer = receivePacket(server_socket);
	if (packetFromServer.payload == NULL)
	{
		perror("receive packet failed");
		exit(EXIT_FAILURE);
	}
	printf("packet received from server: (header: (version: %u, type: %u, length: %u), payload: %s)\n",
		   packetFromServer.header.version,
		   packetFromServer.header.messageType,
		   packetFromServer.header.messageLength,
		   packetFromServer.payload);

	close(server_socket);
	printf("Success: completed\n");
	exit(EXIT_SUCCESS);
}

// socket -> connect -> send and receive -> close
