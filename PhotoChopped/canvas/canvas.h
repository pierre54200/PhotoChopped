#pragma once

#include <QWidget>
#include <QImage>
#include <QColor>
#include <QPoint>

class Tool;

class Canvas : public QWidget
{
    Q_OBJECT

public:
    explicit Canvas(QWidget *parent = nullptr);

    void setTool(Tool *tool);
    Tool *tool() const { return m_tool; }

    void setBrushSize(int size);
    int brushSize() const { return m_brushSize; }

    void setColor(const QColor &color) { m_color = color; }
    QColor color() const { return m_color; }

    QImage &image() { return m_image; }
    const QImage &image() const { return m_image; }

    void refresh(const QRect &area) { update(area); }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void drawCheckerboard(QPainter &painter, const QRect &rect);

    QImage m_image;
    Tool  *m_tool = nullptr;
    QColor m_color = Qt::black;
    int    m_brushSize = 10;

    bool   m_drawing = false;
    QPoint m_cursorPos;
    bool   m_cursorInside = false;
};