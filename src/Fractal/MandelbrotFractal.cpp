#include <limits>
#include <iostream>
#include "MandelbrotFractal.h"


MandelbrotFractal::MandelbrotFractal(const ProgramConfig& program_config)
    : current_iterations_count { program_config.iterations_limit.min }
    , limit_iterations { program_config.iterations_limit }
    , calc_method { program_config.calc_method }
{
    fractal_image.image.create(program_config.window_mode.width, program_config.window_mode.height);
    color_table.resize(program_config.color_table_config.transition_color_smoothness);

    MinMax<std::size_t> color_table_index_range = {
        0, std::size(color_table) / program_config.color_table_config.transition_colors_count
    };

    for (std::size_t i = 0; i != std::size(color_table); ++i)
    {
        color_table[i] = color_table_index_range.lerp(i, program_config.color_table_config.color_range);
    }
}
void MandelbrotFractal::update(const Axis& axis)
{
    auto setFractalPixelCallback = [fractal_ptr = this] (std::size_t spent_iterations, int px, int py)
    {
        fractal_ptr->setFractalPixel(spent_iterations, px, py);
    };
    calc_method->calcFractal(static_cast<std::size_t>(current_iterations_count), axis, setFractalPixelCallback);
    fractal_image.updateSprite();
}

void MandelbrotFractal::shiftIterationsCount(int offset)
{
    int updated_iterations_count = current_iterations_count + offset;
    current_iterations_count = limit_iterations.clamp(updated_iterations_count);
}

void MandelbrotFractal::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(fractal_image, states);
}

void MandelbrotFractal::setFractalPixel(std::size_t iterations_spent, int px, int py)
{
    bool is_in_set = iterations_spent == std::numeric_limits<std::size_t>::max();
    const sf::Color& pixel_color = is_in_set
                                   ? sf::Color::Black
                                   : color_table[iterations_spent % std::size(color_table)];
    fractal_image.image.setPixel(unsigned(px), unsigned(py), pixel_color);
}

void FractalImage::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(sprite, states);
}

void FractalImage::updateSprite()
{
    texture.loadFromImage(image);
    sprite.setTexture(texture);
}
