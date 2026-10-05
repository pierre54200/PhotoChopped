#include "bucket.h"
#include "canvas.h"

#include <vector>
#include <cstdlib>

bool Bucket::isClose(QRgb a, QRgb b) const
{
    return std::abs(qRed(a)   - qRed(b))   <= m_tolerance
           && std::abs(qGreen(a) - qGreen(b)) <= m_tolerance
           && std::abs(qBlue(a)  - qBlue(b))  <= m_tolerance
           && std::abs(qAlpha(a) - qAlpha(b)) <= m_tolerance;
}

void Bucket::move(Canvas &, const QPoint &)
{
}

void Bucket::press(Canvas &canvas, const QPoint &pos)
{
    QImage &image = canvas.image();
    if (!image.rect().contains(pos))
        return;

    const int w = image.width();
    const int h = image.height();

    auto pixel = [&](int x, int y) -> QRgb & {
        return reinterpret_cast<QRgb *>(image.scanLine(y))[x];
    };

    const QRgb target = pixel(pos.x(), pos.y());
    const QRgb fill   = qPremultiply(canvas.color().rgba());

    std::vector<bool> done(size_t(w) * h, false);
    auto canFill = [&](int x, int y) {
        return !done[size_t(y) * w + x] && isClose(pixel(x, y), target);
    };

    std::vector<QPoint> stack{ pos };
    int minX = pos.x(), maxX = pos.x(), minY = pos.y(), maxY = pos.y();

    while (!stack.empty()) {
        const QPoint p = stack.back();
        stack.pop_back();
        const int y = p.y();
        if (!canFill(p.x(), y))
            continue;

        int left = p.x(), right = p.x();
        while (left > 0 && canFill(left - 1, y))       --left;
        while (right < w - 1 && canFill(right + 1, y)) ++right;

        for (int x = left; x <= right; ++x) {
            pixel(x, y) = fill;
            done[size_t(y) * w + x] = true;
        }

        minX = std::min(minX, left);  maxX = std::max(maxX, right);
        minY = std::min(minY, y);     maxY = std::max(maxY, y);

        for (int ny : { y - 1, y + 1 }) {
            if (ny < 0 || ny >= h)
                continue;
            bool inRun = false;
            for (int x = left; x <= right; ++x) {
                const bool ok = canFill(x, ny);
                if (ok && !inRun)
                    stack.push_back(QPoint(x, ny));
                inRun = ok;
            }
        }
    }

    canvas.refresh(QRect(QPoint(minX, minY), QPoint(maxX, maxY)));
}