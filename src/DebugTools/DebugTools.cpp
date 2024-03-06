#include <sstream>
#include <iomanip>
#include <SFML/Graphics.hpp>
#include "DebugTools.h"

Invokes::Invokes()
{
    std::lock_guard lock(m);
    thread_number = number;
    ++number;
}

void setCursor(int row, int col, int background, int foreground)
{
    static HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    static struct S
    {
        S()
        {
            MoveWindow(GetConsoleWindow(), 100, 100, 1256, 600, TRUE);
        }
    } set_size;
    SetConsoleCursorPosition(console, { short(col), short(row) });
    SetConsoleTextAttribute(console, byte(background << 4u | foreground));
}

void DrawableNumber::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(text, states);
}
