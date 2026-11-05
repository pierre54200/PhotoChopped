#pragma once

#include "tool.h"
#include <QRect>

class QPen;
class QBrush;

class Shape : public Tool
{
public:
    enum class ShapeType { Rectangle, Cercle };

    // Forme
    void setShape(ShapeType shape) { m_shape = shape; }
    ShapeType getShape() const { return m_shape; }

    // Plein
    void setFilled(bool filled) { m_filled = filled; }
    bool isFilled() const { return m_filled; }

    // Action
    void press(Canvas &canvas, const QPoint &pos) override;
    void move(Canvas &canvas, const QPoint &pos) override;
    void release(Canvas &canvas, const QPoint &pos) override;

    // Apercu
    void drawOverlay(QPainter &painter, const Canvas &canvas, const QPoint &cursorPos) const override;

private:
    QRectF currentRect() const;
    void   drawShape(QPainter &painter, const Canvas &canvas) const;

    ShapeType   m_shape = ShapeType::Rectangle;
    bool   m_filled = false;
    bool   m_dragging = false;
    QPoint m_start;
    QPoint m_end;
};