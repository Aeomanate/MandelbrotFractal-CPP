#ifndef MANDELBROT_CPP_CONFIG_H
#define MANDELBROT_CPP_CONFIG_H

#include "../Utility/Types.h"
#include "FractalCalcMethods.h"

struct ColorTableConfig
{
    MinMax<sf::Color> color_range = { sf::Color::Magenta, sf::Color::Green };
    std::size_t transition_color_smoothness = 40;
    std::size_t transition_colors_count = 2;
};

struct ProgramConfig
{
    MinMax<int> iterations_limit = { 20, 10'000 };
    double initial_zoom_rect_ratio = 0.8;
    sf::VideoMode window_mode = { 1024, 768 };
    ColorTableConfig color_table_config;
    Axis axis = { PlaneBorders<Real> { MinMax<Real> { -2, 1 }, MinMax<Real> { -1, 1 }},
                  PlaneBorders<int> { MinMax<int> { 0, int(window_mode.width) },
                                      MinMax<int> { 0, int(window_mode.height) }}};
    std::shared_ptr<FractalCalcMethod> calc_method = std::make_shared<CalcFractalByRowsParallel>();
};

#endif //MANDELBROT_CPP_CONFIG_H
