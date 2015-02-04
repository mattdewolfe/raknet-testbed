#include <thread>
#include <iostream>
#include <RakThread.h>
#include <string>
#include "NetworkManager.h"

NetworkManager* networkManager;
void UpdateNetwork();
// Ask player to enter an IP or host a game, then call necessary Raknet function
void JoinOrHostGame();
// Basic function to request player enter their name
void GetPlayerName();
// Called when player is ready to begin, sends message to network helper to set ready event
void WaitingForReady();

std::thread* networkUpdates;
std::string playerName;

int main(void)
{
	networkManager = new NetworkManager();
	GetPlayerName();
	JoinOrHostGame();
	// Start up a seperate thread for network updates
	networkUpdates = new std::thread(UpdateNetwork);
	networkUpdates->detach();
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

void GetPlayerName()
{
	std::cout << "Enter your name: ";
	std::cin >> playerName;
	std::cin.clear();
	char temp[5];
	gets_s(temp);
	if (playerName == "Network" || playerName == "NETWORK" || playerName == "network")
	{
		std::cout << "You are not a network.\n";
		GetPlayerName();
	}
}
void JoinOrHostGame()
{
	// Ask player for IP address to connect to
	char ip[256];
	std::cout << "Enter host IP address (or press enter to host): ";
	gets_s(ip);
	// List active connections
	if (ip[0] == 0)
	{
		// Init raknet
		networkManager->Init(true);
		networkManager->ListIP();
	}
	else 
	{
		networkManager->Init(false);
		if (networkManager->EstablishConnection(ip) == true)
		{
			networkManager->ListIP();
		}
	}
}

void WaitingForReady()
{
	// Wait for a key press
	std::cout << ": " << playerName << " : Press Enter when you are ready to play.\n";
	getchar();
	// Once hit, let the other clients know this player is ready
	networkManager->SetEventState(GameMessages::ID_READY_TO_PLAY, true);
}

void UpdateNetwork()
{
	networkManager->CheckPackets();
}