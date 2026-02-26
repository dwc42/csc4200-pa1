#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif
#include <string.h>
#include <stdio.h>
// #include <unistd.h>
#include <stdlib.h>
#define REMOTE_SERVER_IP "0.0.0.0"
#define REMOTE_SERVER_PORT 5000
#define BUFFER_SIZE 256
int main()
{
	int local_socket;
	struct sockaddr_in server_addr;
	struct sockaddr_in local_addr;
	socklen_t client_addr_len = sizeof(local_addr);
	char buffer[BUFFER_SIZE] = {0};
	// 1. create local socket
	local_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (local_socket < 0)
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
	if (connect(local_socket, (struct sockaddr_in *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("connection failed");
		exit(EXIT_FAILURE);
	}
	printf("Success: Connected to server at %s:%d\n", REMOTE_SERVER_IP, REMOTE_SERVER_PORT);
	char message[] = "Hello Server";
	if (send(local_socket, message, strlen(message), 0) < 0)
	{
		perror("send failed");
		close(local_socket);
		exit(EXIT_FAILURE);
	}
	printf("Message sent to server: %s\n", message);
	int bytes = recv(local_socket, buffer, BUFFER_SIZE - 1, 0);
	if (bytes < 0)
	{
		perror("receive failed");
		exit(EXIT_FAILURE);
	}
	buffer[bytes] = '\0';
	printf("Response from server: %s\n", buffer);
	close(local_socket);
	printf("Success: completed\n");
	exit(EXIT_SUCCESS);
}

// socket -> connect -> send and receive -> close
