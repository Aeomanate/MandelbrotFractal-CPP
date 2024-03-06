#ifndef MANDELBROT_CPP_FRACTALCALCMETHODS_H
#define MANDELBROT_CPP_FRACTALCALCMETHODS_H

#include <functional>
#include "../Multithreading/ThreadPool.h"
#include "../Utility/Types.h"

class FractalCalcMethod
{
public:
    using ResultCallback = std::function<void(std::size_t spent_iterations, int px, int py)>;

    [[nodiscard]] static std::size_t isInFractalBody(std::size_t iterations_count, Complex c) ;
    virtual void calcFractal(std::size_t iterations_count, const Axis& axis, ResultCallback callbackSetResult) = 0;
};

class CalcFractalByRowsParallel: public FractalCalcMethod
{
public:
    void calcFractal(std::size_t iterations_count, const Axis &axis, ResultCallback callbackSetResult) override;
};

class CalcFractalByPixelsParallel: public FractalCalcMethod
{
public:
    void calcFractal(std::size_t iterations_count, const Axis &axis, ResultCallback callbackSetResult) override;
};

class CalcFractalByPixelsSingleThread: public FractalCalcMethod
{
public:
    void calcFractal(std::size_t iterations_count, const Axis &axis, ResultCallback callbackSetResult) override;
};


#endif //MANDELBROT_CPP_FRACTALCALCMETHODS_H
