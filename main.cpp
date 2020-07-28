#include <iostream>
#include <type_traits>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <gmpxx.h>
#include <SFML/Graphics.hpp>
using namespace std::chrono_literals;

struct Retarder {
    using Clock = std::chrono::steady_clock;
    
    template <class TimeUnit>
    bool is_expired(TimeUnit interval) {
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

using Real = double;

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
    , s2c(this, &Axis::screen_to_cartesian_x, &Axis::screen_to_cartesian_y)
    , c2s(this, &Axis::cartesian_to_screen_x, &Axis::cartesian_to_screen_y)
    { }
    
    Axis(Axis const& other)
    : cartesian(other.cartesian)
    , screen(other.screen)
    , s2c(this, &Axis::screen_to_cartesian_x, &Axis::screen_to_cartesian_y)
    , c2s(this, &Axis::cartesian_to_screen_x, &Axis::cartesian_to_screen_y)
    { }

    Real screen_to_cartesian_x(int px) const {
        return linear(px, screen.x, cartesian.x);
    }
    Real screen_to_cartesian_y(int py) const {
        return linear(screen.y.max - py, screen.y, cartesian.y);
    }
    
    int cartesian_to_screen_x(Real x) const {
        return screen.x.clamp(linear(x, cartesian.x, screen.x));
    }
    int cartesian_to_screen_y(Real y) const {
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
        update_rect_sizes();
    }
    
    void set_pos(sf::Vector2i mouse_pos) {
        sf::Vector2f size = zoom_rect.getSize();
        zoom_rect.setPosition(
            float(mouse_pos.x) - size.x/2,
            float(mouse_pos.y) - size.y/2
        );
    }
    void shift_scale_factor(Real variation) {
        if(!retarder.is_expired(10ms)) return;
        scale_factor += variation;
        scale_factor = Limits<Real>(0.05, 0.9).clamp(scale_factor);
        update_rect_sizes();
    }
    
    void update_rect_sizes() {
        float x = Converter<Real, float>(scale_factor * axis.screen.x.max);
        float y = Converter<Real, float>(scale_factor * axis.screen.y.max);
        zoom_rect.setSize(sf::Vector2f(x, y));
    }
    
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(zoom_rect, states);
        
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
    }
    
    Borders<int> zoom_rect_corners() const {
        sf::Vector2i pos(zoom_rect.getPosition());
        sf::Vector2i size(zoom_rect.getSize());
        Borders<int> corners(
            Limits<int>(pos.x, pos.x + size.x),
            Limits<int>(pos.y, pos.y + size.y)
        );
        return corners;
    }
    void zoom_in() {
        Borders<int> corners = zoom_rect_corners();
        Axis prev_axis = Axis(axis);
        
        axis.cartesian.x.min = prev_axis.s2c.x(corners.x.min);
        axis.cartesian.x.max = prev_axis.s2c.x(corners.x.max);
        axis.cartesian.y.min = prev_axis.s2c.y(corners.y.max);
        axis.cartesian.y.max = prev_axis.s2c.y(corners.y.min);
    }
    void zoom_out() {
        Borders<int> corners = zoom_rect_corners();
        Borders<Real> in_axis(
            Limits<Real>(
                axis.s2c.x(corners.x.min),
                axis.s2c.x(corners.x.max)
            ),
            Limits<Real>(
                axis.s2c.y(corners.y.max),
                axis.s2c.y(corners.y.min)
            )
        );
        Borders<Real> dists (
            Limits<Real>(
                std::abs(axis.cartesian.x.min - in_axis.x.min),
                std::abs(axis.cartesian.x.max - in_axis.x.max)
            ),
            Limits<Real>(
                std::abs(axis.cartesian.y.min - in_axis.y.min),
                std::abs(axis.cartesian.y.max - in_axis.y.max)
            )
        );
        
        
        axis.cartesian.x.min -= 2*dists.x.min;
        axis.cartesian.x.max += 2*dists.x.max;
        axis.cartesian.y.min -= 2*dists.y.max;
        axis.cartesian.y.max += 2*dists.y.min;
    }
    
    Axis& axis;
    Retarder retarder;
    Real scale_factor;
    sf::RectangleShape zoom_rect;
};

