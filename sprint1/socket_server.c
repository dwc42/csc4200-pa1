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
#define REMOTE_SERVER_IP "10.128.0.3"
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
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	// configure server address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(REMOTE_SERVER_PORT);
	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	};
	// listen for clients
	if (listen(server_socket, BACKLOG_SIZE) < 0)
	{
		perror("listen failed");
		exit(EXIT_FAILURE);
	}
	printf("Server listening on port %d...\n", REMOTE_SERVER_PORT);
	fflush(stdout);
	while (1)
	{
		client_socket = accept(server_socket, (struct sockaddr *)&client_addr,
							   &client_addr_len);
		if (client_socket < 0)
		{
			perror("accept failed");
			exit(EXIT_FAILURE);
		}
		int bytes = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
		if (bytes == 0)
		{
			perror("client disconnected");
			exit(EXIT_FAILURE);
		}
		else if (bytes < 0)
		{
			perror("receive failed");
			exit(EXIT_FAILURE);
		}
		buffer[bytes] = '\0';
		printf("Message from Client: %s, %d bytes\n", buffer, bytes);
		char reply[BUFFER_SIZE + 16];
		snprintf(reply, sizeof(reply), "You sent: %s", buffer);
		if (send(client_socket, reply, strlen(reply), 0) < 0)
		{
			perror("reply send failed");
			exit(EXIT_FAILURE);
		};
		printf("Returned Message to client: %s, %d bytes\n", buffer, strlen(reply) - 1);
	}
}

// socket -> connect -> send and receive -> close
