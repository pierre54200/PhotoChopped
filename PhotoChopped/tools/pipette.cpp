#include "pipette.h"
#include "canvas.h"

void Pipette::press(Canvas &canvas, const QPoint &pos)
{
    pick(canvas, pos);
}

void Pipette::move(Canvas &canvas, const QPoint &pos)
{
    pick(canvas, pos);
}

void Pipette::pick(Canvas &canvas, const QPoint &pos)
{
    if (!canvas.image().rect().contains(pos))
        return;

    QColor color = canvas.image().pixelColor(pos);
    if (color.alpha() == 0)
        return;

    color.setAlpha(255);
    canvas.setColor(color);
}