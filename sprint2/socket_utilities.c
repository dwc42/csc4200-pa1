#include "socket_utilities.h"
#include "math.h"
int sendPacket(int socket, Packet *packet)
{
	PacketHeader header = packet->header;
	char sendHeaderBuffer[PAYLOAD_CHUNK_SIZE];

	int i = 0;
	memset(&sendHeaderBuffer + i++ * 4, htonl(header.version), sizeof(unsigned));
	memset(&sendHeaderBuffer + i++ * 4, htonl(header.messageType), sizeof(unsigned));
	memset(&sendHeaderBuffer + i++ * 4, htonl(header.messageLength), sizeof(unsigned));

	if (send(socket, sendHeaderBuffer, sizeof(sendHeaderBuffer), 0) < 0)
	{
		return -1;
	}
	char sendBuffer[PAYLOAD_CHUNK_SIZE];
	unsigned bigBuffer;
	char *ptr = packet->payload;
	for (unsigned i = 0; i < header.messageLength; i++, *ptr++)
	{
		if (((i % 4) == 3) || i == header.messageLength - 1)
		{
			memcpy(&bigBuffer, ptr, min(4, i - header.messageLength - 1));
			bigBuffer = htonl(bigBuffer);
			memcpy(&sendBuffer + (i % 4) * 4, &bigBuffer, min(4, i - header.messageLength - 1));
			if ((i % PAYLOAD_CHUNK_SIZE) == PAYLOAD_CHUNK_SIZE - 1)
			{
				if (send(socket, sendBuffer, PAYLOAD_CHUNK_SIZE, 0) < 0)
				{
					return -1;
				}
			}
		}
	}
	return 1;
}
Packet receivePacket(int socket)
{
	char buffer[PAYLOAD_CHUNK_SIZE + 1];

	int bytes = recv(socket, buffer, PAYLOAD_CHUNK_SIZE, 0);
	validateReceiveBytes(bytes);
	Packet packet;
	unsigned bigBuffer;
	int i = 0;

	memset(&packet.header.version, ntohl(&buffer + (i++ * 4)), sizeof(unsigned));
	memset(&packet.header.messageType, ntohl(&buffer + (i++ * 4)), sizeof(unsigned));
	memset(&packet.header.messageLength, ntohl(&buffer + (i++ * 4)), sizeof(unsigned));

	unsigned littleBuffer;
	if (packet.header.messageType != MESSAGE_TYPE)
	{
		perror("incorrect message type");
		exit(EXIT_FAILURE);
	}
	else if (packet.header.version != PROTOCOL_NUMBER)
	{
		perror("incorrect protocol version");
		exit(EXIT_FAILURE);
	}
	char *receiveBuffer;
	char *outputBuffer;
	for (unsigned i = 0; i < packet.header.messageLength; i++, outputBuffer++, receiveBuffer++)
	{
		if (!(i % PAYLOAD_CHUNK_SIZE))
		{
			bytes = recv(socket, buffer, PAYLOAD_CHUNK_SIZE, 0);
			validateReceiveBytes(bytes);
			receiveBuffer = &buffer;
		}
		if (((i % 4) == 3) || i == packet.header.messageLength - 1)
		{

			memcpy(&littleBuffer, receiveBuffer, min(4, i - packet.header.messageLength - 1));
			bigBuffer = ntohl(bigBuffer);
			memcpy(&outputBuffer, &bigBuffer, min(4, i - packet.header.messageLength - 1));
		}
	}
}

void validateReceiveBytes(unsigned bytes)
{
	if (bytes == 0)
	{
		perror("disconnected");
		exit(EXIT_FAILURE);
	}
	else if (bytes < 0)
	{
		perror("receive failed");
		exit(EXIT_FAILURE);
	}
	else if (bytes != PAYLOAD_CHUNK_SIZE)
	{
		perror("header chunk not correct size");
		exit(EXIT_FAILURE);
	}
}