#ifndef BRUSH_H
#define BRUSH_H

#include "tool.h"
#include <QPainter>

class Brush : public Tool
{
public:
    void press(Canvas &canvas, const QPoint &pos) override;
    void move(Canvas &canvas, const QPoint &pos) override;

    QCursor cursor() const override { return Qt::BlankCursor; }
    void drawOverlay(QPainter &painter, const Canvas &canvas, const QPoint &cursorPos) const override;

protected:
    void drawSegment(Canvas &canvas, const QPoint &from, const QPoint &to);

    virtual void setupPainter(QPainter &painter, const Canvas &canvas) const;

private:
    QPoint m_lastPoint;
};

#endif // BRUSH_H
