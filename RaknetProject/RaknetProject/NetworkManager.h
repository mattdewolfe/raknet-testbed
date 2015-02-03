#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

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

enum GameMessages
{
	ID_TO_CLIENT_MESSAGE = ID_USER_PACKET_ENUM + 1,
	ID_TO_SERVER_MESSAGE = ID_TO_CLIENT_MESSAGE + 1,
	ID_CLIENT_DISCONNECT = ID_TO_CLIENT_MESSAGE + 2,
	ID_REPLY_CHOICE = ID_TO_CLIENT_MESSAGE + 3, 
	ID_READY_TO_PLAY = ID_TO_CLIENT_MESSAGE + 4 
};

class NetworkManager
{
public:
	NetworkManager();
	~NetworkManager() { Destroy(); }

	// Fire up raknet with apps and connection settings
	void Init();
	// Connect to a remote
	bool EstablishConnection(char* _ip  = "127.0.0.1");
	// List IP addresses
	void ListIP();
	// Packet checking is done here
	void CheckPackets();
	// Request the networking fire off a message
	void NetworkMessage(GameMessages _messageType, int _answerValue = 0);
	// Close and shutdown the servers
	void Destroy();
		
	// Get/set flag for master 
	void SetIsMaster(bool isMaster) { bIsMaster = isMaster; }
	bool GetIsMaster() { return bIsMaster; }

private:
	static const unsigned int MAX_CONNECTIONS = 8;
	static const unsigned int PORT = 60000;

	bool bIsMaster;
	bool bIsHost;

	RakPeerInterface *rakPeer;
	ReadyEvent readyEventPlugin;

	FullyConnectedMesh2 FCM2;
	ConnectionGraph2 connectedGraph2;

protected:
	
};
#endif