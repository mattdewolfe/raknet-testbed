#include "GameManager.h"

GameManager::GameManager(void) :
	state(LOADING),
	bIsHostClient(false),
	bIsQuestionAsker(false),
	topQuestionCard(0),
	topAnswerCard(0),
	currentQuestionCard(0)
{
	xmlManager = new XMLScriptManager();
	srand(time(NULL));
	xmlManager->Init();
	networkManager = new NetworkManager(this);
}

GameManager::~GameManager(void)
{

}

void GameManager::DisplayCards()
{
	system("cls");
	printout << "You have the following cards." endline
	// Iterate through players cards and display them at the start of a round
	for (int i = 0; i < cards.size(); i++)
	{
		printout << i+1 << ": " << xmlManager->GetAnswerCardText(cards[i]) endline
	}
}

// Should loop listening for key input and calling function as needed
void GameManager::KeyPress(const char _ch)
{
	printout << _ch;
	// Iterate through key press options and take action based on that
	if (_ch == 'q' || _ch == 'Q')
	{
		Shutdown();
	}
	// If they are waiting to submit an answer
	else if (state == SUBMIT_ANSWER)
	{
		// And their input is within hand size
		if (_ch > 0 && _ch < HAND_SIZE)
		{
			// Grab that card and forward to server
			int temp = RemoveCardFromHand(_ch);
			printout << " : Game : You submitted the following card: " endline
			printout << " -> " << xmlManager->GetAnswerCardText(_ch);
			networkManager->PeerToPeerMessage(playerName, 
				ID_SEND_ANSWER_CARD,
				networkManager->GetConnectedMachine(0), 
				_ch, 
				false);
			networkManager->SetEventState(ID_SEND_ANSWER_CARD, true);
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
	if (cards.size() > 2)
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

void GameManager::DealCardToClient()
{

}

// Clears screen and displays info for this round
void GameManager::StartNextRound(int _questionCard)
{
	state = SUBMIT_ANSWER;
	DisplayCards();

	printout << "\n - A new round has begun. -" endline
	printout << "\n: Game : Complete the following. " endline
	// Printout the question for this round
	printout << "\n->" << xmlManager->GetQuestionCardText(_questionCard) endline
}

// Clear screen, let players know the game has started
// and send cards to all players
void GameManager::StartGame()
{
	system("cls");
	printout << " - The game has started. -" endline

	// If this is the host, deal cards to each player
	if (networkManager->IsHost() == true)
	{
		bIsQuestionAsker = true;
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
				networkManager->PeerToPeerMessage(playerName, 
					ID_DEAL_CARD_TO_PLAYER,
					networkManager->GetConnectedMachine(i), 
					answerDeck[topAnswerCard%answerDeck.size()]);
				topAnswerCard++;
			}
		}
		// Send message to start the round, along with 
		// number of next question card
		currentQuestionCard = questionDeck[topQuestionCard%questionDeck.size()];
		networkManager->PeerToPeerMessage(playerName, 
			ID_START_NEXT_ROUND, 
			networkManager->GetConnectedMachine(0),
			currentQuestionCard,
			true);
		// Call start round on host, rather than sending a message to myself
		StartNextRound(currentQuestionCard);
		topQuestionCard++;
	}

	// Startup input listener thread
	inputListener = new std::thread(&GameManager::ListenForInput, this);
}

// Key listening thread function. Should constantly pass single key presses on
// for handling - after the game has been started
void GameManager::ListenForInput()
{
	char ch=0;
	while (1)
	{
		if (_kbhit())
		{
			ch=_getch();
			KeyPress(ch);
		}
	}
}

// Basic function to get the local players name
// and ensure they don't want their name to be network, like a silly chump
void GameManager::RequestPlayerName()
{
	printout << "Enter your name: ";
	std::cin >> playerName;
	std::cin.clear();
	// Get a temp to clear return key
	char temp[5];
	gets_s(temp);
	// Create a temp string to check that the name is allowed
	std::string check = playerName;
	std::transform(check.begin(), check.end(), check.begin(), ::tolower);

	if (check == "network" || check == "game")
	{
		printout << "\n : Game : Illegal name choice." endline
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
	printout << "Enter host IP address (or press enter to host): ";
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

	// If this is the host, we need to shuffle decks of cards
	if (networkManager->IsHost() == true)
	{
		ShuffleDecks();
	}
	// Wait for a key press
	printout << ": " << playerName << " : Press Enter when you are ready to play." endline
	getchar();
	// Once hit, let the other clients know this player is ready
	networkManager->SetEventState(GameMessages::ID_READY_TO_PLAY, true);
	networkManager->PeerToPeerMessage(playerName, 
		GameMessages::ID_READY_TO_PLAY, 
		networkManager->GetConnectedMachine(0),
		-1, 
		true);
	printout << ": " << playerName << " : Your are ready." endline
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
	exit(0);
}
