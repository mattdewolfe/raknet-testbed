#include "GameManager.h"

GameManager* gameManager;

int main(void)
{
	gameManager = new GameManager();
	gameManager->Init();
	while (1) { }
}




