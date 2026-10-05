#include "eraser.h"
#include "canvas.h"

void Eraser::setupPainter(QPainter &painter, const Canvas &canvas) const
{
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.setPen(QPen(Qt::black, canvas.brushSize(),
                        Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}