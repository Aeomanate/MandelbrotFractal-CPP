#include <iostream>
#include "MainWindow.h"

int main()
{
    try
    {
        sf::Color deep_blue = sf::Color(0, 60, 192);
        sf::Color gold = sf::Color(255, 140, 0);

        ProgramConfig program_config;
        program_config.color_table_config.color_range = { deep_blue, gold };

        MainWindow mainWindow(program_config);
        mainWindow.startLoop();

    } catch (const std::exception& e)
    {
        std::cout << e.what();
    }
    return 0;
}
