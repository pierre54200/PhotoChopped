#ifndef BUCKET_H
#define BUCKET_H

#include "tool.h"
#include <QRgb>

class Bucket : public Tool
{
public:
    void press(Canvas &canvas, const QPoint &pos) override;
    void move(Canvas &canvas, const QPoint &pos) override;

    void setTolerance(int tolerance) { m_tolerance = qBound(0, tolerance, 255); }
    int tolerance() const { return m_tolerance; }

private:
    bool isClose(QRgb a, QRgb b) const;

    int m_tolerance = 30;
};

#endif // BUCKET_H
