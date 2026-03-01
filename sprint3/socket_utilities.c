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
	for (unsigned i = 0; i < header.messageLength; i++)
	{
		if (((i % 4) == 3) || i == header.messageLength - 1)
		{
			unsigned startOfChunk = (i / 4) * 4;
			unsigned bytes = i - startOfChunk + 1;
			bigBuffer = 0;
			memcpy(&bigBuffer, packet->payload + startOfChunk, bytes);
			bigBuffer = htonl(bigBuffer);
			memcpy(sendBuffer + startOfChunk % PAYLOAD_CHUNK_SIZE, &bigBuffer, 4);
			if ((i % PAYLOAD_CHUNK_SIZE) == PAYLOAD_CHUNK_SIZE - 1 || i == header.messageLength - 1)
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

	int bytes = recv(socket, buffer, PAYLOAD_CHUNK_SIZE, MSG_WAITALL);
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
			bytes = recv(socket, buffer, PAYLOAD_CHUNK_SIZE, MSG_WAITALL);
			validateReceiveBytes(bytes);
		}
		if (((i % 4) == 3) || i == packet.header.messageLength - 1)
		{
			unsigned startOfChunk = (i / 4) * 4;
			unsigned bytes = i - startOfChunk + 1;
			memcpy(&littleBuffer, buffer + startOfChunk % PAYLOAD_CHUNK_SIZE, 4);
			littleBuffer = ntohl(littleBuffer);
			memcpy(packet.payload + startOfChunk, &littleBuffer, bytes);
		}
	}
	packet.payload[packet.header.messageLength] = '\0';
	return packet;
}
int sendPacketFloat(int socket, PacketFloat *packetFloat)
{
	Packet packet;
	packet.header = packetFloat->header;
	packet.header.messageLength = sizeof(float);
	packet.payload = malloc(sizeof(float));
	if (packet.payload == NULL)
	{
		perror("malloc failed");
		return -1;
	}
	memcpy(packet.payload, &packetFloat->payload, sizeof(float));
	int output = sendPacket(socket, &packet);
	freePacket(&packet);
	return output;
}
PacketFloat receivePacketFloat(int socket)
{
	Packet packet = receivePacket(socket);
	PacketFloat packetFloat;
	packetFloat.header = packet.header;
	memcpy(&packetFloat.payload, packet.payload, sizeof(float));
	freePacket(&packet);
	return packetFloat;
}
void freePacket(Packet *packet)
{
	if (packet->payload == NULL)
		return;
	free(packet->payload);
}