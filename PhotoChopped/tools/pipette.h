#pragma once

#include "tool.h"

class Pipette : public Tool
{
public:
    // Action
    void press(Canvas &canvas, const QPoint &pos) override;
    void move(Canvas &canvas, const QPoint &pos) override;

private:
    // Action
    void pick(Canvas &canvas, const QPoint &pos);
};
