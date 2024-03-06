#ifndef MANDELBROT_CPP_ZOOMER_H
#define MANDELBROT_CPP_ZOOMER_H

#include <SFML/Graphics.hpp>
#include "../Utility/Types.h"

class Zoomer : public sf::Drawable
{
public:
    Zoomer(Axis& axis, Real scale_factor);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    void setZoomRectanglePosition(sf::Vector2i mouse_pos);

    void shiftScaleFactor(Real variation);

    void updateRectSizesByScaleFactor();

    void calcNewCartesianByCurrentZoomRectCorners();

    void zoomIn();
    void zoomOut(sf::Vector2i mouse_pos);

    Axis& axis;
    Real scale_factor;
    sf::RectangleShape zoom_rect;

private:
    void drawZoomCorners(sf::RenderTarget& target, sf::RenderStates states) const;
    PlaneBorders<int> calcZoomRectCorners() const;
};


#endif //MANDELBROT_CPP_ZOOMER_H
