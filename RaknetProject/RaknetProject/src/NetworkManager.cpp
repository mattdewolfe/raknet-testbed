#include "NetworkManager.h"

NetworkManager::NetworkManager() 
	: rakPeer(0), 
	bIsHost(false),
	bGameStarted(false),
	bIsNewRoundReady(false)

{
	printf(" - Networking enabled - .\n");
}

// Registered RPC call when a player is booted
void NetworkManager::KickedFromServerRPC(BitStream *_bitStream, Packet *_packet)
{
	std::cout << " You were kicked from the server.\0";
	RakPeerInterface *rakPeerTemp = RakPeerInterface::GetInstance();
	rakPeerTemp->Shutdown(100);
}
// Called at set intervals, for rpc testing
void NetworkManager::IntervalTickRPC(BitStream *_bitStream, Packet *_packet)
{
	std::cout << " Interval tick RPC called.\0";
}

void NetworkManager::Init(bool _isHost)
{
	bIsHost = _isHost;
	rakPeer = RakPeerInterface::GetInstance();
	
	// Attach plugins
	rakPeer->AttachPlugin(&readyEventPlugin);
	rakPeer->SetMaximumIncomingConnections(MAX_CONNECTIONS);
	rakPeer->AttachPlugin(&FCM2);
	rakPeer->AttachPlugin(&connectedGraph2);
	rakPeer->AttachPlugin(&rpc);
	
	// Setup RPC functions
	rpc.RegisterSlot("Kicked", KickedFromServerRPC, 0);
	rpc.RegisterSlot("Interval", IntervalTickRPC, 0);
	//rpc.RegisterBlockingFunction("Blocking", CFunc3);
	
	// Setup fully connected mesh
	FCM2.SetAutoparticipateConnections(true);
	FCM2.SetConnectOnNewRemoteConnection(true, "");
	connectedGraph2.SetAutoProcessNewConnections(true);

	SocketDescriptor sd(PORT,0);
	while (IRNS2_Berkley::IsPortInUse(sd.port,sd.hostAddress,sd.socketFamily,SOCK_DGRAM)==true)
	{
		printf(" : Network : Port %d in use, checking next... \n", sd.port);
		sd.port++;
	}
	StartupResult sr = rakPeer->Startup(MAX_CONNECTIONS, &sd, 1);
	RakAssert(sr==RAKNET_STARTED);
	printf(" : Network : Initialized on port %d.\n", sd.port);
	// Give time for threads to startup
	RakSleep(100);
}

// Connects to target IP
bool NetworkManager::EstablishConnection(const char _ip[])
{
	ConnectionAttemptResult car;
	if (bIsHost == true)
	{
		car = rakPeer->Connect("127.0.0.1", PORT, 0, 0, 0);
		std::cout << " : Network : Hosting game..." << std::endl;
	}
	else
	{
		//car = rakPeer->Connect(_ip, PORT, 0, 0, 0);
		std::cout << " : Network : Attempting to connect to " << _ip << "..." << std::endl;
		// Temp line for faster testing on my home PC
		car = rakPeer->Connect("192.168.0.102", PORT, 0, 0, 0);
		// Temp line for testing on school pc
		// car = rakPeer->Connect("10.10.107.141", PORT, 0, 0, 0);
	}
	RakAssert(car==CONNECTION_ATTEMPT_STARTED);

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

// Send a system message, generally with card value, to target machine or all clients
void NetworkManager::PeerToPeerMessage(std::string _name, GameMessages _messageType, SystemAddress _address, int _cardValue, bool _broadcast)
{
	RakNet::BitStream bsOut;
	bsOut.Write((unsigned char)_messageType);
	bsOut.Write(_cardValue);
	bsOut.Write(_name);

	// Check if this message is intended to broadcast or not, and call matching 
	// version of rak-Send
	
	if (_broadcast == true)
	{
		assert(_messageType != ID_ASSIGN_QUESTION_ASKER_NO_BROADCAST);
		rakPeer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,UNASSIGNED_SYSTEM_ADDRESS,true);
	}
	else 
	{
		rakPeer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,_address,false);
	}
}

