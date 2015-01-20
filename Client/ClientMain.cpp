#include <stdio.h>
#include <string.h>
#include <thread>

#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"  // MessageID

#define SERVER_PORT 6000
using namespace RakNet;

AddressOrGUID hostAddress;
bool isListening = false;
SystemAddress addressList[10];
char name[30];
char spacer[3] = ": ";
void ListenForPackets(RakPeerInterface* _peer);
void ShutdownClient(RakPeerInterface* _peer);
void GetConnectionList(RakPeerInterface* _peer);

enum GameMessages
{
	ID_TO_CLIENT_MESSAGE = ID_USER_PACKET_ENUM + 1,
	ID_TO_SERVER_MESSAGE = ID_TO_CLIENT_MESSAGE + 1
};

int main()
{
	RakPeerInterface *peer = RakPeerInterface::GetInstance();
	SocketDescriptor sd;
	StartupResult result = peer->Startup(1,&sd,1);
	//check to make sure server startup was a success
	assert(result == RAKNET_STARTED || result == RAKNET_ALREADY_STARTED );
	printf("Enter server IP or hit enter for 127.0.0.1\n");
	char str[512];	
	gets_s(str);
	if (str[0]==0)
	{
		strcpy_s(str, "127.0.0.1");
	}
	printf("Starting the client.\n");
	ConnectionAttemptResult con_result = peer->Connect(str, SERVER_PORT, 0,0);
	assert(result == CONNECTION_ATTEMPT_STARTED || result == ALREADY_CONNECTED_TO_ENDPOINT || result == CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS );

	// Start packet listener in a new thread
	isListening = true;
	std::thread packetListener (ListenForPackets, peer);

	GetConnectionList(peer);
	gets_s(name);

	// New while loop for handling input
	while (str[0]!='q')
	{
		printf("Message: ");
		gets_s(str);
		BitStream bsOut;
		char message[512];
		strcpy(message, name);
		strcat(message, spacer);
		strcat(message, str);
		bsOut.Write((RakNet::MessageID)ID_TO_SERVER_MESSAGE);
		bsOut.Write(message);
		peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,addressList[0],false);
	}

	printf("Farewell...\n");
	isListening = false;
	ShutdownClient(peer);
	// Once we stop listening (close thread) we will do this
	packetListener.join();
}

void GetConnectionList(RakPeerInterface* _peer)
{
	unsigned short numConnections = 10;
	_peer->GetConnectionList(addressList, &numConnections);
	printf(" -- Currently connected to %d systems.\n", numConnections);
	printf("Enter your name: ");
}

void ShutdownClient(RakPeerInterface* _peer)
{
	printf("shutting down client");
	_peer->Shutdown(100);
	RakNet::RakPeerInterface::DestroyInstance(_peer);
}

void ListenForPackets(RakPeerInterface* _peer)
{
	Packet *packet;
	while (isListening == true)
	{
		for (packet = _peer->Receive(); packet; _peer->DeallocatePacket(packet), packet=_peer->Receive())
		{
			switch (packet->data[0])
			{
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
				printf("Another client has disconnected.\n");
				break;
			case ID_REMOTE_CONNECTION_LOST:
				printf("Another client has lost the connection.\n");
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION:
				printf("Another client has connected.\n");
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
				printf("Our connection request has been accepted.\n");
				hostAddress = packet->systemAddress;
				GetConnectionList(_peer);
				break;
			case ID_NEW_INCOMING_CONNECTION:
				printf("A connection is incoming.\n");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				printf("The server is full.\n");
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				printf("We have been disconnected.\n");
				break;
			case ID_CONNECTION_LOST:
				printf("Connection lost.\n");
				break;
			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
		}
	}
}