#pragma once

#include "tool.h"
#include <QRgb>

class Bucket : public Tool
{
public:
    // Action
    void press(Canvas &canvas, const QPoint &pos) override;
    void move(Canvas &canvas, const QPoint &pos) override;

    // Tolerance
    void setTolerance(int tolerance) { m_tolerance = qBound(0, tolerance, 255); }
    int tolerance() const { return m_tolerance; }

private:
    bool isClose(QRgb a, QRgb b) const;

    int m_tolerance = 30;
};
