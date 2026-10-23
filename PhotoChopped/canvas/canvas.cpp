#include "canvas.h"
#include "tool.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <cmath>
#include <algorithm>

static constexpr double MIN_ZOOM = 0.05;
static constexpr double MAX_ZOOM = 32.0;
static constexpr double ZOOM_STEP = 1.25;
static constexpr int    MAX_HISTORY = 30;

static QImage applyAdjustments(const QImage &source, int brightness, int saturation)
{
    if (brightness == 0 && saturation == 0)
        return source;

    QImage result = source.copy();
    const int offset = brightness * 255 / 100;
    const int satFactor = 256 + saturation * 256 / 100;

    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            const QRgb p = line[x];
            const int a = qAlpha(p);
            if (a == 0)
                continue;
            int r = qRed(p), g = qGreen(p), b = qBlue(p);

            if (offset != 0) {
                const int o = offset * a / 255;
                r += o; g += o; b += o;
            }
            if (satFactor != 256) {
                const int gray = (r * 77 + g * 150 + b * 29) >> 8;
                r = gray + (((r - gray) * satFactor) >> 8);
                g = gray + (((g - gray) * satFactor) >> 8);
                b = gray + (((b - gray) * satFactor) >> 8);
            }
            line[x] = qRgba(std::clamp(r, 0, a), std::clamp(g, 0, a), std::clamp(b, 0, a), a);
        }
    }
    return result;
}

