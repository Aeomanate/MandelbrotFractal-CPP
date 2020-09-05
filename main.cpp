#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <queue>
#include <functional>
#include <type_traits>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <windows.h>
#include <conio.h>
#include <gmpxx.h>
#include <SFML/Graphics.hpp>
using namespace std::chrono_literals;

struct Retarder {
    using Clock = std::chrono::steady_clock;
    
    template <class TimeUnit>
    bool isExpired(TimeUnit interval) {
        if(Clock::now() - last_update < interval) {
            return false;
        }
        last_update = Clock::now();
        return true;
    }
    
    std::chrono::time_point<Clock> last_update { 0s };
};

template <class T>
struct Limits {
    Limits(T min, T max)
        : min(min)
        , max(max)
    { }
    
    T clamp(T value) const {
        return
            value < min ? min :
            value > max ? max :
            value;
    }
    T avg() const {
        return (min + max) / 2;
    }
    void combine(Limits<T> other) {
        min = std::min(min, other.min);
        max = std::max(max, other.max);
    }
    
    T min, max;
};

template <class T>
struct Borders {
    Borders(Limits<T> x, Limits<T> y)
        : x(x)
        , y(y)
    { }
    
    Limits<T> x, y;
};

using Real = long double;

template <class From, class To>
struct Converter {
    Converter(From const& x): converted(To(x)) { }
    
    operator To() const {
        return converted;
    }
    
    To converted;
};

template <class To>
struct Converter<mpf_class, To> {
    Converter(mpf_class const& x) {
        if constexpr (std::is_same_v<To, sf::Uint8>) {
            converted = (sf::Uint8) x.get_ui();
        } else
        if constexpr (std::is_same_v<To, int>) {
            converted = (int) x.get_si();
        } else
        if constexpr (std::is_same_v<To, float>) {
            converted = (float) x.get_d();
        } else
        if constexpr (std::is_same_v<To, mpf_class>) {
            converted = x;
        } else {
            throw std::logic_error("Unexpected ConvertTo type");
        }
    }
    
    operator To() const {
        return converted;
    }
    To converted;
};


template<class In1, class In2>
In2 linear(In1 value_from, Limits<In1> from, Limits<In2> to) {
    Real result = ((Real)(value_from) - from.min)
                  / (from.max - from.min)
                  * (to.max - to.min)
                  + to.min;
    return Converter<Real, In2>(result);
}

struct DrawableNumber: public sf::Drawable {
    DrawableNumber(Real number, sf::Vector2i pos, Borders<int> bounds) {
        if(!is_load) {
            s << std::setprecision(3) << std::scientific;
            is_load = true;
            if(!font.loadFromFile("arial.ttf")) {
                throw std::logic_error("Font not loaded");
            }
        }
        
        s.str("");
        s << number;
        text.setString(s.str());
        text.setFont(font);
        text.setCharacterSize(14);
        text.setFillColor(sf::Color(255, 255, 255));
        
        text_bounds = text.getLocalBounds();
        float width = text_bounds.width;
        float height = text_bounds.height;
        float offset_x = linear(int(pos.x), bounds.x, Limits<float>(0, width));
        float offset_y = linear(int(pos.y), bounds.y, Limits<float>(0, 1.5f*height));
        text.setPosition(float(pos.x) - offset_x, float(pos.y) - offset_y);
    }
    
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(text, states);
    }
    
    sf::Text text;
    sf::FloatRect text_bounds;
    inline static bool is_load = false;
    inline static sf::Font font;
    static inline std::stringstream s{};
};


template <class Ret, class In, class Owner, class Callable = Ret(Owner::*)(In) const>
struct ShortcutTransformFunctions {
    ShortcutTransformFunctions(Owner* owner, Callable x, Callable y)
        : owner(owner)
        , _x(x)
        , _y(y)
    { }
    
    Ret x(In in) const {
        return (owner->*_x)(in);
    }
    
    Ret y(In in) const {
        return (owner->*_y)(in);
    }
    
    Owner* owner;
    Callable _x, _y;
};

