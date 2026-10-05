#ifndef TOOL_H
#define TOOL_H

#include <QPoint>
#include <QCursor>

class Canvas;
class QPainter;

class Tool
{
public:
    virtual ~Tool() = default;

    virtual void press(Canvas &canvas, const QPoint &pos) = 0;
    virtual void move(Canvas &canvas, const QPoint &pos) = 0;
    virtual void release(Canvas &canvas, const QPoint &pos) { Q_UNUSED(canvas); Q_UNUSED(pos); }

    virtual QCursor cursor() const { return Qt::CrossCursor; }

    virtual void drawOverlay(QPainter &painter, const Canvas &canvas, const QPoint &cursorPos) const
    {
        Q_UNUSED(painter); Q_UNUSED(canvas); Q_UNUSED(cursorPos);
    }
};

#endif // TOOL_H
