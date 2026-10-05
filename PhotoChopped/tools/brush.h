#ifndef BRUSH_H
#define BRUSH_H

#include "tool.h"
#include <QPainter>

class Brush : public Tool
{
public:
    void press(Canvas &canvas, const QPoint &pos) override;
    void move(Canvas &canvas, const QPoint &pos) override;

protected:
    // trace un trait entre deux points ; la gomme reutilise cette fonction
    void drawSegment(Canvas &canvas, const QPoint &from, const QPoint &to);

    // reglage du QPainter avant de dessiner : la gomme le redefinit
    virtual void setupPainter(QPainter &painter, const Canvas &canvas) const;

private:
    QPoint m_lastPoint;
};

#endif // BRUSH_H
