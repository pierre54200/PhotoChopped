#ifndef ERASER_H
#define ERASER_H

#include "brush.h"

// La gomme trace comme un pinceau, mais rend les pixels transparents.
class Eraser : public Brush
{
public:
    QCursor cursor() const override { return Qt::BlankCursor; }
    void drawOverlay(QPainter &painter, const Canvas &canvas, const QPoint &cursorPos) const override;

protected:
    void setupPainter(QPainter &painter, const Canvas &canvas) const override;
};

#endif // ERASER_H
