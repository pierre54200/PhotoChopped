#pragma once

#include <QPoint>
#include <QCursor>

class Canvas;
class QPainter;
class QKeyEvent;

class Tool
{
public:
    virtual ~Tool() = default;

    // Action
    virtual void press(Canvas &canvas, const QPoint &pos) = 0;
    virtual void move(Canvas &canvas, const QPoint &pos) = 0;
    virtual void release(Canvas &canvas, const QPoint &pos) { Q_UNUSED(canvas); Q_UNUSED(pos); }

    virtual QCursor cursor() const { return Qt::CrossCursor; }

    virtual bool keyPress(Canvas &canvas, QKeyEvent *event) { Q_UNUSED(canvas); Q_UNUSED(event); return false; }

    virtual bool overlayFollowsCursor() const { return true; }

    virtual void drawOverlay(QPainter &painter, const Canvas &canvas, const QPoint &cursorPos) const
    {
        Q_UNUSED(painter); Q_UNUSED(canvas); Q_UNUSED(cursorPos);
    }
};