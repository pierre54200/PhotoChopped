#ifndef ERASER_H
#define ERASER_H

#include "brush.h"

class Eraser : public Brush
{

protected:
    void setupPainter(QPainter &painter, const Canvas &canvas) const override;
};

#endif // ERASER_H