struct Complex {
    Real re, im;
};

struct Mandelbrot: public sf::Drawable {
    Mandelbrot(sf::VideoMode screen_sizes, unsigned max_iterations, Real R)
    : R2(R*R)
    , limit_iterations(max_iterations)
    , count_iterations(0, limit_iterations)
    {
        image.create(screen_sizes.width, screen_sizes.height);
    
        sf::Color deep_blue = sf::Color(0, 60, 192);
        sf::Color gold = sf::Color(255, 140, 0);
        color_table[0] = black;
        for(unsigned i = 0; i != 512; ++i) {
            color_table[i] = iter_to_color(i, deep_blue, gold);
        }
    }
    
    std::tuple<bool, unsigned> isInSet(Real x, Real y) {
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
    
    void update(Axis const& axis) {
        
        Limits<unsigned> cur_iters(count_iterations.max, count_iterations.min);
        for(int py = 0; py != axis.screen.y.max; ++py) {
            for(int px = 0; px != axis.screen.x.max; ++px) {
                auto [in_set, iter] = isInSet(axis.s2c.x(px), axis.s2c.y(py));
                if(!in_set) {
                    cur_iters.min = std::min(cur_iters.min, iter);
                    cur_iters.max = std::max(cur_iters.max, iter);
                }
                image.setPixel(
                    unsigned(px), unsigned(py),
                    in_set ? black : color_table[iter%std::size(color_table)]
                );
            }
        }
        count_iterations = cur_iters;
        
        texture.loadFromImage(image);
        sprite.setTexture(texture);
    }
    
    void shift_max_iteration(int offset) {
        int new_limit_iterations = int(limit_iterations) + offset;
        limit_iterations = unsigned(Limits<int>(1, 10'000).clamp(new_limit_iterations));
    }
    
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(sprite, states);
    }
    
    sf::Uint8 iter_to_color_channel(unsigned iter, Limits<sf::Uint8> color_channel) {
        return linear(iter, count_iterations, color_channel);
    }
    sf::Color iter_to_color(unsigned iter, sf::Color min, sf::Color max) {
        sf::Uint8 r = iter_to_color_channel(iter, {min.r, max.r});
        sf::Uint8 g = iter_to_color_channel(iter, {min.g, max.g});
        sf::Uint8 b = iter_to_color_channel(iter, {min.b, max.b});
        return sf::Color { r, g, b };
    }
    
    Real R2;
    unsigned limit_iterations;
    Limits<unsigned> count_iterations;
    sf::Color color_table[512];
    sf::Color black = sf::Color(0, 0, 0);
    sf::Image image;
    sf::Texture texture;
    sf::Sprite sprite;
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
                    if(e.key.code == sf::Keyboard::Add) {
                        mandelbrot.shift_max_iteration(+20);
                        is_need_recalc = true;
                    }
                    else if(e.key.code == sf::Keyboard::Subtract) {
                        mandelbrot.shift_max_iteration(-1);
                        is_need_recalc = true;
                    }
                    break;
                
                case sf::Event::EventType::MouseButtonPressed:
                    break;
                
                case sf::Event::EventType::MouseButtonReleased:
                    break;
                
                case sf::Event::EventType::MouseWheelScrolled: {
                    if(e.mouseWheelScroll.delta > 0) {
                        mandelbrot.shift_max_iteration(+2);
                        zoomer.zoom_in();
                    } else {
                        mandelbrot.shift_max_iteration(-2);
                        zoomer.zoom_out();
                    }
                    is_need_recalc = true;
                    break;
                }
                
                default:
                    break;
            }
        }
        
        if(sf::Mouse::isButtonPressed(sf::Mouse::Button::XButton1)) {
            zoomer.shift_scale_factor(0.005);
        }
        else if(sf::Mouse::isButtonPressed(sf::Mouse::Button::XButton2)) {
            zoomer.shift_scale_factor(-0.005);
        }
        
        zoomer.set_pos(sf::Mouse::getPosition(window));
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
    Mandelbrot mandelbrot;
};

int main() {
    Main({1024, 512}, 20, 2, 0.8).mainLoop();
    return 0;
}
