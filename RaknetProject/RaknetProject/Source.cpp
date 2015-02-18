#include "GameManager.h"

GameManager* gameManager;

int main(void)
{
	gameManager = new GameManager();
	gameManager->Init();
	// A fun little loop so our threads keep running
	while (1) 
	{}
}




