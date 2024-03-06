#ifndef MANDELBROT_CPP_MAINWINDOW_H
#define MANDELBROT_CPP_MAINWINDOW_H

#include "SFML/Graphics.hpp"
#include <unordered_map>
#include "Fractal/MandelbrotFractal.h"
#include "Fractal/Zoomer.h"
#include "Fractal/Config.h"

class MainWindow
{
public:
    explicit MainWindow(const ProgramConfig& program_config);

    void startLoop();

private:
    void handleInput();
    void initSfmlEventHandler();
    void handleProgramClose(const sf::Event&);
    void handlePressedKeyKeyboard(const sf::Event& e);
    void handlePressedKeyMouse(const sf::Event& e);
    void handleMouseWheelScroller(const sf::Event& e);

    void handleMouseButtonMoving();
    void handleMouseButtonIsPressing();

    void update();

    void draw();

private:
    bool is_program_work = true;

    using WindowEventHandlerFunc = decltype(&MainWindow::handleProgramClose);
    std::unordered_map<sf::Event::EventType, WindowEventHandlerFunc> sfml_event_handler;

    bool is_fractal_recalc_needed = true;
    sf::RenderWindow window;
    Axis axis;
    Zoomer zoomer;
    Timer zoom_rect_change_timeout;
    MandelbrotFractal mandelbrot_fractal;
};


#endif //MANDELBROT_CPP_MAINWINDOW_H
