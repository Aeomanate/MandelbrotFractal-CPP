#ifndef MANDELBROT_CPP_TASKITERATORS_H
#define MANDELBROT_CPP_TASKITERATORS_H

template<class TaskTemplate>
class RowTasksIterator
{
public:
    RowTasksIterator(TaskTemplate task, int rows)
        : task_by_row(task)
        , rows(rows)
    { }

    RowTasksIterator& operator ++()
    {
        ++cur_row;
        return *this;
    }

    auto operator *()
    {
        return [row = cur_row, task = task_by_row]
        {
            return task(row);
        };
    }

    explicit operator bool()
    {
        return cur_row != rows;
    }

private:
    TaskTemplate task_by_row;
    int cur_row = 0, rows;
};

template<class TaskTemplate>
class PixelTasksIterator
{
public:
    PixelTasksIterator(TaskTemplate task, int rows, int cols)
        : task(task)
        , cols(cols)
        , size(rows * cols)
    { }

    PixelTasksIterator& operator ++()
    {
        ++cur;
        return *this;
    }

    auto operator *()
    {
        return [py = cur / cols, px = cur % cols, task = task]
        {
            return task(py, px);
        };
    }

    explicit operator bool()
    {
        return cur != size;
    }

private:
    TaskTemplate task;
    int cols, size, cur = 0;
};

#endif //MANDELBROT_CPP_TASKITERATORS_H
