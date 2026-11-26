#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

using namespace std;

const float CELL = 70.f;					 // one cell is 70 x 70 pixels
const float BOARD_X = 20.f, BOARD_Y = 100.f; // top-left corner of the board
const float HUD = 80.f;						 // height of the HUD at the top of the window
const int Points_per_candy = 10;			 // points for each candy removed
const int NUM_COLORS = 6;
const int Rows = 8, Cols = 8;							   // number of rows and columns in the board
const float windows_width = 600.f, windows_height = 680.f; // window dimensions
const bool RUN_TESTS = true;


const int SHUFFLE_FRAMES = 60;   // about one second at 60 FPS
bool shuffling = false;
int shuffleTimer = 0;
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
// ---------- animation ----------used AI
enum AnimPhase
{
	AnimNone,
	AnimSwap,
	AnimBadSwap,
	AnimClear,
	AnimFall
};
AnimPhase animPhase = AnimNone;

float offX[Rows][Cols] = {0}; // visual pixel offset of each candy (slides to 0)
float offY[Rows][Cols] = {0};
float clearT = 0.f; // 0..1 progress of the "pop" animation
float badT = 0.f;	// 0..1 progress of the failed-swap wiggle
int swapR1, swapC1, swapR2, swapC2;
int combo = 0;
float gameTime = 0.f;

struct Particle
{
	float x, y, vx, vy, life;
	sf::Color col;
};
vector<Particle> particles;

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
sf::Texture bgTexture;

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
				CurrentScore += POINTS_PER_CANDY;
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

