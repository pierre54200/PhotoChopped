#ifndef PENCIL_H
#define PENCIL_H

#include "brush.h"

class Pencil : public Brush
{
protected:
    void setupPainter(QPainter &painter, const Canvas &canvas) const override;
};

#endif // PENCIL_H
