#include "NetworkManager.h"

NetworkManager::NetworkManager() : rakPeer(0), bIsHost(false) ,bIsMaster(false)
{
}

void NetworkManager::Init(bool isHost)
{
	rakPeer = RakPeerInterface::GetInstance();
	bIsHost = isHost;
	
	rakPeer->AttachPlugin(&readyEventPlugin);
	rakPeer->AttachPlugin(&FCM2);
	rakPeer->AttachPlugin(&connectedGraph2);
	rakPeer->SetMaximumIncomingConnections(MAX_CONNECTIONS);
	
	FCM2.SetAutoparticipateConnections(true);
	FCM2.SetConnectOnNewRemoteConnection(true, "");
	connectedGraph2.SetAutoProcessNewConnections(true);

	SocketDescriptor sd(PORT,0);
	while (IRNS2_Berkley::IsPortInUse(sd.port,sd.hostAddress,sd.socketFamily,SOCK_DGRAM)==true)
		sd.port++;

	StartupResult sr = rakPeer->Startup(MAX_CONNECTIONS, &sd, 1);
	RakAssert(sr==RAKNET_STARTED);

	// Give time for threads to startup
	RakSleep(200);
}

void NetworkManager::Destroy()
{
	if(rakPeer)
		RakPeerInterface::DestroyInstance(rakPeer);
}

bool NetworkManager::EstablishConnection()
{
	ConnectionAttemptResult car = rakPeer->Connect("127.0.0.1", PORT, 0, 0, 0);
	RakAssert(car==CONNECTION_ATTEMPT_STARTED);
	return true;
}

// Fire off a message to connected machines
void NetworkMessage(GameMessages _messageType, int _answerValue = 0)
{

}

// Pole packets and take action as needed
void NetworkManager::CheckPackets()
{
	Packet *p = rakPeer->Receive();
	if (p)
	{
		switch (p->data[0])
		{
		case ID_NEW_INCOMING_CONNECTION:
			printf("ID_NEW_INCOMING_CONNECTION\n");
			readyEventPlugin.AddToWaitList(0, p->guid);
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			printf("ID_CONNECTION_REQUEST_ACCEPTED\n");
			readyEventPlugin.AddToWaitList(0, p->guid);
			break;
		case ID_READY_EVENT_ALL_SET:
			printf("Got ID_READY_EVENT_ALL_SET from %s\n", p->guid.ToString());
			break;

		case ID_READY_EVENT_SET:
			printf("Got ID_READY_EVENT_SET from %s\n", p->guid.ToString());
			break;

		case ID_READY_EVENT_UNSET:
			printf("Got ID_READY_EVENT_UNSET from %s\n", p->guid.ToString());
			break;

		case ID_DISCONNECTION_NOTIFICATION:
			// Connection lost normally
			printf("ID_DISCONNECTION_NOTIFICATION\n");
			break;
		case ID_ALREADY_CONNECTED:
			// Connection lost normally
			printf("ID_ALREADY_CONNECTED with guid %" PRINTF_64_BIT_MODIFIER "u\n", p->guid);
			break;
		case ID_INCOMPATIBLE_PROTOCOL_VERSION:
			printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
			break;
		case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
			printf("ID_REMOTE_DISCONNECTION_NOTIFICATION\n"); 
			break;
		case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
			printf("ID_REMOTE_CONNECTION_LOST\n");
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
			printf("ID_REMOTE_NEW_INCOMING_CONNECTION\n");
			break;
		case ID_CONNECTION_BANNED: // Banned from this server
			printf("We are banned from this server.\n");
			break;			
		case ID_CONNECTION_ATTEMPT_FAILED:
			printf("Connection attempt failed\n");
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			// Sorry, the server is full.  I don't do anything here but
			// A real app should tell the user
			printf("ID_NO_FREE_INCOMING_CONNECTIONS\n");
			break;
		case ID_INVALID_PASSWORD:
			printf("ID_INVALID_PASSWORD\n");
			break;
		case ID_CONNECTION_LOST:
			// Couldn't deliver a reliable packet - i.e. the other system was abnormally
			// terminated
			printf("ID_CONNECTION_LOST\n");
			break;
		case ID_CONNECTED_PING:
		case ID_UNCONNECTED_PING:
			printf("Ping from %s\n", p->systemAddress.ToString(true));
			break;
		}

		rakPeer->DeallocatePacket(p);
	}		

	// Keep raknet threads responsive
	RakSleep(30);	
}