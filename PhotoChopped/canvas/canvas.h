#pragma once

#include <QWidget>
#include <QImage>
#include <QColor>
#include <QPoint>
#include <QPointF>
#include <QVector>

#include "layer.h"

static constexpr double MIN_ZOOM = 0.05;
static constexpr double MAX_ZOOM = 32.0;
static constexpr double ZOOM_STEP = 1.25;
static constexpr int    MAX_HISTORY = 30;

class Tool;

class Canvas : public QWidget
{
    Q_OBJECT

public:
    explicit Canvas(QWidget *parent = nullptr);

    // Tool
    void setTool(Tool *tool);
    Tool *tool() const { return m_tool; }

    // Brush Size
    void setBrushSize(int size);
    int brushSize() const { return m_brushSize; }

    // Color
    void setColor(const QColor &color);
    QColor color() const { return m_color; }

    // Image
    QImage &image() { return m_layers[m_current].image; }
    const QImage &image() const { return m_layers[m_current].image; }

    // Image Size
    QSize imageSize() const { return m_layers.first().image.size(); }

    // Fusionne tout les calques en une image
    QImage flattenedImage() const;

    // Nouveau
    void newImage(const QSize &size, const QColor &background = Qt::white);

    // Ouvrir une image
    void setImage(const QImage &image);

    // Ouvrir un .pcp
    void setLayers(const QVector<Layer> &layers, int current, int layerCounter);

    // Recadrer : garde seulement la zone 'area' de tous les calques
    void cropTo(const QRect &area);

    // Layers
    int layerCount() const { return m_layers.size(); }
    const Layer &layer(int index) const { return m_layers[index]; }
    int currentLayer() const { return m_current; }
    int layerCounter() const { return m_layerCounter; }
    void setCurrentLayer(int index);
    void addLayer();
    void duplicateLayer();
    void removeLayer();
    void moveLayerUp();
    void moveLayerDown();
    void setLayerVisible(int index, bool visible);
    void setLayerOpacity(int index, double opacity);
    void setLayerBrightness(int index, int brightness);
    void setLayerSaturation(int index, int saturation);
    QImage renderedLayer(int index) const;
    void renameLayer(int index, const QString &name);

    // Ctrl+Z
    bool canUndo() const { return !m_layers[m_current].undoStack.isEmpty(); }
    // Ctrl+Shift+Z
    bool canRedo() const { return !m_layers[m_current].redoStack.isEmpty(); }

    // Refresh l'image après l'utilisation d'un outil
    void refresh(const QRect &imageArea);

    // Zoom
    double zoom() const { return m_zoom; }

public slots:
    // Zoom
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitToWindow();

    // Ctrl+Z
    void undo();
    // Ctrl+Shift+Z
    void redo();

signals:
    void colorChanged(const QColor &color);
    void layersChanged();
    void currentLayerChanged(int index);
    void layerEdited(int index);
    void historyChanged();
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

    QImage makeEmptyLayerImage() const;
    void   resetDocument(const QImage &background);
    void   commitStroke();

    QVector<Layer> m_layers;
    int    m_current = 0;
    int    m_layerCounter = 1;
    Tool  *m_tool = nullptr;
    QColor m_color = Qt::black;
    int    m_brushSize = 10;

    QPointF m_offset;
    double  m_zoom = 1.0;
    bool    m_centered = false;

    bool    m_panning = false;
    bool    m_spaceDown = false;
    QPointF m_panStart;
    QPointF m_offsetAtPanStart;

    bool   m_drawing = false;
    QImage m_beforeStroke;
    QPoint m_cursorPos;
    bool   m_cursorInside = false;
};