void fillBoard()
{
	for (int r = 0; r < Rows; r++)
	{
		for (int c = 0; c < Cols; c++)
		{
			int candy;
			do
			{
				candy = rand() % NUM_COLORS + 1; // 1 to 6, 0 is empty
			} while ((c >= 2 && board[r][c - 1] == candy && board[r][c - 2] == candy) ||
				(r >= 2 && board[r - 1][c] == candy && board[r - 2][c] == candy));
			board[r][c] = candy;
		}
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
	animPhase = AnimNone;
	particles.clear();
	for (int r = 0; r < Rows; r++)
		for (int c = 0; c < Cols; c++)
			offX[r][c] = offY[r][c] = 0.f;
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
	if (x < BOARD_X || x >= BOARD_X + Cols * CELL 
	|| y < BOARD_Y || y >= BOARD_Y + Rows * CELL)
		return false;

	col = (x - BOARD_X) / CELL;
	row = (y - BOARD_Y) / CELL;
	return true;
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
float approach(float v, float amt)
{
	if (v > 0)
		return max(0.f, v - amt);
	if (v < 0)
		return min(0.f, v + amt);
	return 0.f;
}

// returns true while any candy is still sliding
bool updateOffsets(float dt)
{
	bool moving = false;
	float step = 650.f * dt; // pixels per second
	for (int r = 0; r < Rows; r++)
		for (int c = 0; c < Cols; c++)
			if (offX[r][c] != 0.f || offY[r][c] != 0.f)
			{
				offX[r][c] = approach(offX[r][c], step);
				offY[r][c] = approach(offY[r][c], step);
				if (offX[r][c] != 0.f || offY[r][c] != 0.f)
					moving = true;
			}
	return moving;
}

void spawnParticles(int r, int c)
{
	float cx = BOARD_X + c * CELL + CELL / 2;
	float cy = BOARD_Y + r * CELL + CELL / 2;
	for (int i = 0; i < 8; i++)
	{
		float a = (rand() % 360) * 3.14159f / 180.f;
		float s = 80.f + rand() % 160;
		particles.push_back({cx, cy, cos(a) * s, sin(a) * s, 0.5f, colors[board[r][c]]});
	}
}

void updateParticles(float dt)
{
	for (size_t i = 0; i < particles.size();)
	{
		Particle &p = particles[i];
		p.life -= dt;
		if (p.life <= 0.f)
			particles.erase(particles.begin() + i);
		else
		{
			p.x += p.vx * dt;
			p.y += p.vy * dt;
			p.vy += 300.f * dt; // gravity
			i++;
		}
	}
}

// like dropCandies() + refill(), but sets offsets so candies fall smoothly
void dropAndRefillAnimated()
{
	for (int c = 0; c < Cols; c++)
	{
		int writeRow = Rows - 1;
		for (int r = Rows - 1; r >= 0; r--)
		{
			if (board[r][c] != 0)
			{
				if (writeRow != r)
				{
					board[writeRow][c] = board[r][c];
					board[r][c] = 0;
					offY[writeRow][c] = (r - writeRow) * CELL; // starts above
				}
				writeRow--;
			}
		}
		int n = writeRow + 1; // number of empty cells at the top
		for (int k = writeRow; k >= 0; k--)
		{
			board[k][c] = rand() % NUM_COLORS + 1;
			offY[k][c] = -n * CELL; // fall in from above the board
		}
	}
}

void startClear()
{
	combo++;
	for (int r = 0; r < Rows; r++)
		for (int c = 0; c < Cols; c++)
			if (marked[r][c])
				spawnParticles(r, c);
	animPhase = AnimClear;
	clearT = 0.f;
}

void updateAnimation(float dt)
{
	switch (animPhase)
	{
	case AnimNone:
		break;
	case AnimSwap: // candies slide into place, then pop the matches
		if (!updateOffsets(dt))
		{
			findMatches();
			startClear();
		}
		break;
	case AnimBadSwap: // wiggle then return
		badT += dt / 0.3f;
		if (badT >= 1.f)
			animPhase = AnimNone;
		break;
	case AnimClear: // candies shrink away
		clearT += dt / 0.25f;
		if (clearT >= 1.f)
		{
			removeMarked();
			dropAndRefillAnimated();
			animPhase = AnimFall;
		}
		break;
	case AnimFall: // candies fall, then check for cascades
		if (!updateOffsets(dt))
		{
			if (findMatches() > 0)
				startClear();
			else
			{
				animPhase = AnimNone;
				checkGameEnd();
			}
		}
		break;
	}
}
void playermove(int r1, int c1, int r2, int c2)
{
	swapR1 = r1;
	swapC1 = c1;
	swapR2 = r2;
	swapC2 = c2;
	if (trySwap(r1, c1, r2, c2))
	{
		Moves_left--;
		combo = 0;
		// board is already swapped; make each candy visually start at the other's cell
		offX[r1][c1] = (c2 - c1) * CELL;
		offY[r1][c1] = (r2 - r1) * CELL;
		offX[r2][c2] = (c1 - c2) * CELL;
		offY[r2][c2] = (r1 - r2) * CELL;
		animPhase = AnimSwap;
	}
	else
	{
		animPhase = AnimBadSwap;
		badT = 0.f;
	}
}
void updateGame(float dt)
{
	gameTime += dt;
	updateParticles(dt);
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
		if (pressedKey == KEY_ESC)//Pressing Esc now sets gameState to Menu
		{
			gameState = Menu;
			break;
		}
		updateAnimation(dt);
		if (animPhase != AnimNone || gameState != Playing)
			break; // ignore clicks while animating
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

	sf::Vector2i mouse = sf::Mouse::getPosition(window);
	int hr, hc;
	if (cellFromClick(mouse.x, mouse.y, hr, hc))
	{
		sf::RectangleShape hover({ CELL, CELL });
		hover.setPosition({ BOARD_X + hc * CELL, BOARD_Y + hr * CELL });
		hover.setFillColor(sf::Color(255, 255, 255, 40));
		window.draw(hover);
	}

	cellBg.setFillColor(sf::Color(20, 25, 55, 150));
	cellBg.setOutlineThickness(1.f);
	cellBg.setOutlineColor(sf::Color(60, 70, 130));

	// clip drawing to the board area
	sf::View clip(sf::FloatRect({BOARD_X, BOARD_Y}, {Cols * CELL, Rows * CELL}));
	clip.setViewport(sf::FloatRect({BOARD_X / windows_width, BOARD_Y / windows_height},
								   {Cols * CELL / windows_width, Rows * CELL / windows_height}));
	window.setView(clip);

	// backgrounds first
	for (int r = 0; r < Rows; r++)
		for (int c = 0; c < Cols; c++)
		{
			cellBg.setPosition({BOARD_X + c * CELL, BOARD_Y + r * CELL});
			window.draw(cellBg);
		}

	// then candies
	for (int r = 0; r < Rows; r++)
	{
		for (int c = 0; c < Cols; c++)
		{
			if (board[r][c] == 0)
				continue;

			float ox = offX[r][c], oy = offY[r][c];

			// failed-swap wiggle
			if (animPhase == AnimBadSwap)
			{
				float s = sin(badT * 3.14159f) * CELL;
				if (r == swapR1 && c == swapC1)
				{
					ox += (swapC2 - swapC1) * s;
					oy += (swapR2 - swapR1) * s;
				}
				else if (r == swapR2 && c == swapC2)
				{
					ox += (swapC1 - swapC2) * s;
					oy += (swapR1 - swapR2) * s;
				}
			}

			float rad = 28.f;
			if (animPhase == AnimClear && marked[r][c])
				rad = 28.f * (1.f - clearT); // shrink away

			bool sel = (r == selRow && c == selCol);
			if (sel)
				rad += 2.f + 2.f * sin(gameTime * 10.f); // pulsing selection

			float cx = BOARD_X + c * CELL + CELL / 2 + ox;
			float cy = BOARD_Y + r * CELL + CELL / 2 + oy;

			candy.setRadius(rad);
			candy.setPosition({cx - rad, cy - rad});
			candy.setFillColor(colors[board[r][c]]);
			candy.setOutlineThickness(sel ? 4.f : 0.f);
			candy.setOutlineColor(sf::Color::White);
			window.draw(candy);
			// shine highlight
			sf::CircleShape shine(rad * 0.25f);
			shine.setFillColor(sf::Color(255, 255, 255, 140));
			shine.setPosition({cx - rad * 0.5f, cy - rad * 0.6f});
			window.draw(shine);
		}
	}

	window.setView(window.getDefaultView()); // back to normal
}
void centerText(sf::Text &t, float x, float y)
{
	sf::FloatRect b = t.getLocalBounds();
	t.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
	t.setPosition({x, y});
}
void drawGame(sf::RenderWindow &window)
{
	window.clear(sf::Color(15, 20, 45)); // clear the window with a dark color
	static sf::Sprite bg(bgTexture);
	bg.setScale({windows_width / bgTexture.getSize().x,
				 windows_height / bgTexture.getSize().y});
	window.draw(bg);
	switch (gameState)
	{
	case Menu:
	{
		// floating candy bubbles
		sf::CircleShape bubble;
		for (int i = 0; i < 16; i++)
		{
			float rad = 10.f + (i % 4) * 6.f;
			float x = 40.f + (i * 137) % 520 + 15.f * sin(gameTime + i);
			float y = windows_height + 40.f -
					  fmod(gameTime * (25.f + (i % 5) * 12.f) + i * 70.f, windows_height + 80.f);
			sf::Color col = colors[i % 6 + 1];
			col.a = 150;
			bubble.setRadius(rad);
			bubble.setOrigin({rad, rad});
			bubble.setPosition({x, y});
			bubble.setFillColor(col);
			window.draw(bubble);
		}

		// title: shadow + outlined text that pulses
		float pulse = 1.f + 0.05f * sin(gameTime * 3.f);
		float ty = 160.f + 8.f * sin(gameTime * 2.f);

		sf::Text shadow(gameFont, "CANDY CRUSH", 64);
		shadow.setStyle(sf::Text::Bold);
		shadow.setFillColor(sf::Color(0, 0, 0, 160));
		centerText(shadow, 306.f, ty + 6.f);
		shadow.setScale({pulse, pulse});
		window.draw(shadow);

		sf::Text title(gameFont, "CANDY CRUSH", 64);
		title.setStyle(sf::Text::Bold);
		title.setFillColor(sf::Color((uint8_t)(200 + 55 * sin(gameTime * 2.f)),
									 (uint8_t)(120 + 100 * sin(gameTime * 2.f + 2.f)),
									 (uint8_t)(200 + 55 * sin(gameTime * 2.f + 4.f))));
		title.setOutlineColor(sf::Color::White);
		title.setOutlineThickness(3.f);
		centerText(title, 300.f, ty);
		title.setScale({pulse, pulse});
		window.draw(title);

		// buttons
		string options[3] = {"New Game", "Continue", "Exit"};
		for (int i = 0; i < 3; i++)
		{
			bool sel = (menuIndex == i);
			float grow = sel ? 1.f + 0.04f * sin(gameTime * 8.f) : 1.f;
			float by = 330.f + i * 80.f;

			sf::RectangleShape btn({260.f, 60.f});
			btn.setOrigin({130.f, 30.f});
			btn.setPosition({300.f, by});
			btn.setScale({grow, grow});
			btn.setFillColor(sel ? sf::Color(255, 90, 160, 230) : sf::Color(30, 30, 80, 190));
			btn.setOutlineThickness(3.f);
			btn.setOutlineColor(sel ? sf::Color::Yellow : sf::Color(120, 120, 220));
			window.draw(btn);

			sf::Text item(gameFont, options[i], 32);
			item.setFillColor(sf::Color::White);
			centerText(item, 300.f, by - 4.f);
			item.setScale({grow, grow});
			window.draw(item);
		}

		if (levelError)
		{
			sf::Text err(gameFont, "level.txt missing or invalid", 24);
			err.setFillColor(sf::Color::Red);
			centerText(err, 300.f, 590.f);
			window.draw(err);
		}
		if (noSavedGame)
		{
			sf::Text msg(gameFont, "No saved game", 24);
			msg.setFillColor(sf::Color::Red);
			centerText(msg, 300.f, 630.f);
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
		// particles
		sf::CircleShape dot(4.f);
		for (const Particle &p : particles)
		{
			sf::Color col = p.col;
			col.a = (uint8_t)(255 * (p.life / 0.5f));
			dot.setFillColor(col);
			dot.setPosition({p.x - 4.f, p.y - 4.f});
			window.draw(dot);
		}

		// combo text
		if (combo >= 2 && animPhase != AnimNone)
		{
			sf::Text comboText(gameFont, "COMBO x" + std::to_string(combo) + "!", 30);
			comboText.setFillColor(sf::Color::Yellow);
			comboText.setPosition({220.f, 55.f});
			window.draw(comboText);
		}

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

if (shuffling)
{
	sf::RectangleShape overlay({ 600.f, 680.f });
	overlay.setFillColor(sf::Color(0, 0, 0, 160));
	window.draw(overlay);

	sf::Text msg(font, "Shuffling...", 48);
	msg.setFillColor(sf::Color::White);
	msg.setPosition({ 170.f, 320.f });
	window.draw(msg);
}

int main()
{

	srand(static_cast<unsigned int>(time(0)));

	sf::RenderWindow window(sf::VideoMode({(unsigned)windows_width, (unsigned)windows_height}), "Candy Crush");
	window.setFramerateLimit(60);

	if (!gameFont.openFromFile("font.ttf"))
	{
		cerr << "Error: Could not load font\n";
		return 1; // stop, because drawing text without a font can crash
	}
	if (!bgTexture.loadFromFile("background.jpeg"))
		 cerr << "Could not load background.jpeg\n";

	// stretch the image to fill the window:
	// (skip this block if you use tiling below)

	if (RUN_TESTS)
		testEngine();
	fillBoard();

	sf::Clock clock;
	while (window.isOpen() && !quitGame)
	{
		float dt = clock.restart().asSeconds();
		if (dt > 0.05f)
			dt = 0.05f; // avoid huge jumps
		handleInput(window);
		updateGame(dt);
		drawGame(window);
	}
	return 0;
}
