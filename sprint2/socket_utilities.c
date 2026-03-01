#include "socket_utilities.h"
#include "math.h"
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
int sendPacket(int socket, Packet *packet)
{
	PacketHeader header = packet->header;
	char sendHeaderBuffer[PAYLOAD_CHUNK_SIZE];

	int i = 0;
	unsigned temp;
	temp = htonl(header.version);
	memcpy(sendHeaderBuffer + i++ * 4, &temp, sizeof(unsigned));
	temp = htonl(header.messageType);
	memcpy(sendHeaderBuffer + i++ * 4, &temp, sizeof(unsigned));
	temp = htonl(header.messageLength);
	memcpy(sendHeaderBuffer + i++ * 4, &temp, sizeof(unsigned));

	if (send(socket, sendHeaderBuffer, sizeof(sendHeaderBuffer), 0) < 0)
	{
		return -1;
	}
	char sendBuffer[PAYLOAD_CHUNK_SIZE];
	memset(sendBuffer, 0, PAYLOAD_CHUNK_SIZE);
	unsigned bigBuffer;
	char *ptr = packet->payload;
	for (unsigned i = 0; i < header.messageLength; i++, ptr++)
	{
		if (((i % 4) == 3) || i == header.messageLength - 1)
		{
			memcpy(&bigBuffer, ptr, fmin(4, header.messageLength - i));
			bigBuffer = htonl(bigBuffer);
			memcpy(sendBuffer + (i % 3) * 4, &bigBuffer, fmin(4, header.messageLength - i));
			if ((i % PAYLOAD_CHUNK_SIZE) == PAYLOAD_CHUNK_SIZE - 1)
			{
				if (send(socket, sendBuffer, PAYLOAD_CHUNK_SIZE, 0) < 0)
				{
					return -1;
				}
				memset(sendBuffer, 0, PAYLOAD_CHUNK_SIZE);
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
	int i = 0;

	unsigned temp;
	memcpy(&temp, buffer + (i++ * 4), sizeof(unsigned));
	packet.header.version = ntohl(temp);
	memcpy(&temp, buffer + (i++ * 4), sizeof(unsigned));
	packet.header.messageType = ntohl(temp);
	memcpy(&temp, buffer + (i++ * 4), sizeof(unsigned));
	packet.header.messageLength = ntohl(temp);

	unsigned littleBuffer;
	printf("header: (messageType: %u, protocol version: %u, length: %u)\n",
		   packet.header.messageType,
		   packet.header.version,
		   packet.header.messageLength);
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
	packet.payload = malloc(packet.header.messageLength + 1);
	if (!packet.payload)
	{
		perror("malloc failed");
		exit(EXIT_FAILURE);
	}
	for (unsigned i = 0; i < packet.header.messageLength; i++)
	{
		if (!(i % PAYLOAD_CHUNK_SIZE))
		{
			bytes = recv(socket, buffer, PAYLOAD_CHUNK_SIZE, 0);
			validateReceiveBytes(bytes);
		}
		if (((i % 4) == 3) || i == packet.header.messageLength - 1)
		{

			memcpy(&littleBuffer, buffer + (i % 3) * 4, fmin(4, packet.header.messageLength - i));
			littleBuffer = ntohl(littleBuffer);
			memcpy(packet.payload + (unsigned)((i / 4) * 4), &littleBuffer, fmin(4, packet.header.messageLength - i));
		}
	}
	packet.payload[packet.header.messageLength] = '\0';
	return packet;
}
