#include "canvas.h"
#include "tools/tool.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>

Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
    , m_image(800, 600, QImage::Format_ARGB32_Premultiplied)
{
    m_image.fill(Qt::white);
    setMinimumSize(m_image.size());
    setMouseTracking(true);   // recoit les mouvements meme sans clic
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

void Canvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || !m_tool)
        return;
    m_drawing = true;
    m_tool->press(*this, event->position().toPoint());
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    m_cursorPos = event->position().toPoint();
    m_cursorInside = true;

    if (m_drawing && m_tool && (event->buttons() & Qt::LeftButton))
        m_tool->move(*this, m_cursorPos);

    update();   // pour redessiner l'overlay de l'outil (cercle de la gomme...)
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || !m_drawing)
        return;
    m_drawing = false;
    if (m_tool)
        m_tool->release(*this, event->position().toPoint());
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
    for (int y = rect.top() - rect.top() % s; y <= rect.bottom(); y += s)
        for (int x = rect.left() - rect.left() % s; x <= rect.right(); x += s)
            painter.fillRect(x, y, s, s, ((x / s + y / s) % 2) ? dark : light);
}

void Canvas::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    const QRect dirty = event->rect().intersected(m_image.rect());

    // damier gris sous l'image : montre les zones transparentes
    painter.save();
    painter.setClipRect(dirty);
    drawCheckerboard(painter, dirty);
    painter.drawImage(dirty, m_image, dirty);
    painter.restore();

    if (m_tool && m_cursorInside)
        m_tool->drawOverlay(painter, *this, m_cursorPos);
}