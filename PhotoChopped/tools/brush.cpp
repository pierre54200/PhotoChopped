#include "brush.h"
#include "canvas.h"

void Brush::press(Canvas &canvas, const QPoint &pos)
{
    m_lastPoint = pos;
    drawSegment(canvas, pos, pos);
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

void Brush::drawOverlay(QPainter &painter, const Canvas &canvas, const QPoint &cursorPos) const
{
    // cercle qui montre la taille de la gomme
    const qreal r = canvas.brushSize() / 2.0;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::white, 1.5));
    painter.drawEllipse(QPointF(cursorPos), r, r);
    painter.setPen(QPen(Qt::black, 1, Qt::DashLine));
    painter.drawEllipse(QPointF(cursorPos), r, r);
}