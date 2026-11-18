#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>
using namespace std;

const int POINTS_PER_CANDY = 10;
const int NUM_COLORS = 6;

int score = 0;
int movesLeft = 20;
int targetScore = 800;

int main()
{
    sf::RenderWindow window(sf::VideoMode({600, 680}), "Candy Crush");
    window.setFramerateLimit(60);

    const float CELL = 70.f;
    const float BOARD_X = 20.f, BOARD_Y = 100.f;
    const float r = 8.f;
    const float c = 8.f;
    const float window_width = 600.f;
    const float window_height = 680.f;
    const float HUD_HEIGHT = 80.f;

    sf::Color colors[7] = {
        // one cell is 70 x 70 pixels
        // top-left corner of the board
        sf::Color(25, 25, 45),
        sf::Color::Red,
        sf::Color(255, 150, 0), sf::Color::Yellow,
        sf::Color::Green,
        sf::Color::Blue,
        sf::Color(170, 90, 255)}; // 6: purple

    sf::CircleShape candy(28.f);  // radius 28, inside a 70 x 70 cell
    // for the candy at row r and column c:
    candy.setFillColor(colors[board[r][c]]);
    candy.setPosition({BOARD_X + c * CELL + 7, BOARD_Y + r * CELL + 7});
    window.draw(candy);
}
