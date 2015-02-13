#include "NetworkManager.h"
#include "GameManager.h"

NetworkManager::NetworkManager(GameManager* _game) 
	: rakPeer(0), 
	bIsHost(false),
	bGameStarted(false),
	game(_game)

{
	printf("Networking enabled.\n");
}

void NetworkManager::Init(bool _isHost)
{
	bIsHost = _isHost;
	printf("getting rakpeer");
	rakPeer = RakPeerInterface::GetInstance();
	
	rakPeer->AttachPlugin(&readyEventPlugin);
	rakPeer->AttachPlugin(&FCM2);
	rakPeer->AttachPlugin(&connectedGraph2);
	rakPeer->SetMaximumIncomingConnections(MAX_CONNECTIONS);
	
	FCM2.SetAutoparticipateConnections(true);
	FCM2.SetConnectOnNewRemoteConnection(true, "");
	connectedGraph2.SetAutoProcessNewConnections(true);

	SocketDescriptor sd(PORT,0);
	while (IRNS2_Berkley::IsPortInUse(sd.port,sd.hostAddress,sd.socketFamily,SOCK_DGRAM)==true)
	{
		printf("Port %d in use, checking next... \n", sd.port);
		sd.port++;
	}
	StartupResult sr = rakPeer->Startup(MAX_CONNECTIONS, &sd, 1);
	RakAssert(sr==RAKNET_STARTED);
	printf("Networking initialized on port %d.\n", sd.port);
	// Give time for threads to startup
	RakSleep(100);
}

void NetworkManager::Destroy()
{
	if(rakPeer)
	{
		rakPeer->Shutdown(100,0);
		RakPeerInterface::DestroyInstance(rakPeer);
	}
}

bool NetworkManager::EstablishConnection(const char _ip[])
{
	ConnectionAttemptResult car;
	if (bIsHost == true)
	{
		car = rakPeer->Connect("127.0.0.1", PORT, 0, 0, 0);
	}
	else
	{
		car = rakPeer->Connect(_ip, PORT, 0, 0, 0);
	}
	RakAssert(car==CONNECTION_ATTEMPT_STARTED);
	printf("Attempting to connect to %s...\n", _ip);
	return true;
}

// List the IP addresses of my system
void NetworkManager::ListIP()
{
	for(unsigned int i = 0; i < rakPeer->GetNumberOfAddresses(); ++i)
	{
		printf("LocalIP: %s\n",rakPeer->GetLocalIP(i));
	}
}
// Send a message to a specific machine
void NetworkManager::PeerToPeerMessage(GameMessages _messageType, SystemAddress _address, int _cardValue)
{
	RakNet::BitStream bsOut;
	bsOut.Write(_messageType);
	bsOut.Write(_cardValue);
	rakPeer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,_address,false);
}
// Fire off a message to connected machines
void NetworkManager::NetworkMessage(GameMessages _messageType, int _cardValue)
{
	RakNet::BitStream bsOut;
	bsOut.Write(_messageType);
	bsOut.Write(_cardValue);
	rakPeer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,rakPeer->GetMyBoundAddress(),true);
}

void NetworkManager::SetEventState(GameMessages _event, bool _isReady)
{
	readyEventPlugin.SetEvent(_event,_isReady);
}

// Get a specific system address for peer to peer
SystemAddress NetworkManager::GetConnectedMachine(int _playerNum)
{
	// First off grab host address
	SystemAddress temp = rakPeer->GetSystemAddressFromIndex(0);
	// Next get number of systems, and try to grab 
	// the desired system address (ensuring within bounds of connected systems)
	rakPeer->GetConnectionList(remoteSystems, &numberOfSystems);
	if (_playerNum < numberOfSystems)
	{
		temp = rakPeer->GetSystemAddressFromIndex(_playerNum);
	}
	// Return result
	return temp;
}

// Pole packets and take action as needed
void NetworkManager::CheckPackets()
{
	while (rakPeer)
	{
		Packet *p = rakPeer->Receive();
		if (p)
		{
			RakNet::BitStream bitStream(p->data, p->length, false);
			int cardVal;
			GameMessages messageType;
			switch (p->data[0])
			{
			// Custom defined events
			// When receiving a card from the host
			case ID_DEAL_CARD_TO_PLAYER:
				bitStream.IgnoreBytes(sizeof(GameMessages));
				bitStream.Read(cardVal);
				game->AddCardToHand(cardVal);
				break;
			// Below are RakNet events
			case ID_NEW_INCOMING_CONNECTION:
				printf(": Network : A Player is joining...\n");
				readyEventPlugin.AddToWaitList(ID_READY_TO_PLAY, p->guid);
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
				printf(": Network : Joining game...\n");
				readyEventPlugin.AddToWaitList(ID_READY_TO_PLAY, p->guid);
				break;
			case ID_READY_EVENT_ALL_SET:
				bitStream.IgnoreBytes(sizeof(MessageID));
				bitStream.Read(messageType);
				// Act depending on ready event type
				switch(messageType)
				{
				// Everyone is ready to play
				case ID_READY_TO_PLAY:
					if (bGameStarted == false)
					{
						system("cls");
						printf(": Network :  \\('^')/ Let the game being!\n", p->guid.ToString());
						Sleep(35);
						bGameStarted = true;
						if (bIsHost)
						{
							// Store number of connections
							rakPeer->GetConnectionList(remoteSystems, &numberOfSystems);
						}
						game->StartRound();
					}
					break;
				// Everyone has submitted an answer card
				case ID_SEND_ANSWER_CARD:
					
					break;
				}
				break;

			case ID_READY_EVENT_SET:
				printf(": Network : %s is ready.\n", p->guid.ToString());
				break;

			case ID_READY_EVENT_UNSET:
				printf(": Network : %s is not ready.\n", p->guid.ToString());
				break;

			case ID_DISCONNECTION_NOTIFICATION:
				// Connection lost normally
				printf(": Network : ID_DISCONNECTION_NOTIFICATION\n");
				break;
			case ID_ALREADY_CONNECTED:
				// Connection lost normally
				printf(": Network : ID_ALREADY_CONNECTED with guid %" PRINTF_64_BIT_MODIFIER "u\n", p->guid);
				break;
			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				printf(": Network : ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
				break;
			case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
				printf(": Network : ID_REMOTE_DISCONNECTION_NOTIFICATION\n"); 
				break;
			case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
				printf(": Network : ID_REMOTE_CONNECTION_LOST\n");
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
				printf(": Network : ID_REMOTE_NEW_INCOMING_CONNECTION\n");
				break;
			case ID_CONNECTION_BANNED: // Banned from this server
				printf(": Network : We are banned from this server.\n");
				break;			
			case ID_CONNECTION_ATTEMPT_FAILED:
				printf(": Network : Connection attempt failed\n");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				// Sorry, the server is full.  I don't do anything here but
				// A real app should tell the user
				printf(": Network : ID_NO_FREE_INCOMING_CONNECTIONS\n");
				break;
			case ID_INVALID_PASSWORD:
				printf(": Network : ID_INVALID_PASSWORD\n");
				break;
			case ID_CONNECTION_LOST:
				// Couldn't deliver a reliable packet - i.e. the other system was abnormally
				// terminated
				printf(": Network : ID_CONNECTION_LOST\n");
				break;
			case ID_CONNECTED_PING:
			case ID_UNCONNECTED_PING:
				printf(": Network : Ping from %s\n", p->systemAddress.ToString(true));
				break;
			}

			rakPeer->DeallocatePacket(p);
		}		

		// Keep raknet threads responsive
		RakSleep(30);	
	}
}