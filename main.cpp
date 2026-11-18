#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <optional>
#include <string>

using namespace std;

const float CELL = 70.f;								   // one cell is 70 x 70 pixels
const float BOARD_X = 20.f, BOARD_Y = 100.f;			   // top-left corner of the board
const float HUD = 80.f;									   // height of the HUD at the top of the window
const int Points_per_candy = 10;					   // points for each candy removed
const int NUM_COLORS = 6;
const int Rows = 8, Cols = 8;							   // number of rows and columns in the board
const float windows_width = 600.f, windows_height = 680.f; // window dimensions
const bool RUN_TESTS = true;

int Moves_left;
int CurrentScore;
int TargetScore;

// states
const int Menu = 0;
const int Playing = 1;
const int GameOver = 2; // given values because they are used in the switch statement
int gameState = Menu;	// current state of the game

// key constants
const int KEY_NONE = 0;
const int KEY_ESC = 1;
const int KEY_ENTER = 2;
const int KEY_UP = 3;
const int KEY_DOWN = 4;

// global variables for the game board and the selected candy
int selRow = -1, selCol = -1; // selected candy's row and column (-1 means no candy is selected)
bool clicked = false;		  // whether the mouse is clicked or not
int clickX = 0;
int clickY = 0;
int pressedKey = KEY_NONE;
int menuIndex = 0;
bool won = false;

int board[Rows][Cols] = {0};
bool marked[Rows][Cols] = {false};

sf::Font gameFont;

void handleInput(sf::RenderWindow &window)
{
	while (const std::optional event = window.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
			window.close();
		else if (const auto *mouse = event->getIf<sf::Event::MouseButtonPressed>())
		{
			if (mouse->button == sf::Mouse::Button::Left)
			{
				clickX = mouse->position.x;
				clickY = mouse->position.y;
				clicked = true;
			}
		}
		else if (const auto *key = event->getIf<sf::Event::KeyPressed>())
		{
			if (key->code == sf::Keyboard::Key::Escape)
			{
				pressedKey = KEY_ESC;
			}
			else if (key->code == sf::Keyboard::Key::Enter)
			{
				pressedKey = KEY_ENTER;
			}
			else if (key->code == sf::Keyboard::Key::Up)
			{
				pressedKey = KEY_UP;
			}
			else if (key->code == sf::Keyboard::Key::Down)
			{
				pressedKey = KEY_DOWN;
			}
		}
	}

	switch (gameState)
	{
	case Menu:
		break;
	case Playing:
		break;
	case GameOver:
		break;
	}
}
void updateGame()
{
	switch (gameState)
	{
	case Menu: // Handle menu state logic

		break;
	case Playing: // Handle playing state logic

		break;
	case GameOver: // Handle game over state logic

		break;
	}
	clicked = false; // reset
	pressedKey = KEY_NONE;
}
void drawGame(sf::RenderWindow &window)
{
	window.clear(sf::Color(15, 20, 45)); // clear the window with a dark color
	switch (gameState)
	{
	case Menu: // Handle menu state logic
	{
		// Title
		sf::Text title(gameFont, "CANDY CRUSH", 48);
		title.setFillColor(sf::Color::Cyan);
		title.setStyle(sf::Text::Bold);
		title.setPosition({130.f, 120.f});
		window.draw(title);

		// Menu Options: New Game, Continue, Exit (Member C)
		string options[3] = {"New Game", "Continue", "Exit"};
		for (int i = 0; i < 3; i++)
		{
			string label = (menuIndex == i) ? ("> " + options[i]) : ("  " + options[i]);
			sf::Text item(gameFont, label, 32);
			item.setFillColor((menuIndex == i) ? sf::Color::Yellow : sf::Color::White);
			item.setPosition({200.f, 280.f + i * 65.f});
			window.draw(item);
		}
		break;
	}
	case Playing: // Handle playing state logic
	{
		sf::Text playingText(gameFont, "PLAYING", 40);
		playingText.setFillColor(sf::Color::White);
		playingText.setPosition({210.f, 300.f});
		window.draw(playingText);

		break;
	}
	case GameOver: // Handle game over state logic
	{
		sf::Text overText(gameFont, "GAME OVER", 40);
		overText.setFillColor(sf::Color::Red);
		overText.setPosition({180.f, 300.f});
		window.draw(overText);
		break;
	}
	}
	window.display(); // display the contents of the window
}
int findMatches()
{
	for (int r = 0; r < Rows; r++)
	{
		for (int c = 0; c < Cols; c++)
			marked[r][c] = false; // marked all as false
	}
	for (int r = 0; r < Rows; r++)
	{
		for (int c = 0; c <= Cols - 3; c++)
		{
			if (board[r][c] != 0 &&
				board[r][c] == board[r][c + 1] && board[r][c + 2]) // if match,marked=true
			{
				marked[r][c] = true;
				marked[r][c + 1] = true;
				marked[r][c + 2] = true;
			}
		}
	}
	int count = 0;
	for (int r = 0; r < Rows; r++)
	{
		for (int c = 0; c < Cols; c++)
		{
			if (marked[r][c])
				count++;
		}
		// count how many cells are true
	}
	return count;
}
void testEngine()

{
	cout << "DAY 2 Match TEST" << endl;

	for (int r = 0; r < Rows; r++)
	{
		for (int c = 0; c < Cols; c++)
			board[r][c] = 0;
	}
	// test for 3 rows
	board[0][2] = 1;
	board[0][3] = 1;
	board[0][4] = 1;
	// test 2
	board[0][2] = 2;
	board[0][3] = 2;
	// test 3

	int matchedCount = findMatches();
	cout << "Total matched cells (Expected 3): " << matchedCount << endl
		 << endl;

	cout << "Board:" << endl;
	for (int r = 0; r < Rows; r++)
	{
		for (int c = 0; c < Cols; c++)
		{
			cout << board[r][c] << " ";
		}
		cout << endl;
	}

	cout << "\nMarked (1 = true, 0 = false):" << endl;
	for (int r = 0; r < Rows; r++)
	{
		for (int c = 0; c < Cols; c++)
		{
			cout << marked[r][c] << " ";
		}
		cout << endl;
	}
}

int main()
{
	srand(static_cast<unsigned int>(time(0)));

	sf::RenderWindow window(sf::VideoMode({600, 680}), "Candy Crush"); // resolution of the window
	window.setFramerateLimit(60);									   // fps limit

	if (!gameFont.openFromFile("arial.ttf"))
	{
		cerr << "Error: Could not load font\n"; // error handling; cerr is used for error messages
	}

	while (window.isOpen()) // main game loop yayyyy
	{
		handleInput(window);
		updateGame();
		drawGame(window);
	}

	return 0;
}
