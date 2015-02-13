#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <RakThread.h>
#include <thread>
#include <algorithm>
#include "XMLScriptManager.h"
#include "NetworkManager.h"

// Max number of cards the player can hold
#define HAND_SIZE 3

enum GAME_STATE
{
	LOADING = 0, 
	ROUND_START,
	SUBMIT_ANSWER, 
	WAITING_FOR_ANSWERS,
	WAITING_FOR_PLAYERS,
	SCORING, 
	QUIT
};

class GameManager
{
public:

	GameManager(void);
	~GameManager(void);

	GAME_STATE state;

	// Process player input
	void KeyPress(const char _ch);
	// Add a card to the local players hand
	void AddCardToHand(int _cardNum);
	// Return a card number from their hand to pass to server
	int RemoveCardFromHand(int _choice);
	// Clear screen and display information to start the round
	void StartRound();
	// Start up networking and game
	void Init();
	// Shutdown the system
	void Shutdown();
	// Ask the player to enter their name
	void RequestPlayerName();
	// Print out users cards at top of screen
	void DisplayCards();
    // Return the player name - used with server messages
	std::string GetPlayerName() { return playerName; }

private:
	// Reference to XML loader for card info
	XMLScriptManager* xmlManager;
	// Networking manager, handles packets
	NetworkManager* networkManager;

	std::thread* networkUpdates;
	void UpdateNetwork();

	// Flag for determining if this player is the host
	bool bIsHostClient;
	// Local players name
	std::string playerName;

	// Store what cards the player has
	std::vector<int> cards;
	// Question and Answer decks, only accessed by master/host
	std::vector<int> answerDeck;
	std::vector<int> questionDeck;
	// Store current index of array
	int topAnswerCard, topQuestionCard;

	// Shuffle decks of cards before beginning play
	void ShuffleDecks();
	void DealCardsToClient();
};

