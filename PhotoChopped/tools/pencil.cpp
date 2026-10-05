#include "pencil.h"
#include "canvas.h"

void Pencil::setupPainter(QPainter &painter, const Canvas &canvas) const
{
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QPen(canvas.color(), canvas.brushSize(),
                        Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}