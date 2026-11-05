#include "shape.h"
#include "canvas.h"

#include <QPainter>
#include <QGuiApplication>
#include <cmath>

void Shape::press(Canvas &, const QPoint &pos)
{
    m_dragging = true;
    m_start = pos;
    m_end = pos;
}

void Shape::move(Canvas &, const QPoint &pos)
{
    if (m_dragging)
        m_end = pos;
}

void Shape::release(Canvas &canvas, const QPoint &pos)
{
    if (!m_dragging)
        return;
    m_end = pos;
    m_dragging = false;

    const QRectF rect = currentRect();
    if (rect.width() < 1 && rect.height() < 1)
        return;

    QPainter painter(&canvas.image());
    drawShape(painter, canvas);
    painter.end();

    const int margin = canvas.brushSize() / 2 + 2;
    canvas.refresh(rect.toAlignedRect().adjusted(-margin, -margin, margin, margin));
}

QRectF Shape::currentRect() const
{
    const Qt::KeyboardModifiers mods = QGuiApplication::keyboardModifiers();
    double dx = m_end.x() - m_start.x();
    double dy = m_end.y() - m_start.y();

    if (mods & Qt::ShiftModifier) {
        const double side = std::max(std::abs(dx), std::abs(dy));
        dx = std::copysign(side, dx == 0 ? 1.0 : dx);
        dy = std::copysign(side, dy == 0 ? 1.0 : dy);
    }

    const QPointF start = QPointF(m_start) + QPointF(0.5, 0.5);

    if (mods & Qt::ControlModifier)
        return QRectF(start - QPointF(dx, dy), start + QPointF(dx, dy)).normalized();

    return QRectF(start, start + QPointF(dx, dy)).normalized();
}

void Shape::drawShape(QPainter &painter, const Canvas &canvas) const
{
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_filled) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(canvas.color());
    } else {
        QPen pen(canvas.color(), canvas.brushSize());
        pen.setJoinStyle(Qt::MiterJoin);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
    }

    const QRectF rect = currentRect();
    if (m_shape == ShapeType::Rectangle)
        painter.drawRect(rect);
    else
        painter.drawEllipse(rect);
}

void Shape::drawOverlay(QPainter &painter, const Canvas &canvas, const QPoint &) const
{
    if (!m_dragging)
        return;

    painter.save();
    painter.setOpacity(0.6);
    drawShape(painter, canvas);
    painter.restore();

    QPen frame(Qt::black, 1, Qt::DashLine);
    frame.setCosmetic(true);
    painter.setPen(frame);
    painter.setBrush(Qt::NoBrush);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.drawRect(currentRect());
}