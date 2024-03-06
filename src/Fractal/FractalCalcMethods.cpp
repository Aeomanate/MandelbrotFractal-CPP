#include <iostream>
#include "FractalCalcMethods.h"
#include "TaskIterators.h"
#include "../Multithreading/ThreadPoolInstance.h"

std::size_t FractalCalcMethod::isInFractalBody(std::size_t iterations_count, Complex c)
{
    Complex z = { 0, 0 };

    for (std::size_t i = 0; i != iterations_count; ++i)
    {
        // z*z + c
        z = Complex { z.re * z.re - z.im * z.im + c.re, 2 * z.re * z.im + c.im };

        // Math theorem.
        // For any point in the complex plane, we assign a value to k and iterate.
        // if at any particular moment of calculations, for k, the distance from zi(k) to the origin
        // is greater than 2, then we can assume that the given {Zn(k)} will go to infinity
        // (In comparison: the distance is 2, so its square is less than 4 and the square root no need to calcFractal)
        if (z.re * z.re + z.im * z.im > 4)
        {
            return i;
        }
    }

    return std::numeric_limits<std::size_t>::max();
}

void CalcFractalByRowsParallel::calcFractal(std::size_t iterations_count, const Axis& axis,
                                            ResultCallback callbackSetResult)
{
    auto row_task = [&axis, iterations_count, callbackSetResult](int py)
    {
        for (int px = 0; px != axis.screen_borders.x.max; ++px)
        {
            Complex c { axis.screenToCartesianX(px), axis.screenToCartesianY(py) };
            std::size_t spent_iterations = isInFractalBody(iterations_count, c);
            callbackSetResult(spent_iterations, px, py);
        }
    };
    ThreadPoolSimpleInstance::get().addTasks(
        RowTasksIterator<decltype(row_task)> { row_task, axis.screen_borders.y.max });
    ThreadPoolSimpleInstance::get().joinMainToWorkers();
}

void CalcFractalByPixelsParallel::calcFractal(std::size_t iterations_count, const Axis& axis,
                                              ResultCallback callbackSetResult)
{
    auto pixel_task = [&](int px, int py)
    {
        Complex c { axis.screenToCartesianX(px), axis.screenToCartesianY(py) };
        std::size_t spent_iterations = isInFractalBody(iterations_count, c);
        callbackSetResult(spent_iterations, px, py);
    };
    ThreadPoolSimpleInstance::get().addTasks(
        PixelTasksIterator<decltype(pixel_task)> { pixel_task, axis.screen_borders.y.max, axis.screen_borders.x.max });
    ThreadPoolSimpleInstance::get().joinMainToWorkers();
}

void CalcFractalByPixelsSingleThread::calcFractal(std::size_t iterations_count, const Axis& axis,
                                                  ResultCallback callbackSetResult)
{
    for (int py = 0; py != axis.screen_borders.y.max; ++py)
    {
        for (int px = 0; px != axis.screen_borders.x.max; ++px)
        {
            Complex c { axis.screenToCartesianX(px), axis.screenToCartesianY(py) };
            std::size_t spent_iterations = isInFractalBody(iterations_count, c);
            callbackSetResult(spent_iterations, px, py);
        }
    }
}
