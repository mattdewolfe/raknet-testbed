#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <RakThread.h>
#include <thread>
#include <algorithm>
#include <time.h> 
#include "XMLScriptManager.h"
#include "NetworkManager.h"

// Simple definition to avoid using std:: all the time
#define printout std::cout
// Simple definition to improve readability of cout lines
#define endline << "\n";
// Max number of cards the player can hold
#define HAND_SIZE 3

enum GAME_STATE
{
	LOADING = 0, 
	ROUND_START,
	SUBMIT_ANSWER, 
	WAITING_FOR_ANSWERS,
	WAITING_FOR_PLAYERS,
	SELECTING_BEST_ANSWER,
	SCORING, 
	QUIT
};

class GameManager
{
public:

	GameManager(void);
	~GameManager(void);

	GAME_STATE state;
	// Make this player the question asker
	void MakeQuestionAsker() { bIsQuestionAsker = true; }
	// Process player input
	void KeyPress(const char _ch);
	// Add a card to the local players hand
	void AddCardToHand(int _cardNum);
	// Return a card number from their hand to pass to server
	int RemoveCardFromHand(int _choice);
	// Clear screen and deal cards to players
	void StartGame();
	// Takes in value referencing questions from xml
	// and displays round info
	void StartNextRound(int _questionCard);
	// Start up networking and game
	void Init();
	// Shutdown the system
	void Shutdown();
	// Ask the player to enter their name
	void RequestPlayerName();
	// Print out answer cards to question asker
	void DisplayCards();
	// Show answer cards to the question asker, to pick from
	void ShowAnswerCardsToAsker();
	// Award point to this player
	void AwardPoint();
	// Show the answers to all players and the winning answer
	void DisplayAnswersAndWinner(int _winnerReference);
	// Get the next question card value
	int GetNextQuestionCard();
    // Return the player name - used with server messages
	std::string GetPlayerName() { return playerName; }

private:
	// Reference to XML loader for card info
	XMLScriptManager* xmlManager;
	// Networking manager, handles packets
	NetworkManager* networkManager;

	// Network update thread - runs CheckPackets on NetworkManager
	std::thread* networkUpdates;

	// User Input thread - runs ListenForInput on this object
	std::thread* inputListener;
	void ListenForInput();

	// Flag for determining if this player is the host
	bool bIsHostClient;
	// Flag for denoting this player is the question ASKER
	bool bIsQuestionAsker;
		
	// Store what cards the player has
	std::vector<int> cards;
	// Question and Answer decks, only accessed by master/host
	std::vector<int> answerDeck;
	std::vector<int> questionDeck;
	
	// Number of points this player has
	int points;
	// Local players name
	std::string playerName;
	// Store current index of array
	int topAnswerCard, topQuestionCard;
	// Store xml reference value of current question
	int currentQuestionCard;

	// Shuffle decks of cards before beginning play
	void ShuffleDecks();
	// Deal the specified number of cards to all systems
	void DealPlayersXCards(int _x);
	// Convert char input to an int
	int InputToInt(char _in) { return _in - 48; }
};