struct Axis {
    Axis(Borders<Real> cartesian, Borders<int> screen)
    : cartesian(cartesian)
    , screen(screen)
    , s2c(this, &Axis::screenToCartesianX, &Axis::screenToCartesianY)
    , c2s(this, &Axis::cartesianToScreenX, &Axis::cartesianToScreenY)
    { }
    
    Axis(Axis const& other)
    : cartesian(other.cartesian)
    , screen(other.screen)
    , s2c(this, &Axis::screenToCartesianX, &Axis::screenToCartesianY)
    , c2s(this, &Axis::cartesianToScreenX, &Axis::cartesianToScreenY)
    { }

    Real screenToCartesianX(int px) const {
        return linear(px, screen.x, cartesian.x);
    }
    Real screenToCartesianY(int py) const {
        return linear(screen.y.max - py, screen.y, cartesian.y);
    }
    
    int cartesianToScreenX(Real x) const {
        return screen.x.clamp(linear(x, cartesian.x, screen.x));
    }
    int cartesianToScreenY(Real y) const {
        return screen.y.clamp(screen.y.max - linear(y, cartesian.y, screen.y));
    }
    
    Borders<Real> cartesian;
    Borders<int> screen;
    ShortcutTransformFunctions<Real, int, Axis> s2c;
    ShortcutTransformFunctions<int, Real, Axis> c2s;
};

struct Zoomer: public sf::Drawable {
    Zoomer(Axis& axis, Real scale_factor)
    : axis(axis)
    , scale_factor(scale_factor)
    {
        zoom_rect.setSize(sf::Vector2f(sf::Vector2i(axis.screen.x.max, axis.screen.y.max)));
        zoom_rect.setOutlineColor(sf::Color(255, 0, 0));
        zoom_rect.setOutlineThickness(1);
        zoom_rect.setFillColor(sf::Color::Transparent);
        updateRectSizes();
    }
    
    void setPos(sf::Vector2i mouse_pos) {
        sf::Vector2f size = zoom_rect.getSize();
        zoom_rect.setPosition(
            float(mouse_pos.x) - size.x/2,
            float(mouse_pos.y) - size.y/2
        );
    }
    void shiftScaleFactor(Real variation) {
        scale_factor += variation;
        scale_factor = Limits<Real>(0.1, 0.98).clamp(scale_factor);
        updateRectSizes();
    }
    
    void updateRectSizes() {
        float x = Converter<Real, float>(scale_factor * axis.screen.x.max);
        float y = Converter<Real, float>(scale_factor * axis.screen.y.max);
        zoom_rect.setSize(sf::Vector2f(x, y));
    }
    
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(zoom_rect, states);
        /*
        Borders<int> corners = zoom_rect_corners();
        struct { Real number; sf::Vector2i pos; } zoom_in_bounds[] = {
            {
                axis.s2c.x(corners.x.min),
                sf::Vector2i(corners.x.min, corners.y.avg())
            },
            {
                axis.s2c.x(corners.x.max),
                sf::Vector2i(corners.x.max, corners.y.avg())
            },
            {
                axis.s2c.y(corners.y.min),
                sf::Vector2i(corners.x.avg(), corners.y.min)
            },
            {
                axis.s2c.y(corners.y.max),
                sf::Vector2i(corners.x.avg(), corners.y.max)
            },
            
        };
        for(auto const& bound: zoom_in_bounds) {
            target.draw(DrawableNumber(bound.number, bound.pos, corners), states);
        }
        */
    }
    
    Borders<int> zoomRectCorners() const {
        sf::Vector2i pos(zoom_rect.getPosition());
        sf::Vector2i size(zoom_rect.getSize());
        Borders<int> corners(
            Limits<int>(pos.x, pos.x + size.x),
            Limits<int>(pos.y, pos.y + size.y)
        );
        return corners;
    }
    
    void axisCartesianResize() {
        Borders<int> corners = zoomRectCorners();
        Axis prev_axis = Axis(axis);
    
        axis.cartesian.x.min = prev_axis.s2c.x(corners.x.min);
        axis.cartesian.x.max = prev_axis.s2c.x(corners.x.max);
        axis.cartesian.y.min = prev_axis.s2c.y(corners.y.max);
        axis.cartesian.y.max = prev_axis.s2c.y(corners.y.min);
    }
    
    void zoomIn() {
        axisCartesianResize();
    }
    void zoomOut(sf::Vector2i mouse_pos) {
        Real prev = scale_factor;
        scale_factor = 2 - scale_factor;
        updateRectSizes();
        setPos(mouse_pos);
        axisCartesianResize();
        
        scale_factor = prev;
        updateRectSizes();
    }
    
    Axis& axis;
    Real scale_factor;
    sf::RectangleShape zoom_rect;
};

