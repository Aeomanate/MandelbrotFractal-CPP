#ifndef MANDELBROT_CPP_MANDELBROTFRACTAL_H
#define MANDELBROT_CPP_MANDELBROTFRACTAL_H

#include <SFML/Graphics.hpp>
#include "../Utility/Functions.h"
#include "../Multithreading/ThreadPool.h"
#include "Config.h"

class FractalImage : public sf::Drawable
{
public:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    void updateSprite();

    sf::Image image;

private:
    sf::Texture texture;
    sf::Sprite sprite;
};

class FractalCalcMethod;

class MandelbrotFractal : public sf::Drawable
{
public:
    explicit MandelbrotFractal(const ProgramConfig& program_config);

    void update(Axis const& axis);

    void shiftIterationsCount(int offset);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    void setFractalPixel(std::size_t iterations_spent, int px, int py);

private:
    int current_iterations_count;
    MinMax<int> limit_iterations;
    std::vector<sf::Color> color_table;
    FractalImage fractal_image;
    std::shared_ptr<FractalCalcMethod> calc_method;
};

#endif //MANDELBROT_CPP_MANDELBROTFRACTAL_H
