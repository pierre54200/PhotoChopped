#include "canvas.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
    , m_image(800, 600, QImage::Format_ARGB32_Premultiplied)
{
    m_image.fill(Qt::white);
    setFixedSize(m_image.size());
    setCursor(Qt::CrossCursor);
}

void Canvas::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(event->rect(), m_image, event->rect());
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_lastPoint = event->position();
        m_drawing = true;
        drawLineTo(event->position());
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if (m_drawing && (event->buttons() & Qt::LeftButton))
        drawLineTo(event->position());
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_drawing) {
        drawLineTo(event->position());
        m_drawing = false;
    }
}

void Canvas::drawLineTo(const QPointF &end)
{
    {
        QPainter painter(&m_image);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(m_penColor, m_penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(m_lastPoint, end);
    }
    const int margin = m_penWidth / 2 + 2;
    update(QRectF(m_lastPoint, end).normalized()
               .adjusted(-margin, -margin, margin, margin)
               .toAlignedRect());

    m_lastPoint = end;
}