template <class Result>
class TaskPool {
  public:
    using Task = std::function<Result()>;
    template <class TasksContainer>
    void add(TasksContainer tasks_container) {
        {
            std::lock_guard lock(tasks.m);
            do {
                tasks.q.push(*tasks_container);
            } while(++tasks_container);
        }
        notifyAll();
    }
    Task getTask() {
        Task task;
        if(std::lock_guard lock(tasks.m); !tasks.q.empty()) {
            task = tasks.q.front();
            tasks.q.pop();
        }
        return task;
    }
    void notifyAll() {
        tasks.v.notify_all();
    }
    bool isEmpty() {
        std::lock_guard lock(tasks.m);
        return tasks.q.empty();
    }
    void waitForTasks(bool const& is_program_work) {
        std::unique_lock lock(tasks.m);
        tasks.v.wait(lock, [&] { return !tasks.q.empty() || !is_program_work; });
    }
  private:
    struct {
        std::queue<Task> q;
        std::mutex m;
        std::condition_variable v;
    } tasks;
};

template <class Result>
class Worker {
  public:
    Worker() { }
    Worker(bool const& is_program_work, TaskPool<Result>& task_pool)
    : thread(&Worker::work, this, std::ref(is_program_work), std::ref(task_pool))
    { }
    ~Worker() {
        if(thread.joinable()) thread.join();
    }
    void work(bool const& is_program_work, TaskPool<Result>& task_pool) {
        while(is_program_work) {
            typename TaskPool<Result>::Task task = task_pool.getTask();
            if(task) {
                finished_work.push_back(std::move(task()));
            } else {
                done_work = true;
                task_pool.waitForTasks(is_program_work);
                done_work = false;
            }
        }
    }
    void workMain(TaskPool<Result>& task_pool) {
        typename TaskPool<Result>::Task task;
        while(task = task_pool.getTask()) {
            finished_work.push_back(std::move(task()));
        }
        
    }
    bool done() const {
        return done_work;
    }
    std::vector<Result>&& getResults() {
        if(!done()) throw std::logic_error("Work not done");
        return std::move(finished_work);
    }
    
  private:
    std::thread thread;
    std::atomic<bool> done_work = true;
    std::vector<Result> finished_work;
};

template <class Result>
class ThreadPool {
  public:
    ThreadPool() {
        for(size_t i = 0; i != std::thread::hardware_concurrency() - 1; ++i) {
            workers.push_back(std::make_unique<Worker<Result>>(is_program_work, task_pool));
        }
        workers.push_back(std::make_unique<Worker<Result>>());
    }
    ~ThreadPool() {
        is_program_work = false;
        task_pool.notifyAll();
        workers.clear();
    }
    template <class TasksContainer>
    void addTasks(TasksContainer task_container) {
        if(!task_container) return;
        task_pool.add(task_container);
    }
    void joinMainToWorkers() {
        workers.back()->workMain(task_pool);
    }
    bool done() {
        bool work_done = task_pool.isEmpty();
        for(std::unique_ptr<Worker<Result>> const& w: workers) {
            if(work_done &= w->done(); !work_done) {
                break;
            }
        }
        return work_done;
    }
    void handleResults(std::function<void(Result&)> result_handler) {
        while(!done());
        for(auto& worker: workers) {
            for(auto& result: worker->getResults()) {
                result_handler(result);
            }
        }
    }
    
