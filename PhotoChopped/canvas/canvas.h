#pragma once

#include <QWidget>
#include <QImage>
#include <QColor>
#include <QPoint>
#include <QPointF>

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

    void setColor(const QColor &color);
    QColor color() const { return m_color; }

    QImage &image() { return m_image; }
    const QImage &image() const { return m_image; }

    // New
    void newImage(const QSize &size, const QColor &background = Qt::white);

    // Open
    void setImage(const QImage &image);

    void refresh(const QRect &imageArea);

    // Zoom
    double zoom() const { return m_zoom; }

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitToWindow();

signals:
    void colorChanged(const QColor &color);
    void zoomChanged(double zoom);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    QPointF widgetToImage(const QPointF &p) const { return (p - m_offset) / m_zoom; }
    QPointF imageToWidget(const QPointF &p) const { return p * m_zoom + m_offset; }
    QPoint  imagePixelAt(const QPointF &widgetPos) const;

    void setZoomAround(double zoom, const QPointF &widgetAnchor);
    void drawCheckerboard(QPainter &painter, const QRect &rect);
    void restoreToolCursor();

    QImage m_image;

    // Tool info
    Tool  *m_tool = nullptr;
    QColor m_color = Qt::black;
    int    m_brushSize = 10;

    // Zoom
    QPointF m_offset;
    double  m_zoom = 1.0;
    bool    m_centered = false;

    // Movement
    bool    m_panning = false;
    bool    m_spaceDown = false;
    QPointF m_panStart;
    QPointF m_offsetAtPanStart;

    // Dessin
    bool   m_drawing = false;
    QPoint m_cursorPos;        // en pixels de l'image
    bool   m_cursorInside = false;
};