#include "eraser.h"
#include "canvas.h"

void Eraser::setupPainter(QPainter &painter, const Canvas &canvas) const
{
    // "Clear" remplace les pixels par du transparent au lieu de peindre par-dessus
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.setPen(QPen(Qt::black, canvas.brushSize(),
                        Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}

void Eraser::drawOverlay(QPainter &painter, const Canvas &canvas, const QPoint &cursorPos) const
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