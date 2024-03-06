#ifndef MANDELBROT_CPP_DEBUGTOOLS_H
#define MANDELBROT_CPP_DEBUGTOOLS_H

#include <vector>
#include <map>
#include <conio.h>
#include <iostream>
#include <Windows.h>
#include "../Multithreading/ThreadPool.h"
#include "../Utility/Types.h"
#include "SFML/Graphics.hpp"
#include "../Utility/Functions.h"

using namespace std::chrono_literals;

struct Invokes
{
    Invokes();

    inline static int number = 0;
    int thread_number = -1, invokes = 0;
    static inline std::mutex m;
};

void setCursor(int row, int col, int background = 0xF, int foreground = 0x0);

void testMultiThreading();


#endif //MANDELBROT_CPP_DEBUGTOOLS_H
