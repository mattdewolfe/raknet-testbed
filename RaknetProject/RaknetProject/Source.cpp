#include "GameManager.h"

GameManager* gameManager;

int main(void)
{
	gameManager = new GameManager();
	gameManager->Init();	
		
	// After manager is started, just keep passing info to it
	char ch=0;
	while (1)
	{
		if (_kbhit())
		{
			ch=_getch();
			gameManager->KeyPress(ch);
			if (gameManager->state == QUIT)
				break;
		}
	}
	gameManager->Shutdown();
}




