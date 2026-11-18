#include <iostream>
#include <SFML/Graphics.hpp>
#include <optional>
using namespace std;

const float CELL = 70.f; // one cell is 70 x 70 pixels 
const float BOARD_X = 20.f, BOARD_Y = 100.f;   // top-left corner of the board 
const float HUD = 80.f; // height of the HUD at the top of the window
const float Points_per_candy = 10.f; // points for each candy removed
const float Rows = 8.f, Cols = 8.f; // number of rows and columns in the board
const float windows_width = 600.f, windows_height = 680.f; // window dimensions
const bool RUN_TESTS = true;
int Moves_left;
int CurrentScore;
int TargetScore;

//states
const int Menu = 0;
const int Playing = 1;
const int GameOver = 2; // given values because they are used in the switch statement
int gameState = Menu; // current state of the game

// key constants
const int KEY_NONE = 0;
const int KEY_ESC = 1;
const int KEY_ENTER = 2;
const int KEY_UP = 3;
const int KEY_DOWN = 4;

// global variables for the game board and the selected candy
int  selRow = -1, selCol = -1; // selected candy's row and column (-1 means no candy is selected) 
bool clicked = false; // whether the mouse is clicked or not
int clickX = 0;
int clickY = 0;
int pressedKey = KEY_NONE;

int board[Rows][Cols] = { 0 };
bool marked[Rows][Cols] = { false };


void handleInput(sf::RenderWindow& window)
{
	while (const std::optional event = window.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
			window.close();
		else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>())
		{
			if (mouse->button == sf::Mouse::Button::Left)
			{
				clickX = mouse->position.x;
				clickY = mouse->position.y;
				clicked = true;
			}
		}
		else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
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
void drawGame(sf::RenderWindow& window)
{
	window.clear(sf::Color(15, 20, 45)); // clear the window with a dark color
	switch (gameState)
	{
	case Menu: // Handle menu state logic

		break;
	case Playing: // Handle playing state logic

		break;
	case GameOver: // Handle game over state logic

		break;
	}
	window.display(); // display the contents of the window
}
int findMatches()
{
	for (int r=0;r<Rows;r++)
	{
		for (int c = 0;c < Cols;c++)
			marked[r][c] = false; //marked all as false 
		
	}
	for (int r = 0;r < Rows;r++)
	{
		for (int c = 0;c <= Cols - 3;c++)
		{
			if (board[r][c] != 0 &&
				board[r][c] == board[r][c + 1] && board[r][c + 2]) //if match,marked=true
			{
				markes[r][c] = true;
				marked[r][c + 1] = true;
				marked[r][c + 2] = true;
			}
				
		}
	}
	int count = 0;
	for (int r = 0;r < Rows;r++)
	{
		for (int c = 0;c < Cols;c++)
		{
			if (marked[r][c])
				count++;
		}
		//count how many cells are true


	}return count;
}
void testEngine()

{
	cout << "DAY 2 Match TEST" << endl;

	for (int r = 0;r < Rows;r++)
	{
		for (int c = 0;c < Cols;c++)
			board[r][c] = 0;


	}
	//test for 3 rows
	board[0][2] = 1; 
	board[0][3] = 1;
	board[0][4] = 1;
	//test 2
	board[0][2] = 2;
	board[0][3] = 2;
	//test 3
}


int main()
{
	srand(static_cast<unsigned int>(time(0)); 

	sf::RenderWindow window(sf::VideoMode({ 600, 680 }), "Candy Crush"); //resolution of the window
	window.setFramerateLimit(60); //fps limit

	sf::Color colors[7] = { sf::Color(25, 25, 45),
		// 0: empty 
		sf::Color::Red,
		// 1 
		sf::Color(255, 150, 0),
		// 2: orange 
		sf::Color::Yellow,
		// 3 
		sf::Color::Green,
		sf::Color::Blue,
		// 5 
		sf::Color(170, 90, 255) }; // 6: purple 
	sf::CircleShape candy(28.f);
	// 4 
	// radius 28, inside a 70 x 70 cell 
	// for the candy at row r and column c: 
	candy.setFillColor(colors[board[r][c]]);
	candy.setPosition({ BOARD_X + c * CELL + 7, BOARD_Y + r * CELL + 7 });
	window.draw(candy);

	if (!gamefont.openFromFile("arial.ttf"))
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
