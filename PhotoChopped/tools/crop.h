#pragma once

#include "tool.h"
#include <QRect>

class Crop : public Tool
{
public:
    void press(Canvas &canvas, const QPoint &pos) override;
    void move(Canvas &canvas, const QPoint &pos) override;
    void release(Canvas &canvas, const QPoint &pos) override;
    bool keyPress(Canvas &canvas, QKeyEvent *event) override;

    void drawOverlay(QPainter &painter, const Canvas &canvas, const QPoint &cursorPos) const override;
    bool overlayFollowsCursor() const override { return false; }

    bool hasSelection() const { return m_hasRect; }
    void apply(Canvas &canvas);
    void cancel();

private:
    enum class Mode { None, Creating, Moving };

    QRect clampToImage(const QRect &rect, const Canvas &canvas) const;

    Mode   m_mode = Mode::None;
    bool   m_hasRect = false;
    QRect  m_rect;
    QPoint m_start;
    QPoint m_grab;
    QRect  m_rectAtGrab;
};
