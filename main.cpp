#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime>

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

sf::Color colors[7] = {
	sf::Color(25, 25, 45),	// 0: empty
	sf::Color::Red,			// 1
	sf::Color(255, 150, 0), // 2: orange
	sf::Color::Yellow,		// 3
	sf::Color::Green,		// 4
	sf::Color::Blue,		// 5
	sf::Color(170, 90, 255) // 6: purple
};

sf::Font gameFont;

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
void removeMarked()
{
	for (int r = 0; r < Rows; r++)
	{
		for (int c = 0; c < Cols; c++)
		{
			if (marked[r][c])
			{
				board[r][c] = 0;
				CurrentScore += Points_per_candy;
			}
		}
	}
}
void dropCandies()
{
	for (int c = 0; c < Cols; c++)
	{
		int writeRow = Rows - 1;

		for (int r = Rows - 1; r >= 0; r--)
		{
			if (board[r][c] != 0)
			{
				board[writeRow][c] = board[r][c];
				if (writeRow != r)
					board[r][c] = 0;
				writeRow--;
			}
		}

		while (writeRow >= 0)
		{
			board[writeRow][c] = 0;
			writeRow--;
		}
	}
}
void refill()
{
	for (int r = 0; r < Rows; r++)
	{
		for (int c = 0; c < Cols; c++)
		{
			if (board[r][c] == 0)
			{
				board[r][c] = rand() % NUM_COLORS + 1;
			}
		}
	}
}
int resolveBoard()
{
	int totalCleared = 0;

	int matches = findMatches();

	while (matches > 0)
	{
		totalCleared += matches;

		removeMarked();

		dropCandies();

		refill();

		matches = findMatches();
	}

	return totalCleared;
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
bool cellFromClick(int x, int y, int &row, int &col)
{
	if (x < 20 || x >= 580 || y < 100 || y >= 660)
		return false;

	col = (x - BOARD_X) / CELL;
	row = (y - BOARD_Y) / CELL;
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
	{
		int row, col;
		if (clicked)
		{
			if (cellFromClick(clickX, clickY, row, col))
			{
				if (selRow == -1)
				{
					selRow = row;
					selCol = col;
				}
				else if (abs(row - selRow) + abs(col - selCol) == 1)
				{
					playermove(selRow, selCol, row, col);
					selRow = selCol = -1;
				}
				else
				{
					selRow = row;
					selCol = col;
				}
			}
		}
		break;
	}
	case GameOver: // Handle game over state logic
	{
		if (pressedKey == KEY_ENTER)
		{
			gameState = Menu;
		}
		break;
	}
	}
	clicked = false; // reset
	pressedKey = KEY_NONE;
}

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

