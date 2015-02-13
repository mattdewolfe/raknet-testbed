#include "GameManager.h"

GameManager::GameManager(void) :
	state(LOADING),
	bIsHostClient(false),
	topQuestionCard(0),
	topAnswerCard(0)
{
	xmlManager = new XMLScriptManager();
	xmlManager->Init();
	networkManager = new NetworkManager(this);
}

GameManager::~GameManager(void)
{

}

void GameManager::DisplayCards()
{
	system("cls");
	std::cout << "You have the following cards. \n";
	// Iterate through players cards and display them at the start of a round
	for (int i = 0; i < cards.size(); i++)
	{
		std::string cardInfo = "a";
		cardInfo += std::to_string(cards[i]);
		const char *cstr = cardInfo.c_str();
		std::string card = xmlManager->GetStringVariableFromScript("answers", cstr);
		std::cout << i+1 << ": " << card << "\n";
	}
}

// Should loop listening for key input and calling function as needed
void GameManager::KeyPress(const char _ch)
{
	// Iterate through key press options and take action based on that
	if (_ch == 'q' || _ch == 'Q')
	{
		state = QUIT;
	}
	// If they are waiting to submit an answer
	else if (state == SUBMIT_ANSWER)
	{
		// And their input is within hand size
		if (_ch > 0 && _ch < HAND_SIZE)
		{
			// Grab that card and forward to server
			int temp = RemoveCardFromHand(_ch);
			state = WAITING_FOR_PLAYERS;
		}
	}
	// If we are in the scoring state
	else if (state == SCORING)
	{
		// And this is the host client
		if (bIsHostClient == true)
		{
			// Then get an answer for the best card
		}
	}
}

void GameManager::AddCardToHand(int _cardNum)
{
	cards.push_back(_cardNum);
	DisplayCards();
}

// Return a card number from their hand to pass to server
int GameManager::RemoveCardFromHand(int _choice)
{
	// Ensure the card they want is within hand size
	if (_choice < cards.size() && _choice > -1)
	{
		// Store it to a temp variable
		int temp = cards[_choice];
		// Assign last card to this position
		cards[_choice] = cards.back();
		// Pop back card from array
		cards.pop_back();
		// And return the temp we created
		return temp;
	}
	// Otherwise card selection is not possible, return -1
	return -1;
}

void GameManager::DealCardsToClient()
{

}

// Clear screen and display information to start the round
void GameManager::StartRound()
{
	system("cls");
	std::cout << " -- A new round has begun. --\n";
	std::cout << "You have the following cards. \n";
	// If this is the host, deal cards to each player
	if (networkManager->IsHost() == true)
	{
		// First deal 3 cards to self from back of answer vector
		for (int i = 0; i < 3; i++)
		{
			AddCardToHand(answerDeck[topAnswerCard%answerDeck.size()]);
			topAnswerCard++;
		}
		// Next do the same for each connected system
		for (int i = 0; i < networkManager->GetNumberOfConnections(); i++)
		{
			// Each system needs 3 cards
			for (int j = 0; j < 3; j++)
			{
				// Send a peer to peer message to a specific client
				// get machine address from manager
				// and get card value from top of card answer deck
				networkManager->PeerToPeerMessage(ID_DEAL_CARD_TO_PLAYER,
					networkManager->GetConnectedMachine(i), 
					answerDeck[topAnswerCard%answerDeck.size()]);
				topAnswerCard++;
			}
		}	
	}
	// Force a delay while waiting for cards
	else
	{
		Sleep(200);
	}
}

// Basic function to get the local players name
// and ensure they don't want their name to be network, like a silly chump
void GameManager::RequestPlayerName()
{
	std::cout << "Enter your name: ";
	std::cin >> playerName;
	std::cin.clear();
	char temp[5];
	gets_s(temp);
	if (playerName == "Network" || playerName == "NETWORK" || playerName == "network")
	{
		std::cout << "You are not a network.\n";
		RequestPlayerName();
	}
}

// Start up networking and join a game
// This is a big sloppy function we may want to refine into smaller parts
void GameManager::Init()
{
	RequestPlayerName();

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
	networkUpdates = new std::thread(&GameManager::UpdateNetwork, this);
	networkUpdates->detach();
	// If this is the host, we need to shuffle decks of cards
	if (networkManager->IsHost() == true)
	{
		ShuffleDecks();
	}
	// Wait for a key press
	std::cout << ": " << playerName << " : Press Enter when you are ready to play.\n";
	getchar();
	// Once hit, let the other clients know this player is ready
	networkManager->SetEventState(GameMessages::ID_READY_TO_PLAY, true);
}

void GameManager::ShuffleDecks()
{
	// Get size of question deck
	int questionSize = xmlManager->GetIntVariableFromScript("questions", "questionCount");
	// Add ints to deck based on size
	for (int i = 0; i < questionSize; i++)
	{
		questionDeck.push_back(i);
	}
	// Repeat this for the answer deck
	int answerSize = xmlManager->GetIntVariableFromScript("answers", "answerCount");
	for (int i = 0; i < answerSize; i++)
	{
		answerDeck.push_back(i);
	}

	// Shuffle both decks
	std::random_shuffle(questionDeck.begin(), questionDeck.end());
	std::random_shuffle(answerDeck.begin(), answerDeck.end());

}
void GameManager::UpdateNetwork()
{
	networkManager->CheckPackets();
}

void GameManager::Shutdown()
{
	networkManager->Destroy();
}
