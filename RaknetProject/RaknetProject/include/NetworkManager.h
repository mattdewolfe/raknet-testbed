#pragma once

#include <assert.h>
#include <string>
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

// NOTE - We must still ignore (MessageID) when reading in our bit streams
// or else data will not be properly parsed. This is an oddity of Raknet.
enum GameMessages
{
	ID_TO_CLIENT_MESSAGE = ID_USER_PACKET_ENUM + 1,
	ID_TO_SERVER_MESSAGE = ID_TO_CLIENT_MESSAGE + 1,
	ID_CLIENT_DISCONNECT = ID_TO_CLIENT_MESSAGE + 2,
	ID_REPLY_CHOICE = ID_TO_CLIENT_MESSAGE + 3, 
	ID_READY_TO_PLAY = ID_TO_CLIENT_MESSAGE + 4, 
	ID_DEAL_CARD_TO_PLAYER =  ID_TO_CLIENT_MESSAGE + 5,
	ID_SEND_ANSWER_CARD =  ID_TO_CLIENT_MESSAGE + 6,
	ID_START_NEXT_ROUND = ID_TO_CLIENT_MESSAGE + 7,
	ID_ASSIGN_QUESTION_ASKER_NO_BROADCAST = ID_TO_CLIENT_MESSAGE + 8,
	ID_AWARD_POINT = ID_TO_CLIENT_MESSAGE + 9,
	ID_READY_FOR_NEXT_ROUND = ID_TO_CLIENT_MESSAGE + 10
};

// Stores information for each submitted answer each round
struct AnswerInfo
{
	// Store the last card they submitted
	int submittedAnswer;
	// Store the address this answer came from
	SystemAddress address;
	// Stores their name
	std::string playerName;
};

class NetworkManager
{
private:
	static const unsigned int MAX_CONNECTIONS = 8;
	static const unsigned int MIN_CONNECTIONS = 3;
	static const unsigned int PORT = 60000;

public:
	// Stores player info regarding address, name, score, etc
	AnswerInfo answerInfo[MAX_CONNECTIONS];
	// Access the list of machines connected to the host
	SystemAddress GetConnectedMachine(int _playerNum);
	
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
	** @parem - std::string - name of the player sending this message
	** @parem - GameMessage - the event type we are sending
	** @parem - SystemAddress - system of target machine
	** @parem - int - value of the card being passed across, as we use this for 
	**			sending and receiving card choices across clients and host
	** @parem - bool - flag to denote if we want this message sent to target client or broadcast
	**			 to all clients aside from this one
	*/
	void PeerToPeerMessage(std::string _name, GameMessages _messageType, SystemAddress _address, int _cardValue = -1, bool _broadcast = false);
	// Set an event flag for this client
	void SetEventState(GameMessages _event, bool _isReady);
	// Get number of systems connected
	unsigned short GetNumberOfConnections() { return numberOfSystems; }
	unsigned short GetNumberOfAnswers() { return totalAnswersReceived; }
	bool IsHost() { return bIsHost; }
	
	// Close and shutdown the servers
	void Destroy();

private:
	// Add a players answer information to answer array
	void AddAnswerInfo(int _cardVal, std::string _name, SystemAddress _address);

	// List of addresses connected to host
	SystemAddress remoteSystems[MAX_CONNECTIONS];
	unsigned short numberOfSystems;
	GameManager* game;
	
	
	// Tracks how many answers have been submitted
	int totalAnswersReceived;
	// Track the current question asker
	int currentQuestionAsker;
	// Has the game started - no more joiners
	bool bGameStarted;
	// Is this the host machine
	bool bIsHost;
	// Is the new round ready to start
	bool bIsNewRoundReady;

	// Raknet variables - key to standard networking, event system, and connected graph
	RakPeerInterface *rakPeer;
	ReadyEvent readyEventPlugin;

	FullyConnectedMesh2 FCM2;
	ConnectionGraph2 connectedGraph2;
	
};