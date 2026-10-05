#ifndef PIPETTE_H
#define PIPETTE_H

#include "tool.h"

class Pipette : public Tool
{
public:
    void press(Canvas &canvas, const QPoint &pos) override;
    void move(Canvas &canvas, const QPoint &pos) override;

private:
    void pick(Canvas &canvas, const QPoint &pos);
};

#endif // PIPETTE_H
