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
// #include <unistd.h>
#include <stdlib.h>
#define REMOTE_SERVER_IP "0.0.0.0"
#define REMOTE_SERVER_PORT 5000
#define BUFFER_SIZE 256
#define BACKLOG_SIZE 5
int main()
{
	int server_socket, client_socket;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	char buffer[BUFFER_SIZE] = {0};
	// 1. create local socket
	server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	// configure server address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(REMOTE_SERVER_PORT);
	if (bind(server_socket, &server_addr, sizeof(struct sockaddr_in)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	};
	while (1)
	{

		// listen for clients
		if (listen(server_socket, BACKLOG_SIZE) < 0)
		{
			perror("connection failed");
			exit(EXIT_FAILURE);
		}
		client_socket = accept(server_socket, (struct sockaddr *)&client_addr_len,
							   &client_addr_len);
		if (client_socket < 0)
		{
			perror("accept failed");
			exit(EXIT_FAILURE);
		}
		int bytes = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
		if (bytes < 0)
		{
			perror("receive failed");
			exit(EXIT_FAILURE);
		}
		buffer[bytes] = '\0';
		printf("Message from Client: %s\n", buffer);
		char *reply = strcat("You sent: ", buffer);
		if (reply == NULL)
		{
			perror("reply concat failed");
			exit(EXIT_FAILURE);
		}
		if (send(client_socket, reply, strlen(reply), 0) < 0)
		{
			perror("reply send failed");
			exit(EXIT_FAILURE);
		};
	}
}

// socket -> connect -> send and receive -> close
