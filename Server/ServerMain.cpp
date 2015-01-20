#include <stdio.h>
#include <string.h>
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"  // MessageID

#define MAX_CLIENTS 125
#define SERVER_PORT 6000
using namespace RakNet;

void ListenForPackets(RakPeerInterface* _peer);

enum GameMessages
{
	ID_TO_CLIENT_MESSAGE = ID_USER_PACKET_ENUM + 1,
	ID_TO_SERVER_MESSAGE = ID_TO_CLIENT_MESSAGE + 1
};

int main()
{
	RakPeerInterface *peer = RakNet::RakPeerInterface::GetInstance();
	
	RakNet::SocketDescriptor sd(SERVER_PORT,0);
	RakNet::StartupResult result = peer->Startup(MAX_CLIENTS, &sd, 1);
	//check to make sure server startup was a success
	assert(result == RAKNET_STARTED || result == RAKNET_ALREADY_STARTED );
	
	printf("Server start up successfull\n");

	for(unsigned int i = 0; i < peer->GetNumberOfAddresses(); ++i)
		printf("LocalIP: %s\n",peer->GetLocalIP(i));

	// We need to let the server accept incoming connections from the clients
	peer->SetMaximumIncomingConnections(MAX_CLIENTS);

	printf("waiting for packets...\n");
	
	ListenForPackets(peer);

	RakNet::RakPeerInterface::DestroyInstance(peer);

	return 0;
}


void ListenForPackets(RakPeerInterface* _peer)
{
	Packet *packet;
	while (1)
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
			case ID_TO_SERVER_MESSAGE:
				{
					
					RakNet::RakString rs;
					RakNet::BitStream bsIn(packet->data,packet->length,false);
					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);
					printf("%s\n", rs.C_String());
					
					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_TO_CLIENT_MESSAGE);
					bsOut.Write(rs);
					_peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,true);
				}
				break;
			case ID_TO_CLIENT_MESSAGE:
				{
					RakNet::RakString rs;
					RakNet::BitStream bsIn(packet->data,packet->length,false);
					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);
					printf("%s\n", rs.C_String());
				}
			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
		}
	}
}