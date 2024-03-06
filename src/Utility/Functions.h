#ifndef MANDELBROT_CPP_FUNCTIONS_H
#define MANDELBROT_CPP_FUNCTIONS_H

#include <gmpxx.h>
#include <chrono>
#include <SFML/Graphics.hpp>

template<class To, class From>
To convert(From const &from)
{
    return static_cast<To>(from);
}

template<class To>
To convert(mpf_class const &from)
{
    if constexpr (std::is_same_v<To, sf::Uint8>)
    {
        return static_cast<sf::Uint8>(from.get_ui());
    }
    else if constexpr (std::is_same_v<To, int>)
    {
        return static_cast<int>(from.get_si());
    }
    else if constexpr (std::is_same_v<To, float>)
    {
        return static_cast<float>(from.get_d());
    }
    else if constexpr (std::is_same_v<To, mpf_class>)
    {
        return from;
    }
    else
    {
        throw std::logic_error("Unexpected ConvertTo type");
    }
}

auto check_speed(auto f) {
    auto a = std::chrono::steady_clock::now();
    f();
    auto b = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(b - a).count();
}


#endif //MANDELBROT_CPP_FUNCTIONS_H
