#include <RakThread.h>
#include <thread>
#include "NetworkManager.h"

NetworkManager* networkManager;
void UpdateNetwork();
char* GetInput(char* _requestToUser);
void JoinOrHostGame();
void WaitingForReady();

char* playerName;

int main(void)
{
	networkManager = new NetworkManager();
	// Get players name
	playerName = GetInput("Enter your name: ");
	// Init raknet
	networkManager->Init();
	JoinOrHostGame();
	// Start up a seperate thread for network updates
	std::thread networkThread(UpdateNetwork);
	networkThread.detach();
	// Call function to wait for input
	WaitingForReady();

	// Listen for keyboard input and loop
	char ch=0;
	while (1)
	{
		if (_kbhit())
		{
			ch=_getch();
		}
		if (ch =='q' || ch == 'Q')
		{
			break;
		}

	}
	networkManager->Destroy();
}

char* GetInput(char* _requestToUser)
{
	printf(_requestToUser);
	char temp[512];
	gets_s(temp);
	return temp;
}

void JoinOrHostGame()
{
	// Ask player for IP address to connect to
	char* ip;
	ip = GetInput("Enter host IP address (or press enter to host): ");
	// List active connections
	if (ip[0] != 0)
	{
		if (networkManager->EstablishConnection() == true)
		{
			networkManager->ListIP();
		}
	}
	else 
	{
		if (networkManager->EstablishConnection() == true)
		{
			networkManager->ListIP();
		}
	}
}

void WaitingForReady()
{
	// Wait for a key press
	printf(": %c : Press Enter when you are ready to play.\n", &playerName);
	getchar();
	// Once hit, let the other clients know this player is ready
	networkManager->NetworkMessage(GameMessages::ID_READY_TO_PLAY);
}

void UpdateNetwork()
{
	networkManager->CheckPackets();
}