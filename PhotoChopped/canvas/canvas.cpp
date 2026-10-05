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

Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
    , m_image(800, 600, QImage::Format_ARGB32_Premultiplied)
{
    m_image.fill(Qt::white);
    setMouseTracking(true);              // recoit les mouvements meme sans clic
    setFocusPolicy(Qt::StrongFocus);     // pour la touche espace
}

// ---------- Outils / couleur / taille ----------

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

void Canvas::newImage(const QSize &size, const QColor &background)
{
    m_image = QImage(size, QImage::Format_ARGB32_Premultiplied);
    m_image.fill(background);

    // grande image : on la fait tenir dans la fenetre ; petite : 100 % centree
    if (size.width() > width() || size.height() > height())
        fitToWindow();
    else
        resetZoom();
}

// ---------- Zoom ----------

void Canvas::setZoomAround(double zoom, const QPointF &widgetAnchor)
{
    zoom = std::clamp(zoom, MIN_ZOOM, MAX_ZOOM);
    if (qFuzzyCompare(zoom, m_zoom))
        return;

    // le point de l'image sous l'ancre reste sous l'ancre apres le zoom
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
    m_offset = QPointF(width() - m_image.width(), height() - m_image.height()) / 2.0;
    emit zoomChanged(m_zoom);
    update();
}

void Canvas::fitToWindow()
{
    const double margin = 20;
    const double zx = (width()  - 2 * margin) / m_image.width();
    const double zy = (height() - 2 * margin) / m_image.height();
    m_zoom = std::clamp(std::min(zx, zy), MIN_ZOOM, MAX_ZOOM);
    m_offset = QPointF(width()  - m_image.width()  * m_zoom,
                       height() - m_image.height() * m_zoom) / 2.0;
    emit zoomChanged(m_zoom);
    update();
}

void Canvas::wheelEvent(QWheelEvent *event)
{
    const double steps = event->angleDelta().y() / 120.0;   // 1 cran = 120
    if (steps == 0)
        return;
    // zoom centre sur la souris
    setZoomAround(m_zoom * std::pow(ZOOM_STEP, steps), event->position());
    event->accept();
}

void Canvas::resizeEvent(QResizeEvent *)
{
    // au premier affichage, on centre l'image
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
{
    if (event->button() == Qt::MiddleButton
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

    const QRectF imageOnScreen(m_offset, QSizeF(m_image.size()) * m_zoom);
    const QRect visible = imageOnScreen.toAlignedRect().intersected(event->rect());

    if (!visible.isEmpty())
        drawCheckerboard(painter, visible);


    painter.translate(m_offset);
    painter.scale(m_zoom, m_zoom);

    painter.setRenderHint(QPainter::SmoothPixmapTransform, m_zoom < 1.0);
    painter.drawImage(0, 0, m_image);

    if (m_tool && m_cursorInside && !m_panning && !m_spaceDown)
        m_tool->drawOverlay(painter, *this, m_cursorPos);
}

// Open the image
void Canvas::setImage(const QImage &image)
{
    m_image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    if (m_image.width() > width() || m_image.height() > height())
        fitToWindow();
    else
        resetZoom();
}