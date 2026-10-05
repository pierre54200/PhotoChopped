#include "brush.h"
#include "canvas.h"

void Brush::press(Canvas &canvas, const QPoint &pos)
{
    m_lastPoint = pos;
    drawSegment(canvas, pos, pos);   // un simple clic laisse un point
}

void Brush::move(Canvas &canvas, const QPoint &pos)
{
    drawSegment(canvas, m_lastPoint, pos);
    m_lastPoint = pos;
}

void Brush::setupPainter(QPainter &painter, const Canvas &canvas) const
{
    painter.setPen(QPen(canvas.color(), canvas.brushSize(),
                        Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}

void Brush::drawSegment(Canvas &canvas, const QPoint &from, const QPoint &to)
{
    QPainter painter(&canvas.image());
    painter.setRenderHint(QPainter::Antialiasing);
    setupPainter(painter, canvas);
    painter.drawLine(from, to);

    const int r = canvas.brushSize() / 2 + 2;
    canvas.refresh(QRect(from, to).normalized().adjusted(-r, -r, r, r));
}