  private:
    bool is_program_work = true;
    TaskPool<Result> task_pool;
    std::vector<std::unique_ptr<Worker<Result>>> workers;
};

class Fractal: public sf::Drawable {
  public:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(sprite, states);
    }
    
    void updateSprite() {
        texture.loadFromImage(image);
        sprite.setTexture(texture);
    }
    
    sf::Image image;
  private:
    sf::Texture texture;
    sf::Sprite sprite;
};

template <class TaskTemplate>
class RowTasks {
  public:
    RowTasks(TaskTemplate task, int end_row): task(task), end_row(end_row) { }
  
    RowTasks& operator++() {
        ++cur_row;
        return *this;
    }
    auto operator*() {
        return [cur_row=cur_row, task=task] {
            return task(cur_row);
        };
    }
    operator bool() {
        return cur_row != end_row;
    }
    
  private:
    TaskTemplate task;
    int cur_row = 0, end_row;
};

struct Mandelbrot: public sf::Drawable {
    Mandelbrot(sf::VideoMode screen_sizes, unsigned max_iterations, Real R)
    : R2(R*R)
    , limit_iterations(max_iterations)
    , amount_iterations(0, limit_iterations)
    {
        fractal.image.create(screen_sizes.width, screen_sizes.height);
    
        sf::Color deep_blue = sf::Color(0, 60, 192);
        sf::Color gold = sf::Color(255, 140, 0);
        for(unsigned i = 0; i != std::size(color_table); ++i) {
            color_table[i] = iterToColor(i, deep_blue, gold);
        }
    }
    
    std::tuple<bool, unsigned> isInSet(Real x, Real y) {
        struct Complex { Real re, im; };
        Complex z = {0, 0};
        Complex c = { x, y };
        for(unsigned i = 0; i != limit_iterations; ++i) {
            z = Complex {
                z.re*z.re - z.im*z.im + c.re,
                z.re*z.im + z.im*z.re + c.im
            };
            
            if(z.re*z.re + z.im*z.im > R2) {
                return { false, i };
            }
        }
        return { true, limit_iterations };
    }
    
    Limits<unsigned> updateT1(Axis const& axis) {
        auto cur_iter = Limits<unsigned>(amount_iterations.max, amount_iterations.min);
        for(int py = 0; py != axis.screen.y.max; ++py) {
            for(int px = 0; px != axis.screen.x.max; ++px) {
                auto [in_set, iter] = isInSet(axis.s2c.x(px), axis.s2c.y(py));
                if(in_set) {
                    cur_iter.max = std::max(cur_iter.max, iter);
                    cur_iter.min = std::min(cur_iter.min, iter);
                }
                fractal.image.setPixel(
                    unsigned(px), unsigned(py),
                    in_set ? sf::Color(0, 0, 0)
                           : color_table[iter % std::size(color_table)]
                );
            }
        }
        return cur_iter;
    }
    Limits<unsigned> updateT8(Axis const& axis) {
        auto task = [this, &axis] (int py) {
            auto iters_in_row = Limits<unsigned>(
                amount_iterations.max, amount_iterations.min
            );
            for(int px = 0; px != axis.screen.x.max; ++px) {
                auto[in_set, iter] = isInSet(
                    axis.s2c.x(px), axis.s2c.y(py)
                );
                if(in_set) {
                    iters_in_row.max = std::max(iters_in_row.max, iter);
                    iters_in_row.min = std::min(iters_in_row.min, iter);
                }
                fractal.image.setPixel(
                    unsigned(px), unsigned(py),
                    in_set ? sf::Color(0, 0, 0)
                           : color_table[iter % std::size(color_table)]
                );
            }
            return iters_in_row;
        };
        thread_pool.addTasks(RowTasks<decltype(task)>(task, axis.screen.y.max));
        thread_pool.joinMainToWorkers();
        
        auto result_iters = Limits<unsigned>(
            amount_iterations.max, amount_iterations.min
        );
        thread_pool.handleResults([&] (Limits<unsigned> const& iters_in_row) {
            result_iters.combine(iters_in_row);
        });
        return result_iters;
    }
    
