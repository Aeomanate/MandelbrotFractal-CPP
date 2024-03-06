#include "Zoomer.h"

Zoomer::Zoomer(Axis& axis, Real scale_factor)
    : axis(axis)
    , scale_factor(scale_factor)
{
    zoom_rect.setSize(sf::Vector2f(sf::Vector2i(axis.screen_borders.x.max, axis.screen_borders.y.max)));
    zoom_rect.setOutlineColor(sf::Color(255, 0, 0));
    zoom_rect.setOutlineThickness(1);
    zoom_rect.setFillColor(sf::Color::Transparent);
    updateRectSizesByScaleFactor();
}

void Zoomer::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(zoom_rect, states);
    drawZoomCorners(target, states);
}

void Zoomer::setZoomRectanglePosition(sf::Vector2i mouse_pos)
{
    sf::Vector2f size = zoom_rect.getSize();
    zoom_rect.setPosition((static_cast<float>(mouse_pos.x)) - size.x / 2, (static_cast<float>(mouse_pos.y)) - size.y / 2);
}

void Zoomer::shiftScaleFactor(Real variation)
{
    scale_factor += variation;
    scale_factor = MinMax<Real>(0.1, 0.99).clamp(scale_factor);
    updateRectSizesByScaleFactor();
}

void Zoomer::updateRectSizesByScaleFactor()
{
    auto x = convert<float>(scale_factor * axis.screen_borders.x.max);
    auto y = convert<float>(scale_factor * axis.screen_borders.y.max);
    zoom_rect.setSize(sf::Vector2f(x, y));
}

void Zoomer::drawZoomCorners(sf::RenderTarget& target, sf::RenderStates states) const
{
    PlaneBorders<int> screenCorners = calcZoomRectCorners();
    Complex zoomerCornerMin = { axis.screenToCartesianX(screenCorners.x.min),
                                axis.screenToCartesianY(screenCorners.y.min) };
    Complex zoomerCornerMax = { axis.screenToCartesianX(screenCorners.x.max),
                                axis.screenToCartesianY(screenCorners.y.max) };

    ComplexNumberAtScreenPos numbers_at_pos[] = {
        { zoomerCornerMin, MinMax<int> { screenCorners.x.min, screenCorners.y.min } },
        { zoomerCornerMax, MinMax<int> { screenCorners.x.max, screenCorners.y.max } }
    };

    for (auto const& number_at_pos: numbers_at_pos)
    {
        target.draw(DrawableNumber(number_at_pos, screenCorners), states);
    }
}

PlaneBorders<int> Zoomer::calcZoomRectCorners() const
{
    sf::Vector2i pos(zoom_rect.getPosition());
    sf::Vector2i size(zoom_rect.getSize());
    PlaneBorders<int> corners(MinMax<int>(pos.x, pos.x + size.x), MinMax<int>(pos.y, pos.y + size.y));
    return corners;
}

void Zoomer::calcNewCartesianByCurrentZoomRectCorners()
{
    PlaneBorders<int> zoom_rect_corners = calcZoomRectCorners();
    Axis cur_axis = axis;

    axis.cartesian_borders.x.min = cur_axis.screenToCartesianX(zoom_rect_corners.x.min);
    axis.cartesian_borders.x.max = cur_axis.screenToCartesianX(zoom_rect_corners.x.max);

    axis.cartesian_borders.y.min = cur_axis.screenToCartesianY(zoom_rect_corners.y.min);
    axis.cartesian_borders.y.max = cur_axis.screenToCartesianY(zoom_rect_corners.y.max);
}

void Zoomer::zoomIn()
{
    calcNewCartesianByCurrentZoomRectCorners();
}

void Zoomer::zoomOut(sf::Vector2i mouse_pos)
{
    Real prev = scale_factor;
    scale_factor = 2 - scale_factor;
    updateRectSizesByScaleFactor();

    setZoomRectanglePosition(mouse_pos);

    calcNewCartesianByCurrentZoomRectCorners();

    scale_factor = prev;
    updateRectSizesByScaleFactor();
}
