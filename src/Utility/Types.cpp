#include "Types.h"
#include "Functions.h"

using namespace std::chrono_literals;

ComplexNumberAtScreenPos::ComplexNumberAtScreenPos(Complex number, MinMax<int> screen_bounds)
    : number { number }
    , pos { static_cast<float>(screen_bounds.min), static_cast<float>(screen_bounds.max) }
{ }

DrawableNumber::DrawableNumber(ComplexNumberAtScreenPos number_at_pos, PlaneBorders<int> draw_bounds)
{
    if (!is_load)
    {
        s << std::setprecision(25);
        is_load = true;
        if (!font.loadFromFile("arial.ttf"))
        {
            throw std::logic_error("Font not loaded");
        }
    }

    text.setString(std::format("{:.25f} + {:.25f}i", number_at_pos.number.re, number_at_pos.number.im).c_str());
    text.setFont(font);
    text.setCharacterSize(14);
    text.setFillColor(sf::Color(255, 255, 255));

    text_bounds = text.getLocalBounds();
    float width = text_bounds.width;
    float height = text_bounds.height;
    float offset_x = draw_bounds.x.lerp(static_cast<int>(number_at_pos.pos.x), MinMax<float>(0, width));
    float offset_y = draw_bounds.y.lerp(static_cast<int>(number_at_pos.pos.y), MinMax<float>(0, 1.5f * height));
    text.setPosition(number_at_pos.pos.x - offset_x, number_at_pos.pos.y - offset_y);
}

Timer::Timer()
    : last_time_point { 0s }
{ }

Real Axis::screenToCartesianX(int x) const
{
    return screen_borders.x.lerp(x, cartesian_borders.x);
}

Real Axis::screenToCartesianY(int y) const
{
    return screen_borders.y.lerp(y, cartesian_borders.y);
}
