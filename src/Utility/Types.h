#ifndef MANDELBROT_CPP_TYPES_H
#define MANDELBROT_CPP_TYPES_H

#include <gmpxx.h>
#include <SFML/Graphics.hpp>
#include <chrono>
#include "Functions.h"

using Real = long double;

struct Complex
{
    Real re, im;
};

template<class T>
struct MinMax
{
    MinMax(T min, T max)
        : min(min)
        , max(max)
    { }

    [[nodiscard]] T clamp(T value) const
    {
        return
            value < min ? min :
            value > max ? max :
            value;
    }

    [[nodiscard]] T avg() const
    {
        return (min + max) / 2;
    }

    template<class U>
    U lerp(T value_from, MinMax<U> to_range) const
    {
        Real result = (Real(value_from) - this->min)
            / (this->max - this->min)
            * (to_range.max - to_range.min)
            + to_range.min;
        return convert<U>(result);
    }

    [[nodiscard]] sf::Color lerp(T x, MinMax<sf::Color> to_color_range) const
    {
        sf::Uint8 r = lerp(x, MinMax<sf::Uint8> { to_color_range.min.r, to_color_range.max.r });
        sf::Uint8 g = lerp(x, MinMax<sf::Uint8> { to_color_range.min.g, to_color_range.max.g });
        sf::Uint8 b = lerp(x, MinMax<sf::Uint8> { to_color_range.min.b, to_color_range.max.b });
        return sf::Color { r, g, b };
    }

    T min = { };
    T max = { };
};


template<class T>
struct PlaneBorders
{
    PlaneBorders(MinMax<T> x, MinMax<T> y)
        : x(x)
        , y(y)
    { }

    MinMax<T> x;
    MinMax<T> y;
};

class Axis
{
public:
    [[nodiscard]] Real screenToCartesianX(int x) const;
    [[nodiscard]] Real screenToCartesianY(int y) const;

    PlaneBorders<Real> cartesian_borders;
    PlaneBorders<int> screen_borders;
};

struct ComplexNumberAtScreenPos
{
    ComplexNumberAtScreenPos(Complex number, MinMax<int> screen_bounds);

    Complex number { };
    sf::Vector2f pos;
};

struct DrawableNumber : public sf::Drawable
{
    DrawableNumber(ComplexNumberAtScreenPos number_at_pos, PlaneBorders<int> draw_bounds);

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    sf::Text text;
    sf::FloatRect text_bounds;
    inline static bool is_load { false };
    inline static sf::Font font;
    inline static std::stringstream s;
};

struct Timer
{
    using Clock = std::chrono::steady_clock;

    explicit Timer();

    template<class TimeUnit>
    bool restartWhenExpired(TimeUnit interval)
    {
        if (Clock::now() - last_time_point < interval)
        {
            return false;
        }
        last_time_point = Clock::now();
        return true;
    }

    std::chrono::time_point<Clock> last_time_point;
};

#endif //MANDELBROT_CPP_TYPES_H
