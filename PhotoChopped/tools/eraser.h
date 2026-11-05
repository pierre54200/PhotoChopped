#pragma once

#include "brush.h"

class Eraser : public Brush
{
protected:
    void setupPainter(QPainter &painter, const Canvas &canvas) const override;
};
