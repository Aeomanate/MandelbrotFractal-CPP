#ifndef MANDELBROT_CPP_THREADPOOLINSTANCE_H
#define MANDELBROT_CPP_THREADPOOLINSTANCE_H

#include "ThreadPool.h"

template<class TaskResult>
class ThreadPoolInstanceTemplate
{
public:
    ThreadPoolInstanceTemplate() = delete;

    static ThreadPool<TaskResult>& get()
    {
        static ThreadPool<TaskResult> thread_pool;
        return thread_pool;
    }
};

using ThreadPoolSimpleInstance = ThreadPoolInstanceTemplate<void>;

#endif //MANDELBROT_CPP_THREADPOOLINSTANCE_H
