#ifndef CANVAS_H
#define CANVAS_H

#include <QColor>
#include <QImage>
#include <QWidget>

// La zone de dessin
class Canvas : public QWidget
{
    Q_OBJECT

public:
    explicit Canvas(QWidget *parent = nullptr);

    void setPenColor(const QColor &color) { m_penColor = color; }
    void setPenWidth(int width) { m_penWidth = width; }

    const QImage &image() const { return m_image; }

protected:
    void paintEvent(QPaintEvent *event) override;          // affichage
    void mousePressEvent(QMouseEvent *event) override;     // clic
    void mouseMoveEvent(QMouseEvent *event) override;      // déplacement
    void mouseReleaseEvent(QMouseEvent *event) override;   // relâchement

private:
    void drawLineTo(const QPointF &end);

    QImage m_image;               // les pixels de l'image
    QPointF m_lastPoint;          // dernière position de la souris pendant le trait
    bool m_drawing = false;

    QColor m_penColor = Qt::black;
    int m_penWidth = 5;
};

#endif // CANVAS_H