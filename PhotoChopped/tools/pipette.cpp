#include "pipette.h"
#include "canvas.h"

void Pipette::press(Canvas &canvas, const QPoint &pos)
{
    pick(canvas, pos);
}

void Pipette::move(Canvas &canvas, const QPoint &pos)
{
    pick(canvas, pos);      // la couleur suit la souris tant que le clic est maintenu
}

void Pipette::pick(Canvas &canvas, const QPoint &pos)
{
    if (!canvas.image().rect().contains(pos))   // clic en dehors de l'image
        return;

    QColor color = canvas.image().pixelColor(pos);
    if (color.alpha() == 0)                      // zone effacée : rien à prendre
        return;

    color.setAlpha(255);
    canvas.setColor(color);
}