void drawBoard(sf::RenderWindow &window)
{
	static sf::RectangleShape cellBg({CELL, CELL});
	static sf::CircleShape candy(28.f);

	cellBg.setFillColor(sf::Color(20, 25, 55));
	cellBg.setOutlineThickness(1.f);
	cellBg.setOutlineColor(sf::Color(60, 70, 130));

	for (int r = 0; r < Rows; r++)
	{
		for (int c = 0; c < Cols; c++)
		{
			cellBg.setPosition({BOARD_X + c * CELL, BOARD_Y + r * CELL});
			window.draw(cellBg);

			if (board[r][c] != 0)
			{
				candy.setFillColor(colors[board[r][c]]);

				candy.setPosition({BOARD_X + c * CELL + 7,
								   BOARD_Y + r * CELL + 7});

				if (r == selRow && c == selCol)
				{
					candy.setOutlineThickness(4.f);
					candy.setOutlineColor(sf::Color::White);
				}
				else
				{
					candy.setOutlineThickness(0.f);
				}

				window.draw(candy);
			}
		}
	}
}
bool trySwap(int r1, int c1, int r2, int c2)
{
	int temp = board[r1][c1];
	board[r1][c1] = board[r2][c2];
	board[r2][c2] = temp;
	int matches = findMatches();
	if (matches > 0)
		return true;
	else
	{
		// Swap them back
		board[r2][c2] = board[r1][c1];
		board[r1][c1] = temp;
		return false;
	}
}
void checkGameEnd()
{
	if (CurrentScore >= TargetScore)
	{
		won = true;
		gameState = GameOver;
	}
	else if (Moves_left <= 0)
	{
		won = false;
		gameState = GameOver;
	}
}
void playermove(int r1, int c1, int r2, int c2)
{
	if (trySwap(r1, c1, r2, c2))
	{
		Moves_left--;

		resolveBoard();
		checkGameEnd();
	}
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
	case Playing:
	{
		// HUD background
		sf::RectangleShape hud({windows_width, HUD});
		hud.setPosition({0.f, 0.f});
		hud.setFillColor(sf::Color(10, 15, 35));
		window.draw(hud);

		// SCORE label and value
		sf::Text scoreLabel(gameFont, "SCORE", 18);
		scoreLabel.setFillColor(sf::Color::Cyan);
		scoreLabel.setPosition({65.f, 8.f});
		window.draw(scoreLabel);

		sf::Text scoreValue(gameFont, std::to_string(CurrentScore), 24);
		scoreValue.setFillColor(sf::Color::White);
		scoreValue.setPosition({80.f, 35.f});
		window.draw(scoreValue);

		// MOVES label and value
		sf::Text movesLabel(gameFont, "MOVES", 18);
		movesLabel.setFillColor(sf::Color::Cyan);
		movesLabel.setPosition({265.f, 8.f});
		window.draw(movesLabel);

		sf::Text movesValue(gameFont, std::to_string(Moves_left), 24);
		movesValue.setFillColor(sf::Color::White);
		movesValue.setPosition({280.f, 35.f});
		window.draw(movesValue);

		// TARGET label and value
		sf::Text targetLabel(gameFont, "TARGET", 18);
		targetLabel.setFillColor(sf::Color::Cyan);
		targetLabel.setPosition({465.f, 8.f});
		window.draw(targetLabel);

		sf::Text targetValue(gameFont, std::to_string(TargetScore), 24);
		targetValue.setFillColor(sf::Color::White);
		targetValue.setPosition({480.f, 35.f});
		window.draw(targetValue);

		drawBoard(window);

		break;
	}
	case GameOver: // Handle game over state logic
	{
		sf::Text title(gameFont, "", 45);
		title.setPosition(sf::Vector2f(150, 200));

		if (won)
			title.setString("YOU WIN");
		else
			title.setString("GAME OVER");

		title.setFillColor(sf::Color::Yellow);

		sf::Text finalScore(gameFont, "Final score: " + std::to_string(CurrentScore), 28);

		finalScore.setPosition(sf::Vector2f(190, 300));

		sf::Text instruction(gameFont, "Press Enter", 24);
		instruction.setPosition(sf::Vector2f(220, 400));

		window.draw(title);
		window.draw(finalScore);
		window.draw(instruction);
		break;
	}
	}
	window.display(); // display the contents of the window
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

	cout << "\n1000 RANDOM BOARD TESTS" << endl;

	int passed = 0;
	int failed = 0;

	for (int test = 0; test < 1000; test++)
	{
		// Fill board with random candies
		for (int r = 0; r < Rows; r++)
		{
			for (int c = 0; c < Cols; c++)
			{
				board[r][c] = rand() % NUM_COLORS + 1;
			}
		}

		int oldScore = CurrentScore;

		// Resolve all matches
		int cleared = resolveBoard();

		bool valid = true;

		// Check 1: No matches should remain
		if (findMatches() != 0)
			valid = false;

		// Check 2: No empty cells and no invalid colors
		for (int r = 0; r < Rows; r++)
		{
			for (int c = 0; c < Cols; c++)
			{
				if (board[r][c] < 1 || board[r][c] > NUM_COLORS)
					valid = false;
			}
		}

		// Check 3: Score must increase by 10 per candy
		if (CurrentScore - oldScore != cleared * Points_per_candy)
			valid = false;

		if (valid)
			passed++;
		else
			failed++;
	}

	cout << "Passed: " << passed << endl;
	cout << "Failed: " << failed << endl;

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