static Layer makeLayer(const QString &name, const QImage &image)
{
    Layer layer;
    layer.name = name;
    layer.image = image;
    return layer;
}

Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
{
    QImage background(800, 600, QImage::Format_ARGB32_Premultiplied);
    background.fill(Qt::white);
    m_layers.append(makeLayer(QStringLiteral("Arrière-plan"), background));
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void Canvas::setTool(Tool *tool)
{
    m_tool = tool;
    setCursor(m_tool ? m_tool->cursor() : QCursor(Qt::ArrowCursor));
    update();
}

void Canvas::setBrushSize(int size)
{
    m_brushSize = qMax(1, size);
    update();
}

void Canvas::setColor(const QColor &color)
{
    if (color == m_color)
        return;
    m_color = color;
    emit colorChanged(color);
}

void Canvas::refresh(const QRect &imageArea)
{
    const QRectF r(imageToWidget(imageArea.topLeft()), QSizeF(imageArea.size()) * m_zoom);
    update(r.toAlignedRect().adjusted(-2, -2, 2, 2));
}

void Canvas::resetDocument(const QImage &background)
{
    m_layers.clear();
    m_layers.append(makeLayer(QStringLiteral("Arrière-plan"),
                              background.convertToFormat(QImage::Format_ARGB32_Premultiplied)));
    m_current = 0;
    m_layerCounter = 1;
    emit layersChanged();
    emit currentLayerChanged(m_current);
    emit historyChanged();

    const QSize size = imageSize();
    if (size.width() > width() || size.height() > height())
        fitToWindow();
    else
        resetZoom();
}

void Canvas::newImage(const QSize &size, const QColor &background)
{
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(background);
    resetDocument(image);
}

void Canvas::setImage(const QImage &image)
{
    resetDocument(image);
}

void Canvas::setLayers(const QVector<Layer> &layers, int current, int layerCounter)
{
    if (layers.isEmpty())
        return;
    m_layers = layers;
    m_current = std::clamp(current, 0, int(m_layers.size()) - 1);
    m_layerCounter = std::max(layerCounter, int(m_layers.size()));
    emit layersChanged();
    emit currentLayerChanged(m_current);
    emit historyChanged();

    const QSize size = imageSize();
    if (size.width() > width() || size.height() > height())
        fitToWindow();
    else
        resetZoom();
}

QImage Canvas::flattenedImage() const
{
    QImage result(imageSize(), QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    for (int i = 0; i < m_layers.size(); ++i) {
        if (!m_layers[i].visible)
            continue;
        painter.setOpacity(m_layers[i].opacity);
        painter.drawImage(0, 0, renderedLayer(i));
    }
    return result;
}

QImage Canvas::makeEmptyLayerImage() const
{
    QImage image(imageSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    return image;
}

void Canvas::setCurrentLayer(int index)
{
    if (index < 0 || index >= m_layers.size() || index == m_current)
        return;
    m_current = index;
    emit currentLayerChanged(m_current);
    emit historyChanged();
}

void Canvas::addLayer()
{
    ++m_layerCounter;
    Layer layer = makeLayer(QString("Calque %1").arg(m_layerCounter), makeEmptyLayerImage());
    m_layers.insert(m_current + 1, layer);
    m_current += 1;
    emit layersChanged();
    emit currentLayerChanged(m_current);
    emit historyChanged();
    update();
}

void Canvas::duplicateLayer()
{
    Layer copy = m_layers[m_current];
    copy.name += " (copie)";
    copy.undoStack.clear();
    copy.redoStack.clear();
    m_layers.insert(m_current + 1, copy);
    m_current += 1;
    emit layersChanged();
    emit currentLayerChanged(m_current);
    emit historyChanged();
    update();
}

void Canvas::removeLayer()
{
    if (m_layers.size() <= 1)
        return;
    m_layers.removeAt(m_current);
    m_current = qMin(m_current, int(m_layers.size()) - 1);
    emit layersChanged();
    emit currentLayerChanged(m_current);
    emit historyChanged();
    update();
}

void Canvas::moveLayerUp()
{
    if (m_current >= m_layers.size() - 1)
        return;
    m_layers.swapItemsAt(m_current, m_current + 1);
    m_current += 1;
    emit layersChanged();
    emit currentLayerChanged(m_current);
    emit historyChanged();
    update();
}

void Canvas::moveLayerDown()
{
    if (m_current <= 0)
        return;
    m_layers.swapItemsAt(m_current, m_current - 1);
    m_current -= 1;
    emit layersChanged();
    emit currentLayerChanged(m_current);
    emit historyChanged();
    update();
}

void Canvas::setLayerVisible(int index, bool visible)
{
    if (index < 0 || index >= m_layers.size() || m_layers[index].visible == visible)
        return;
    m_layers[index].visible = visible;
    emit layersChanged();
    update();
}

void Canvas::setLayerOpacity(int index, double opacity)
{
    if (index < 0 || index >= m_layers.size())
        return;
    m_layers[index].opacity = std::clamp(opacity, 0.0, 1.0);
    update();
}

void Canvas::setLayerBrightness(int index, int brightness)
{
    if (index < 0 || index >= m_layers.size())
        return;
    m_layers[index].brightness = std::clamp(brightness, -100, 100);
    update();
    emit layerEdited(index);
}

void Canvas::setLayerSaturation(int index, int saturation)
{
    if (index < 0 || index >= m_layers.size())
        return;
    m_layers[index].saturation = std::clamp(saturation, -100, 100);
    update();
    emit layerEdited(index);
}

QImage Canvas::renderedLayer(int index) const
{
    const Layer &layer = m_layers[index];
    if (layer.brightness == 0 && layer.saturation == 0)
        return layer.image;

    if (layer.cache.isNull()
        || layer.cacheSourceKey  != layer.image.cacheKey()
        || layer.cacheBrightness != layer.brightness
        || layer.cacheSaturation != layer.saturation) {
        layer.cache = applyAdjustments(layer.image, layer.brightness, layer.saturation);
        layer.cacheSourceKey  = layer.image.cacheKey();
        layer.cacheBrightness = layer.brightness;
        layer.cacheSaturation = layer.saturation;
    }
    return layer.cache;
}

void Canvas::renameLayer(int index, const QString &name)
{
    if (index < 0 || index >= m_layers.size() || name.trimmed().isEmpty())
        return;
    m_layers[index].name = name.trimmed();
    emit layersChanged();
}

void Canvas::setZoomAround(double zoom, const QPointF &widgetAnchor)
{
    zoom = std::clamp(zoom, MIN_ZOOM, MAX_ZOOM);
    if (qFuzzyCompare(zoom, m_zoom))
        return;

    const QPointF imagePoint = widgetToImage(widgetAnchor);
    m_zoom = zoom;
    m_offset = widgetAnchor - imagePoint * m_zoom;

    emit zoomChanged(m_zoom);
    update();
}

void Canvas::zoomIn()  { setZoomAround(m_zoom * ZOOM_STEP, rect().center()); }
void Canvas::zoomOut() { setZoomAround(m_zoom / ZOOM_STEP, rect().center()); }

void Canvas::resetZoom()
{
    m_zoom = 1.0;
    m_offset = QPointF(width() - imageSize().width(), height() - imageSize().height()) / 2.0;
    emit zoomChanged(m_zoom);
    update();
}

void Canvas::fitToWindow()
{
    const double margin = 20;
    const QSize size = imageSize();
    const double zx = (width()  - 2 * margin) / size.width();
    const double zy = (height() - 2 * margin) / size.height();
    m_zoom = std::clamp(std::min(zx, zy), MIN_ZOOM, MAX_ZOOM);
    m_offset = QPointF(width()  - size.width()  * m_zoom,
                       height() - size.height() * m_zoom) / 2.0;
    emit zoomChanged(m_zoom);
    update();
}

void Canvas::wheelEvent(QWheelEvent *event)
{
    const double steps = event->angleDelta().y() / 120.0;
    if (steps == 0)
        return;
    setZoomAround(m_zoom * std::pow(ZOOM_STEP, steps), event->position());
    event->accept();
}

void Canvas::resizeEvent(QResizeEvent *)
{
    if (!m_centered && width() > 0 && height() > 0) {
        m_centered = true;
        resetZoom();
    }
}

QPoint Canvas::imagePixelAt(const QPointF &widgetPos) const
{
    const QPointF p = widgetToImage(widgetPos);
    return QPoint(int(std::floor(p.x())), int(std::floor(p.y())));
}

void Canvas::mousePressEvent(QMouseEvent *event)
{    if (event->button() == Qt::MiddleButton
        || (event->button() == Qt::LeftButton && m_spaceDown)) {
        m_panning = true;
        m_panStart = event->position();
        m_offsetAtPanStart = m_offset;
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    if (event->button() != Qt::LeftButton || !m_tool)
        return;
    m_drawing = true;
    m_beforeStroke = m_layers[m_current].image;
    m_tool->press(*this, imagePixelAt(event->position()));
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if (m_panning) {
        m_offset = m_offsetAtPanStart + (event->position() - m_panStart);
        update();
        return;
    }

    m_cursorPos = imagePixelAt(event->position());
    m_cursorInside = true;

    if (m_drawing && m_tool && (event->buttons() & Qt::LeftButton))
        m_tool->move(*this, m_cursorPos);

    update();
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_panning && (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton)) {
        m_panning = false;
        restoreToolCursor();
        return;
    }

    if (event->button() != Qt::LeftButton || !m_drawing)
        return;
    m_drawing = false;
    if (m_tool)
        m_tool->release(*this, imagePixelAt(event->position()));
    commitStroke();
}

void Canvas::commitStroke()
{
    Layer &layer = m_layers[m_current];

    if (layer.image.cacheKey() == m_beforeStroke.cacheKey()) {
        m_beforeStroke = QImage();
        return;
    }

    layer.undoStack.append(m_beforeStroke);
    if (layer.undoStack.size() > MAX_HISTORY)
        layer.undoStack.removeFirst();
    layer.redoStack.clear();
    m_beforeStroke = QImage();

    emit layerEdited(m_current);
    emit historyChanged();
}

void Canvas::undo()
{
    Layer &layer = m_layers[m_current];
    if (m_drawing || layer.undoStack.isEmpty())
        return;
    layer.redoStack.append(layer.image);
    layer.image = layer.undoStack.takeLast();
    update();
    emit layerEdited(m_current);
    emit historyChanged();
}

void Canvas::redo()
{
    Layer &layer = m_layers[m_current];
    if (m_drawing || layer.redoStack.isEmpty())
        return;
    layer.undoStack.append(layer.image);
    layer.image = layer.redoStack.takeLast();
    update();
    emit layerEdited(m_current);
    emit historyChanged();
}

void Canvas::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        m_spaceDown = true;
        if (!m_panning)
            setCursor(Qt::OpenHandCursor);
        update();
        return;
    }

    if (event->modifiers() & Qt::ControlModifier) {
        switch (event->key()) {
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            zoomIn();
            return;
        case Qt::Key_Minus:
            zoomOut();
            return;
        case Qt::Key_0:
            resetZoom();
            return;
        }
    }

    const double step = (event->modifiers() & Qt::ShiftModifier) ? 200 : 50;
    QPointF delta;
    switch (event->key()) {
    case Qt::Key_Left:  delta = QPointF( step, 0); break;
    case Qt::Key_Right: delta = QPointF(-step, 0); break;
    case Qt::Key_Up:    delta = QPointF(0,  step); break;
    case Qt::Key_Down:  delta = QPointF(0, -step); break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }
    m_offset += delta;
    update();
}

void Canvas::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        m_spaceDown = false;
        if (!m_panning)
            restoreToolCursor();
        update();
        return;
    }
    QWidget::keyReleaseEvent(event);
}

