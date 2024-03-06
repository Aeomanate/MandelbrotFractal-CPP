#include "MainWindow.h"
#include "Utility/Functions.h"

using namespace std::chrono_literals;


MainWindow::MainWindow(const ProgramConfig& program_config)
    : window { program_config.window_mode, "MandelbrotFractal" }
    , axis { program_config.axis }
    , zoomer { axis, program_config.initial_zoom_rect_ratio }
    , mandelbrot_fractal { program_config }
{
    initSfmlEventHandler();
}

void MainWindow::startLoop()
{
    while (is_program_work)
    {
        draw();
        handleInput();
        update();
    }
}

void MainWindow::handleInput()
{
    sf::Event e { };
    while (window.pollEvent(e))
    {
        if (!sfml_event_handler.contains(e.type))
        {
            continue;
        }

        (this->*sfml_event_handler[e.type])(e);
    }

    handleMouseButtonMoving();
    handleMouseButtonIsPressing();
}

void MainWindow::initSfmlEventHandler()
{
    sfml_event_handler[sf::Event::EventType::Closed] = &MainWindow::handleProgramClose;
    sfml_event_handler[sf::Event::EventType::KeyPressed] = &MainWindow::handlePressedKeyKeyboard;
    sfml_event_handler[sf::Event::EventType::MouseButtonPressed] = &MainWindow::handlePressedKeyMouse;
    sfml_event_handler[sf::Event::EventType::MouseWheelScrolled] = &MainWindow::handleMouseWheelScroller;
}

void MainWindow::handleProgramClose(const sf::Event&)
{
    is_program_work = false;
}

void MainWindow::handlePressedKeyKeyboard(const sf::Event& e)
{
    if (e.key.code == sf::Keyboard::Space || e.key.code == sf::Keyboard::Add)
    {
        mandelbrot_fractal.shiftIterationsCount(+60);
        is_fractal_recalc_needed = true;
    }
    else if (e.key.code == sf::Keyboard::N || e.key.code == sf::Keyboard::Subtract)
    {
        mandelbrot_fractal.shiftIterationsCount(-20);
        is_fractal_recalc_needed = true;
    }
}

void MainWindow::handlePressedKeyMouse(const sf::Event& e)
{
    if (e.mouseButton.button == sf::Mouse::Button::Left)
    {
        mandelbrot_fractal.shiftIterationsCount(+5);
        zoomer.zoomIn();
        is_fractal_recalc_needed = true;
    }
    else if (e.mouseButton.button == sf::Mouse::Button::Right)
    {
        mandelbrot_fractal.shiftIterationsCount(-5);
        zoomer.zoomOut(sf::Mouse::getPosition(window));
        is_fractal_recalc_needed = true;
    }
}

void MainWindow::handleMouseWheelScroller(const sf::Event& e)
{
    if (e.mouseWheelScroll.delta > 0)
    {
        mandelbrot_fractal.shiftIterationsCount(+3);
        zoomer.zoomIn();
    }
    else
    {
        mandelbrot_fractal.shiftIterationsCount(-2);
        zoomer.zoomOut(sf::Mouse::getPosition(window));
    }
    is_fractal_recalc_needed = true;
}

void MainWindow::handleMouseButtonMoving()
{
    zoomer.setZoomRectanglePosition(sf::Mouse::getPosition(window));
}

void MainWindow::handleMouseButtonIsPressing()
{
    if (!zoom_rect_change_timeout.restartWhenExpired(10ms))
    {
        return;
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::XButton1))
    {
        zoomer.shiftScaleFactor(0.005);
    }
    else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::XButton2))
    {
        zoomer.shiftScaleFactor(-0.005);
    }
}

void MainWindow::update()
{
    if (is_fractal_recalc_needed)
    {
        mandelbrot_fractal.update(axis);
        is_fractal_recalc_needed = false;
    }
}

void MainWindow::draw()
{
    window.clear(sf::Color::White);
    window.draw(mandelbrot_fractal);
    window.draw(zoomer);
    window.display();
}