// Change the target event state to the bool passed in
void NetworkManager::SetEventState(GameMessages _event, bool _isReady)
{
	readyEventPlugin.SetEvent(_event,_isReady);
}

// Get a specific system address for peer to peer
SystemAddress NetworkManager::GetConnectedMachine(int _playerNum)
{
	// Grab first index
	SystemAddress temp = rakPeer->GetSystemAddressFromIndex(0);
	// Next get number of systems, and try to grab 
	// the desired system address (ensuring within bounds of connected systems)
	rakPeer->GetConnectionList(remoteSystems, &numberOfSystems);
	if (_playerNum <= numberOfSystems)
	{
		temp = rakPeer->GetSystemAddressFromIndex(_playerNum);
	}
	// Return result
	return temp;
}

// Pole packets and take action as needed
void NetworkManager::CheckPackets()
{
	std::string playerName;
	int cardVal;
	GameMessages messageType;
	bool looping = true;
	while (looping)
	{
		Packet *p = rakPeer->Receive();
		if (p)
		{
			RakNet::BitStream bitStream(p->data, p->length, false);
			switch (p->data[0])
			{
	/*		// Custom defined events
			// A player has won the game
			case ID_GAME_OVER:
				looping = false;
				bitStream.IgnoreBytes(sizeof(MessageID));
				bitStream.Read(cardVal);
				bitStream.Read(playerName);
				game->GameOverScreen(playerName);
				break;
			// Receive the winning card selection
			case ID_REPLY_CHOICE:
				bitStream.IgnoreBytes(sizeof(MessageID));
				bitStream.Read(cardVal);
				game->DisplayAnswersAndWinner(cardVal);
				break;
			// Assign someone as the question asker
			case ID_AWARD_POINT:
				game->AwardPoint();
				break;
			// Assign someone as the question asker
			case ID_ASSIGN_QUESTION_ASKER_NO_BROADCAST:
				// Sets gameplay flag marking this player as question asker
				game->MakeQuestionAsker();
				break;
			// When a player sends their cards to the host
			case ID_SEND_ANSWER_CARD:
				// If I am the host, broadcast this card to everyone
				bitStream.IgnoreBytes(sizeof(MessageID));
				// Read in the card reference value for the answer
				bitStream.Read(cardVal);
				// Read in the players name
				bitStream.Read(playerName);
				// Store IP address this answer came from
				AddAnswerInfo(cardVal, playerName, p->systemAddress);
				break;
			// When receiving a card from the host
			case ID_DEAL_CARD_TO_PLAYER:
				bitStream.IgnoreBytes(sizeof(MessageID));
				bitStream.Read(cardVal);
				game->AddCardToHand(cardVal);
				break;
			// When receiving a ready to play message
			case ID_READY_TO_PLAY:
				if (bGameStarted == false)
				{
					bitStream.IgnoreBytes(sizeof(MessageID));
					bitStream.Read(cardVal);
					bitStream.Read(playerName);
					std::cout << ": Network : " << playerName << " is ready to play." << std::endl;
				}
				break;
			// When receiving the start next round message (should broadcast from host)
			case ID_START_NEXT_ROUND:
				bIsCardSent = false;
				SetEventState(ID_SEND_ANSWER_CARD, false);
				SetEventState(ID_READY_FOR_NEXT_ROUND, false);
				totalAnswersReceived = 0;
				bitStream.IgnoreBytes(sizeof(MessageID));
				bitStream.Read(cardVal);
				game->StartNextRound(cardVal);
				break;
			// Below are RakNet events
			case ID_NEW_INCOMING_CONNECTION:
				std::cout << ": Network : A player has joined." << std::endl;
				readyEventPlugin.AddToWaitList(ID_READY_TO_PLAY, p->guid);
				readyEventPlugin.AddToWaitList(ID_SEND_ANSWER_CARD, p->guid);
				readyEventPlugin.AddToWaitList(ID_READY_FOR_NEXT_ROUND, p->guid);
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
				std::cout << ": Network : Joining game..." << std::endl;
				readyEventPlugin.AddToWaitList(ID_READY_TO_PLAY, p->guid);
				readyEventPlugin.AddToWaitList(ID_SEND_ANSWER_CARD, p->guid);
				readyEventPlugin.AddToWaitList(ID_READY_FOR_NEXT_ROUND, p->guid);
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
						std::cout << ": Network :  \\('^')/ Let the game being!" << std::endl;
						bGameStarted = true;
						if (bIsHost)
						{
							// Store number of connections
							rakPeer->GetConnectionList(remoteSystems, &numberOfSystems);
						}
						game->StartGame();
					}
					break;
				// Everyone has submitted an answer card
				case ID_SEND_ANSWER_CARD:
					if (bIsCardSent == false)
					{
						bIsCardSent = true;
						bIsNewRoundReady = false;
						game->ShowAnswerCardsToAsker();
					}
					break;
				// Everyone is ready to start a new round
				case ID_READY_FOR_NEXT_ROUND:
					// Flag so actions only happen once
					if (bIsNewRoundReady == false)
					{
						SetEventState(ID_SEND_ANSWER_CARD, false);
						SetEventState(ID_READY_FOR_NEXT_ROUND, false);
						bIsNewRoundReady = true;
						if (bIsHost == true)
						{
							bIsCardSent = false;
							// Increment question asker index, and clamp to bounds
							currentQuestionAsker++;
							// If capped, I am the question asker
							if (currentQuestionAsker >= numberOfSystems)
							{
								currentQuestionAsker = -1;
								game->MakeQuestionAsker();
							}
							// Otherwise, assign a remote system as asker
							else
							{
								PeerToPeerMessage("Host", 
									ID_ASSIGN_QUESTION_ASKER_NO_BROADCAST, 
									remoteSystems[currentQuestionAsker]);
							}
							// Get new card value
							int nextCard = game->GetNextQuestionCard();
							// Reset size of answer info array
							totalAnswersReceived = 0;
							// Send message to clients to start new round
							PeerToPeerMessage(playerName,
								ID_START_NEXT_ROUND,
								UNASSIGNED_SYSTEM_ADDRESS,
								nextCard,
								true);
							// Start a new round on this machine
							game->StartNextRound(nextCard);
						}
					}
					break;
				}
				break;*/

			case ID_READY_EVENT_SET:
				break;

			case ID_READY_EVENT_UNSET:
				break;

			case ID_DISCONNECTION_NOTIFICATION:
				// Connection lost normally
				printf(" : Network : ID_DISCONNECTION_NOTIFICATION\n");
				break;
			case ID_ALREADY_CONNECTED:
				// Connection lost normally
				printf(" : Network : ID_ALREADY_CONNECTED with guid %" PRINTF_64_BIT_MODIFIER "u\n", p->guid);
				break;
			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				printf(" : Network : ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
				break;
			case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
				printf(" : Network : ID_REMOTE_DISCONNECTION_NOTIFICATION\n"); 
				break;
			case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
				printf(" : Network : ID_REMOTE_CONNECTION_LOST\n");
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
				break;
			case ID_CONNECTION_BANNED: // Banned from this server
				printf(" : Network : We are banned from this server.\n");
				break;			
			case ID_CONNECTION_ATTEMPT_FAILED:
				printf(" : Network : Connection attempt failed\n");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				// Sorry, the server is full.  I don't do anything here but
				// A real app should tell the user
				printf(" : Network : ID_NO_FREE_INCOMING_CONNECTIONS\n");
				break;
			case ID_INVALID_PASSWORD:
				printf(" : Network : ID_INVALID_PASSWORD\n");
				break;
			case ID_CONNECTION_LOST:
				// Couldn't deliver a reliable packet - i.e. the other system was abnormally
				// terminated
				printf(" : Network : ID_CONNECTION_LOST\n");
				break;
			case ID_CONNECTED_PING:
			case ID_UNCONNECTED_PING:
				printf(" : Network : Ping from %s\n", p->systemAddress.ToString(true));
				break;
			}

			rakPeer->DeallocatePacket(p);
		}		

		// Keep raknet threads responsive
		RakSleep(10);	
	}
}

void NetworkManager::Destroy()
{
	if(rakPeer)
	{
		rakPeer->Shutdown(100,0);
		RakPeerInterface::DestroyInstance(rakPeer);
	}
}