    void update(Axis const& axis) {
        /*
        auto a = std::chrono::steady_clock::now();
        amount_iterations = updateT1(axis);
        auto b = std::chrono::steady_clock::now();
        auto mcs_a = std::chrono::duration_cast<std::chrono::milliseconds>(b - a).count();
        */
        // a = std::chrono::steady_clock::now();
        amount_iterations = updateT8(axis);
        // b = std::chrono::steady_clock::now();
        // auto mcs_b = std::chrono::duration_cast<std::chrono::milliseconds>(b - a).count();
    
        /*
        std::cout << "T1: " << mcs_a << " " << "T8: " << mcs_a << " "
                  << "Diff (T1-T8) by: " << mcs_a - mcs_b << " "
                  << "Diff (T1/T8) times: " << double(mcs_a)/double(mcs_b) << "\n";
        */
        fractal.updateSprite();
    }
    
    void shiftMaxIteration(int offset) {
        int new_limit_iterations = int(limit_iterations) + offset;
        limit_iterations = unsigned(Limits<int>(1, 10'000).clamp(new_limit_iterations));
    }
    
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(fractal, states);
    }
    
    sf::Uint8 iterToChannel(unsigned iter, Limits<sf::Uint8> color_channel) {
        return linear(iter, amount_iterations, color_channel);
    }
    sf::Color iterToColor(unsigned iter, sf::Color min, sf::Color max) {
        sf::Uint8 r = iterToChannel(iter, { min.r, max.r });
        sf::Uint8 g = iterToChannel(iter, { min.g, max.g });
        sf::Uint8 b = iterToChannel(iter, { min.b, max.b });
        return sf::Color { r, g, b };
    }
    
    Real R2;
    unsigned limit_iterations;
    Limits<unsigned> amount_iterations;
    sf::Color color_table[40];
    Fractal fractal;
    ThreadPool<Limits<unsigned>> thread_pool;
};

class Main {
  public:
    Main(sf::VideoMode video_mode, unsigned max_iterations, Real R, double scale_factor)
    : window(video_mode, "Mandelbrot")
    , axis(
        Borders<Real>( Limits<Real>{-2, 1}, Limits<Real>{-1, 1} ),
        Borders<int>(
            Limits<int>{0, int(video_mode.width)},
            Limits<int>{0, int(video_mode.height)}
        )
      )
    , zoomer(axis, scale_factor)
    , mandelbrot(video_mode, max_iterations, R)
    { }
    
    void mainLoop() {
        while(is_program_work) {
            draw();
            handleInput();
            update();
        }
    }
    
  private:
    void handleInput() {
        sf::Event e;
        while(window.pollEvent(e)) {
            switch(e.type) {
                case sf::Event::EventType::Closed:
                    is_program_work = false;
                    break;
                    
                case sf::Event::EventType::KeyPressed:
                    if(e.key.code == sf::Keyboard::Space) {
                        mandelbrot.shiftMaxIteration(+50);
                        is_need_recalc = true;
                    }
                    else if(e.key.code == sf::Keyboard::N) {
                        mandelbrot.shiftMaxIteration(-1);
                        is_need_recalc = true;
                    }
                    break;
                
                case sf::Event::EventType::MouseButtonPressed:
                    if(e.mouseButton.button == sf::Mouse::Button::Left) {
                        mandelbrot.shiftMaxIteration(+2);
                        zoomer.zoomIn();
                    } else if(e.mouseButton.button == sf::Mouse::Button::Right) {
                        mandelbrot.shiftMaxIteration(-2);
                        zoomer.zoomOut(sf::Mouse::getPosition(window));
                    }
                    is_need_recalc = true;
                    break;
                
                case sf::Event::EventType::MouseButtonReleased:
                    break;
                
                case sf::Event::EventType::MouseWheelScrolled: {
                    if(e.mouseWheelScroll.delta > 0) {
                        mandelbrot.shiftMaxIteration(+2);
                        zoomer.zoomIn();
                    } else {
                        mandelbrot.shiftMaxIteration(-2);
                        zoomer.zoomOut(sf::Mouse::getPosition(window));
                    }
                    is_need_recalc = true;
                    break;
                }
                
                default:
                    break;
            }
        }
    
        zoomer.setPos(sf::Mouse::getPosition(window));
        if(sf::Mouse::isButtonPressed(sf::Mouse::Button::XButton1)) {
            if(!retarder.isExpired(10ms)) return;
            zoomer.shiftScaleFactor(0.005);
        }
        else if(sf::Mouse::isButtonPressed(sf::Mouse::Button::XButton2)) {
            if(!retarder.isExpired(10ms)) return;
            zoomer.shiftScaleFactor(-0.005);
        }
        
    }
    
