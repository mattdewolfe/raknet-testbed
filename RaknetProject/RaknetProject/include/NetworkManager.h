#pragma once

#include <assert.h>
#include "GetTime.h"
#include "Rand.h"
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "ReadyEvent.h"
#include "Kbhit.h"
#include "RakSleep.h"
#include "SocketLayer.h"
#include "FullyConnectedMesh2.h"
#include "ConnectionGraph2.h"

using namespace RakNet;
class GameManager;

enum GameMessages
{
	ID_TO_CLIENT_MESSAGE = ID_USER_PACKET_ENUM + 1,
	ID_TO_SERVER_MESSAGE = ID_TO_CLIENT_MESSAGE + 1,
	ID_CLIENT_DISCONNECT = ID_TO_CLIENT_MESSAGE + 2,
	ID_REPLY_CHOICE = ID_TO_CLIENT_MESSAGE + 3, 
	ID_READY_TO_PLAY = ID_TO_CLIENT_MESSAGE + 4, 
	ID_DEAL_CARD_TO_PLAYER =  ID_TO_CLIENT_MESSAGE + 5,
	ID_SEND_ANSWER_CARD =  ID_TO_CLIENT_MESSAGE + 6,
	ID_START_NEW_ROUND = ID_TO_CLIENT_MESSAGE + 7
};

class NetworkManager
{
public:
	NetworkManager(GameManager* _game);
	~NetworkManager() { Destroy(); }

	// Fire up raknet with apps and connection settings
	void Init(bool _isHost);
	// Connect to a remote
	bool EstablishConnection(const char _ip[]);
	// List IP addresses
	void ListIP();
	// Packet checking is done here
	void CheckPackets();
	/* Request the networking send a peer to peer message
	** @parem - GameMessage - the event type we are sending
	** @parem - SystemAddress - system of target machine
	** @parem - int - value of the card being passed across, as we use this for 
	**			sending and receiving card choices across clients and host
	** @parem - bool - flag to denote if we want this message sent to target client or broadcast
	**			 to all clients aside from this one
	*/
	void PeerToPeerMessage(GameMessages _messageType, SystemAddress _address, int _cardValue = -1, bool _broadcast = false);
	// Set an event flag for this client
	void SetEventState(GameMessages _event, bool _isReady);
	// Access the list of machines connected to the host
	SystemAddress GetConnectedMachine(int _playerNum);
	// Get number of systems connected
	unsigned short GetNumberOfConnections() { return numberOfSystems; }
	// Close and shutdown the servers
	void Destroy();
	
	bool IsHost() { return bIsHost; }

	// Queue of functions and parameters to be called
	// This is added to inside network update and called
	// outside of it for concurrency issues
//	std::vector<void (*func)(int), int> queue;
private:
	static const unsigned int MAX_CONNECTIONS = 8;
	static const unsigned int MIN_CONNECTIONS = 3;
	static const unsigned int PORT = 60000;

	// List of addresses connected to host
	SystemAddress remoteSystems[MAX_CONNECTIONS];
	unsigned short numberOfSystems;
	GameManager* game;
	
	bool bGameStarted;
	bool bIsHost;

	RakPeerInterface *rakPeer;
	ReadyEvent readyEventPlugin;

	FullyConnectedMesh2 FCM2;
	ConnectionGraph2 connectedGraph2;
	
};