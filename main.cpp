#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <optional>
#include <string>
#include <fstream>

using namespace std;

const float CELL = 70.f;					 // one cell is 70 x 70 pixels
const float BOARD_X = 20.f, BOARD_Y = 100.f; // top-left corner of the board
const float HUD = 80.f;						 // height of the HUD at the top of the window
const int Points_per_candy = 10;			 // points for each candy removed
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

bool levelError = false;
bool noSavedGame = false;
bool quitGame = false;

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
bool loadLevel()
{
	ifstream fin("level.txt");
	if (!fin) // file did not open
	{
		levelError = true;
		return false;
	}
	int row, col;
	fin >> row >> col;
	if (row != Rows || col != Cols)
	{
		levelError = true;
		return false;
	}
	fin >> Moves_left;
	fin >> TargetScore;
	CurrentScore = 0;
	selRow = selCol = -1;
	for (int r = 0; r < Rows; r++)
	{
		for (int c = 0; c < Cols; c++)
		{
			fin >> board[r][c];
		}
	}
	if (!fin)
	{
		return false;
	}
	return true;
}
void updateGame()
{
	switch (gameState)
	{
	case Menu: // Handle menu state logic
	{
		if (pressedKey == KEY_DOWN)
		{
			noSavedGame = false;
			levelError = false;
			menuIndex++;
			if (menuIndex == 3)
			{
				menuIndex = 0;
			}
		}
		else if (pressedKey == KEY_UP)
		{
			noSavedGame = false;
			levelError = false;

			menuIndex--;
			if (menuIndex == -1)
			{
				menuIndex = 2;
			}
		}
		else if (pressedKey == KEY_ENTER)
		{
			switch (menuIndex)
			{
			case 0: // new game
			{
				if (loadLevel())
				{
					levelError = false;
					gameState = Playing;
				}
				else
				{
					levelError = true;
				}
				break;
			}
			case 1: // continue
			{
				noSavedGame = true;
				break;
			}
			case 2: // exit
			{
				quitGame = true;
				break;
			}
			}
		}

		break;
	}
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

		if (levelError)
		{
			sf::Text err(gameFont, "level.txt missing or invalid", 24);
			err.setFillColor(sf::Color::Red);
			err.setPosition({150.f, 500.f});
			window.draw(err);
		}
		if (noSavedGame)
		{
			sf::Text msg(gameFont, "No saved game", 24);
			msg.setFillColor(sf::Color::Red);
			msg.setPosition({200.f, 540.f});
			window.draw(msg);
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
				board[r][c] == board[r][c + 1] &&
				board[r][c] == board[r][c + 2]) // if match,marked=true
			{
				marked[r][c] = true;
				marked[r][c + 1] = true;
				marked[r][c + 2] = true;
			}
		}
	}

	for (int r = 0; r <= Rows - 3; r++)
	{
		for (int c = 0; c < Cols; c++)
		{
			if (board[r][c] != 0 &&
				board[r][c] == board[r + 1][c] &&
				board[r][c] == board[r + 2][c]) // if match,marked=true
			{
				marked[r][c] = true;
				marked[r + 1][c] = true;
				marked[r + 2][c] = true;
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
void clearBoard()
{
	for (int r = 0; r < Rows; r++)
		for (int c = 0; c < Cols; c++)
			board[r][c] = 0;
}

void checkTest(const string &name, int expected)
{
	int result = findMatches();
	cout << name << ": got " << result << ", expected " << expected
		 << (result == expected ? "  PASS" : "  FAIL") << endl;
}

void testEngine()
{
	cout << "DAY 3 MATCH TESTS" << endl;

	clearBoard();
	board[0][0] = 1;
	board[0][1] = 1;
	board[0][2] = 1;
	checkTest("Row of 3", 3);

	clearBoard();
	board[0][0] = 2;
	board[1][0] = 2;
	board[2][0] = 2;
	checkTest("Column of 3", 3);

	clearBoard();
	for (int c = 0; c < 4; c++)
		board[3][c] = 4;
	checkTest("Row of 4", 4);

	clearBoard();
	for (int c = 0; c < 5; c++)
		board[3][c] = 4;
	checkTest("Row of 5", 5);

	clearBoard(); // L shape: across 3 and down 3, sharing the corner [2][0]
	board[2][0] = 5;
	board[2][1] = 5;
	board[2][2] = 5;
	board[3][0] = 5;
	board[4][0] = 5;
	checkTest("L shape", 5);

	clearBoard(); // T shape: across 3 on top, down 3 through the middle
	board[2][0] = 6;
	board[2][1] = 6;
	board[2][2] = 6;
	board[3][1] = 6;
	board[4][1] = 6;
	checkTest("T shape", 5);

	clearBoard();
	board[0][0] = 1;
	board[0][1] = 1;
	board[0][2] = 2;
	checkTest("Pair only", 0);

	clearBoard();
	checkTest("Empty cells", 0);

	if (loadLevel())
		checkTest("Sample level", 0);
	else
		cout << "Sample level: could not load level.txt" << endl;
}

int main()
{
	srand(static_cast<unsigned int>(time(0)));

	sf::RenderWindow window(sf::VideoMode({(unsigned)windows_width, (unsigned)windows_height}), "Candy Crush");
	window.setFramerateLimit(60);

	if (!gameFont.openFromFile("arial.ttf"))
	{
		cerr << "Error: Could not load font\n";
		return 1; // stop, because drawing text without a font can crash
	}

	if (RUN_TESTS)
		testEngine();

	while (window.isOpen() && !quitGame)
	{
		handleInput(window);
		updateGame();
		drawGame(window);
	}
	return 0;
}