    void update() {
        if(is_need_recalc) {
            mandelbrot.update(axis);
            is_need_recalc = false;
        }
    }
    
    void draw() {
        window.clear(sf::Color(255, 255, 255));
        window.draw(mandelbrot);
        window.draw(zoomer);
        window.display();
    }
    
  private:
    bool is_program_work = true;
    bool is_need_recalc = true;
    sf::RenderWindow window;
    Axis axis;
    Zoomer zoomer;
    Retarder retarder;
    Mandelbrot mandelbrot;
};

void setCursor(int row, int col, int background = 0xF, int foreground = 0x0) {
    static HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    static struct S {
        S() {
            MoveWindow(GetConsoleWindow(), 100, 100, 1256, 600, TRUE);
        }
    } set_size;
    SetConsoleCursorPosition(console, {short(col), short(row)});
    SetConsoleTextAttribute(console, byte(background << 4u | foreground));
}
struct Invokes {
    Invokes() {
        std::lock_guard lock(m);
        thread_number = number;
        ++number;
    }
    inline static int number = 0;
    int thread_number = -1, invokes = 0;
    static inline std::mutex m;
};
template <class Task>
class TasksContainer {
  public:
    TasksContainer(size_t start, size_t end, Task task = Task())
    : cur(start), end(end), task(task) { }
    TasksContainer& operator++() {
        ++cur;
        return *this;
    }
    Task operator*() {
        if(task) return task;
        else throw std::logic_error("Dereferensing end iterator");
    }
    operator bool() {
        return cur != end;
    }
    
  private:
    size_t cur, end;
    Task task;
};
void testMultiThreading() {
    using Result = std::vector<byte>;
    using Foreman = ThreadPool<Result>;
    using Tasks = TaskPool<Result>;
    using Task = Tasks::Task;
    
    Foreman thread_pool;
    std::mutex cout_mutex;
    std::mutex map_mutex;
    std::map<std::thread::id, Invokes> count_invokes;
    
    Task task = [&cout_mutex, &map_mutex, &count_invokes] {
        std::thread::id id = std::this_thread::get_id();
        map_mutex.lock();
        int& inv = count_invokes[id].invokes;
        int n = count_invokes[id].thread_number;
        map_mutex.unlock();
        for(int j = 0; j != 20; ++j) {
            std::lock_guard lock(cout_mutex);
            setCursor(n*3, j*5, abs(0xF - inv*4), inv*3/2%0x10);
            printf("%c%i", 'A' + n, j);
            std::this_thread::sleep_for(20ms);
        }
        std::unique_lock lock(cout_mutex);
        setCursor(n*3, 20*5);
        printf("%d", inv);
        lock.unlock();
        
        ++inv;
        return std::vector<byte>(1, true);
    };

    int digit;
    while(true) {
        if(!isdigit(digit = _getch())) {
            break;
        } else if(digit == '0') {
            thread_pool.joinMainToWorkers();
            setCursor(3 * 7 + 1, 0);
            thread_pool.handleResults([](std::vector<byte> const& r) {
                for(byte x: r) {
                    std::cout << bool(x) << ' ';
                }
            });
            std::cout << '\n';
            system("pause");
            system("color f0 & cls");
        }
        thread_pool.addTasks(TasksContainer<Task>(0, size_t(digit - '0'), task));
    }
}

int main() {
    // testMultiThreading();
    Main({1024, 512}, 20, 2, 0.8).mainLoop();
    return 0;
}