void Canvas::restoreToolCursor()
{
    if (m_spaceDown)
        setCursor(Qt::OpenHandCursor);
    else
        setCursor(m_tool ? m_tool->cursor() : QCursor(Qt::ArrowCursor));
}

void Canvas::leaveEvent(QEvent *)
{
    m_cursorInside = false;
    update();
}

void Canvas::drawCheckerboard(QPainter &painter, const QRect &rect)
{
    const int s = 10;
    const QColor light(0xcc, 0xcc, 0xcc), dark(0x99, 0x99, 0x99);
    for (int y = rect.top(); y <= rect.bottom(); y += s)
        for (int x = rect.left(); x <= rect.right(); x += s) {
            const bool odd = (((x - rect.left()) / s) + ((y - rect.top()) / s)) % 2;
            painter.fillRect(QRect(x, y, s, s).intersected(rect), odd ? dark : light);
        }
}

void Canvas::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.fillRect(event->rect(), QColor(0x3a, 0x3a, 0x3a));

    const QRectF imageOnScreen(m_offset, QSizeF(imageSize()) * m_zoom);
    const QRect visible = imageOnScreen.toAlignedRect().intersected(event->rect());

    if (!visible.isEmpty())
        drawCheckerboard(painter, visible);

    painter.translate(m_offset);
    painter.scale(m_zoom, m_zoom);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, m_zoom < 1.0);
    for (int i = 0; i < m_layers.size(); ++i) {
        if (!m_layers[i].visible)
            continue;
        painter.setOpacity(m_layers[i].opacity);
        painter.drawImage(0, 0, renderedLayer(i));
    }
    painter.setOpacity(1.0);

    if (m_tool && m_cursorInside && !m_panning && !m_spaceDown)
        m_tool->drawOverlay(painter, *this, m_cursorPos);
}
