#include "GameManager.h"

GameManager::GameManager(void) :
	state(LOADING),
	bIsHostClient(false),
	bIsQuestionAsker(false),
	topQuestionCard(0),
	topAnswerCard(0),
	currentQuestionCard(0),
	points(0)
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

}

// Takes in answer value and sends to peers
void GameManager::SubmitAnswer(int _inputVal)
{
	if (_inputVal >= 0 && _inputVal < HAND_SIZE)
	{
		// Grab that card reference number
		int temp = RemoveCardFromHand(_inputVal);
		// Let the user know they have played that card
		printout << "\n : Game : You submitted the following card -> " 
			<< xmlManager->GetAnswerCardText(temp) endline
		// Forward card selection to server
		networkManager->PeerToPeerMessage(playerName,
			ID_SEND_ANSWER_CARD,
			UNASSIGNED_SYSTEM_ADDRESS,
			temp,
			true);
		networkManager->SetEventState(ID_SEND_ANSWER_CARD, true);
		state = WAITING_FOR_PLAYERS;
	}
}

// Should loop listening for key input and calling function as needed
void GameManager::KeyPress(const char _ch)
{
	printout << _ch;
	// Iterate through key press options and take action based on that
	if (_ch == 'p' || _ch == 'P')
	{
		Shutdown();
	}
	// if they are waiting to submit an answer
	else if (state == SUBMIT_ANSWER)
	{
		// Convert this to an int for index access
		int tempInt = InputToInt(_ch) - 1;
		// And their input is within hand size
		SubmitAnswer(tempInt);
	}
	// key presses when waiting for leader to select winning answer
	else if (state == SELECTING_BEST_ANSWER)
	{
		if (bIsQuestionAsker == true)
		{
			// Convert this to an int for index access
			int tempInt = InputToInt(_ch) - 1;
			// Was the selected value within range of player count
			if (tempInt >= 0 && tempInt < networkManager->GetNumberOfConnections())
			{
				state = SCORING;
				bIsQuestionAsker = false;
				
				// Sends award point message to winning player
				networkManager->PeerToPeerMessage(playerName, 
				ID_AWARD_POINT,
				networkManager->answerInfo[tempInt].address);

				// Sends reply that was selected to all players
				networkManager->PeerToPeerMessage(playerName, 
				ID_REPLY_CHOICE,
				UNASSIGNED_SYSTEM_ADDRESS,
				networkManager->answerInfo[tempInt].submittedAnswer, 
				true);
				
				DisplayAnswersAndWinner(networkManager->answerInfo[tempInt].submittedAnswer);
			}
		}
	}
	// If we are in the scoring state
	else if (state == SCORING)
	{
		if (_ch == 'r' || _ch == 'R')
		{
			networkManager->SetEventState(ID_READY_FOR_NEXT_ROUND, true);
		}
	}
}

// Adds a card to this players hand
void GameManager::AddCardToHand(int _cardNum)
{
	if (cards.size() < 3)
	{
		cards.push_back(_cardNum);
	}
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

// Sends a card to a specified player
void GameManager::DealPlayersXCards(int _x)
{
	// If this is the host
	if (networkManager->IsHost() == true)
	{
		// And then send cards to all systems
		for (int i = 0; i < _x; i++)
		{
			// Give myself a card
			AddCardToHand(answerDeck[topAnswerCard%answerDeck.size()]);
			topAnswerCard++;
			for (int i = 0; i < networkManager->GetNumberOfConnections(); i++)
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
	}
}

// Clears screen and displays info for this round
void GameManager::StartNextRound(int _questionCard)
{
	state = SUBMIT_ANSWER;
	DrawScreenUI();
	printout << "\n - A new round has begun. -" endline
		
	if (bIsQuestionAsker == false)
	{
		printout << "\n: Game : Complete the following." endline
	}

	// Printout the question for this round
	currentQuestionCard = _questionCard;
	printout << "\n->" << xmlManager->GetQuestionCardText(currentQuestionCard) endline

	// If this is the question asker, then they do not get to select a card
	// and go immediately to waiting for other players to submit
	if (bIsQuestionAsker == true)
	{
		state = WAITING_FOR_ANSWERS;
		networkManager->SetEventState(ID_SEND_ANSWER_CARD, true);
		printout << "\n Waiting for answers..." endline
	}
}

// Clears screen and presents UI information (cards/score) to player
void GameManager::DrawScreenUI()
{
	system("cls");
	printout << " - - - - - - -  Offensive Card Game #213  - - - - - - -" endline
	printout << " - Score: " << points endline
	printout << " - Cards" endline
	// Iterate through players cards and display them at the start of a round
	for (int i = 0; i < cards.size(); i++)
	{
		printout << "   " << i+1 << ": " << xmlManager->GetAnswerCardText(cards[i]) endline
	}
	printout << " - - - - - - - " endline
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
		DealPlayersXCards(3);
		// Send message to start the round, along with 
		// number of next question card
		currentQuestionCard = questionDeck[topQuestionCard%questionDeck.size()];
		networkManager->PeerToPeerMessage(playerName, 
			ID_START_NEXT_ROUND, 
			UNASSIGNED_SYSTEM_ADDRESS,
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
	networkUpdates = new std::thread(&NetworkManager::CheckPackets, networkManager);

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
	system("cls");
	printout << ": " << playerName << " : You are ready." endline
}

// Populate and shuffle the decks
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

// Print out answer cards to question asker
void GameManager::ShowAnswerCardsToAsker()
{
	DrawScreenUI();
	if (bIsQuestionAsker == true)
	{
		state = SELECTING_BEST_ANSWER;
		printout << "\n : Game : Select the best answer!" endline
		for (int i = 0; i < networkManager->GetNumberOfAnswers(); i++)
		{
			printout << " : " << i+1 << " : " << xmlManager->GetAnswerCardText(
				networkManager->answerInfo[i].submittedAnswer) endline
		}
	}
	else
	{
		printout << " : Game : Waiting for leader to select a winning answer." endline
	}
}

// Displays all answers cards, to all players - as well a s
void GameManager::DisplayAnswersAndWinner(int _winnerReference)
{
	DrawScreenUI();
	printout << xmlManager->GetQuestionCardText(currentQuestionCard) endline
	// Iterate through all submitted answers
	for (int i = 0; i < networkManager->GetNumberOfAnswers(); i++)
	{
		int tempInt = networkManager->answerInfo[i].submittedAnswer;
		printout << "  -> " << xmlManager->GetAnswerCardText(tempInt) endline
	}
	// Display winning answer
	printout << "\n And the winner is -> " << xmlManager->GetAnswerCardText(_winnerReference) endline
	printout << "\n\n : Game : Press R to start next round." endline
	state = SCORING;
	// Send everyone 1 additional card
	DealPlayersXCards(1);
}

// Returns the xml reference value of the next card value
int GameManager::GetNextQuestionCard()
{
	currentQuestionCard++;
	return currentQuestionCard%questionDeck.size();
}

// Award a point to this player
void GameManager::AwardPoint()
{
	points += 1;
}

void GameManager::Shutdown()
{
	networkManager->Destroy();
	exit(0);
}
