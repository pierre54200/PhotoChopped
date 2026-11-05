#include "crop.h"
#include "canvas.h"

#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>

static QRect rectBetween(const QPoint &a, const QPoint &b)
{
    const int left = std::min(a.x(), b.x());
    const int y0   = std::min(a.y(), b.y());
    return QRect(left, y0, std::abs(b.x() - a.x()), std::abs(b.y() - a.y()));
}

QRect Crop::clampToImage(const QRect &rect, const Canvas &canvas) const
{
    return rect.intersected(QRect(QPoint(0, 0), canvas.imageSize()));
}

void Crop::press(Canvas &, const QPoint &pos)
{
    if (m_hasRect && m_rect.contains(pos)) {
        m_mode = Mode::Moving;
        m_grab = pos;
        m_rectAtGrab = m_rect;
    } else {
        m_mode = Mode::Creating;
        m_start = pos;
        m_rect = QRect(pos, QSize(0, 0));
        m_hasRect = false;
    }
}

void Crop::move(Canvas &canvas, const QPoint &pos)
{
    if (m_mode == Mode::Creating) {
        m_rect = clampToImage(rectBetween(m_start, pos), canvas);
    } else if (m_mode == Mode::Moving) {
        QRect moved = m_rectAtGrab.translated(pos - m_grab);
        const QSize size = canvas.imageSize();
        moved.moveLeft(std::clamp(moved.left(), 0, size.width()  - moved.width()));
        moved.moveTop (std::clamp(moved.top(),  0, size.height() - moved.height()));
        m_rect = moved;
    }
}

void Crop::release(Canvas &canvas, const QPoint &pos)
{
    move(canvas, pos);
    m_mode = Mode::None;
    m_hasRect = m_rect.width() > 0 && m_rect.height() > 0;   // un simple clic ne fait pas de cadre
}

bool Crop::keyPress(Canvas &canvas, QKeyEvent *event)
{
    if (!m_hasRect)
        return false;

    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        apply(canvas);
        return true;
    case Qt::Key_Escape:
        cancel();
        return true;
    default:
        return false;
    }
}

void Crop::apply(Canvas &canvas)
{
    if (!m_hasRect)
        return;
    canvas.cropTo(m_rect);
    cancel();
}

void Crop::cancel()
{
    m_hasRect = false;
    m_mode = Mode::None;
    m_rect = QRect();
}

void Crop::drawOverlay(QPainter &painter, const Canvas &canvas, const QPoint &) const
{
    if (m_mode == Mode::None && !m_hasRect)
        return;
    if (m_rect.isEmpty())
        return;

    const QRectF image(QPointF(0, 0), QSizeF(canvas.imageSize()));
    const QRectF keep(m_rect);

    painter.save();

    QPainterPath outside;
    outside.addRect(image);
    outside.addRect(keep);
    outside.setFillRule(Qt::OddEvenFill);
    painter.fillPath(outside, QColor(0, 0, 0, 140));

    QPen thirds(QColor(255, 255, 255, 110), 1);
    thirds.setCosmetic(true);
    painter.setPen(thirds);
    for (int i = 1; i <= 2; ++i) {
        const double x = keep.left() + keep.width()  * i / 3.0;
        const double y = keep.top()  + keep.height() * i / 3.0;
        painter.drawLine(QPointF(x, keep.top()), QPointF(x, keep.bottom()));
        painter.drawLine(QPointF(keep.left(), y), QPointF(keep.right(), y));
    }

    painter.setBrush(Qt::NoBrush);
    QPen white(Qt::white, 1.5);
    white.setCosmetic(true);
    painter.setPen(white);
    painter.drawRect(keep);
    QPen dashed(Qt::black, 1, Qt::DashLine);
    dashed.setCosmetic(true);
    painter.setPen(dashed);
    painter.drawRect(keep);

    const QPointF anchor = painter.transform().map(keep.bottomLeft());
    painter.resetTransform();
    const QString text = m_hasRect
                             ? QString("%1 × %2 px    Entrée : recadrer    Échap : annuler").arg(m_rect.width()).arg(m_rect.height())
                             : QString("%1 × %2 px").arg(m_rect.width()).arg(m_rect.height());
    const QRectF textRect = painter.fontMetrics().boundingRect(text).adjusted(-6, -3, 6, 3);
    const QRectF label = textRect.translated(anchor - textRect.topLeft() + QPointF(0, 6));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 180));
    painter.drawRoundedRect(label, 4, 4);
    painter.setPen(Qt::white);
    painter.drawText(label, Qt::AlignCenter, text);

    painter.restore();
}