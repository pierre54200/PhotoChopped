#pragma once

#include "brush.h"

class Pencil : public Brush
{
protected:
    void setupPainter(QPainter &painter, const Canvas &canvas) const override;
};