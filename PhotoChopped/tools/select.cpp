#include "select.h"
#include "canvas.h"

#include <QKeyEvent>
#include <QPainter>

static QRect rectBetween(const QPoint &a, const QPoint &b)
{
    return QRect(std::min(a.x(), b.x()), std::min(a.y(), b.y()),
                 std::abs(b.x() - a.x()), std::abs(b.y() - a.y()));
}

void Select::press(Canvas &, const QPoint &pos)
{
    if (m_hasRect && m_rect.contains(pos)) {
        // clic dans la sélection : on va la déplacer
        m_mode = Mode::Moving;
        m_grab = pos;
        m_rectAtGrab = m_rect;
        m_cut = false;               // on ne coupe qu'au premier vrai mouvement
    } else {
        // clic ailleurs : nouvelle sélection
        m_mode = Mode::Creating;
        m_start = pos;
        m_rect = QRect(pos, QSize(0, 0));
        m_hasRect = false;
    }
}

void Select::cut(Canvas &canvas)
{
    QImage &image = canvas.image();
    m_floating = image.copy(m_rectAtGrab);          // on garde les pixels...

    QPainter painter(&image);
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.fillRect(m_rectAtGrab, Qt::transparent); // ...et on vide leur ancienne place
    painter.end();

    m_cut = true;
    canvas.refresh(m_rectAtGrab);
}

void Select::move(Canvas &canvas, const QPoint &pos)
{
    if (m_mode == Mode::Creating) {
        m_rect = rectBetween(m_start, pos).intersected(QRect(QPoint(0, 0), canvas.imageSize()));
    } else if (m_mode == Mode::Moving) {
        if (!m_cut && pos != m_grab)
            cut(canvas);
        m_rect = m_rectAtGrab.translated(pos - m_grab);   // le canvas redessine l'aperçu
    }
}

void Select::release(Canvas &canvas, const QPoint &pos)
{
    move(canvas, pos);

    if (m_mode == Mode::Moving && m_cut) {
        // on pose les pixels à leur nouvelle place, par-dessus ce qu'il y a dessous
        QPainter painter(&canvas.image());
        painter.drawImage(m_rect.topLeft(), m_floating);
        painter.end();
        canvas.refresh(m_rect);

        m_floating = QImage();
        m_cut = false;
        // la partie sortie de l'image est perdue : la sélection reste dans l'image
        m_rect = m_rect.intersected(QRect(QPoint(0, 0), canvas.imageSize()));
    }

    m_mode = Mode::None;
    m_hasRect = m_rect.width() > 0 && m_rect.height() > 0;   // un simple clic désélectionne
}

bool Select::keyPress(Canvas &, QKeyEvent *event)
{
    if (m_hasRect && event->key() == Qt::Key_Escape) {
        clear();
        return true;
    }
    return false;
}

void Select::clear()
{
    m_hasRect = false;
    m_mode = Mode::None;
    m_rect = QRect();
    m_floating = QImage();
    m_cut = false;
}

void Select::drawOverlay(QPainter &painter, const Canvas &, const QPoint &) const
{
    if (m_rect.isEmpty() || (m_mode == Mode::None && !m_hasRect))
        return;

    // pendant un déplacement : les pixels attrapés suivent la souris
    if (m_mode == Mode::Moving && m_cut)
        painter.drawImage(m_rect.topLeft(), m_floating);

    // contour : blanc, puis pointillés noirs, 1 pixel à l'écran quel que soit le zoom
    painter.setBrush(Qt::NoBrush);
    painter.setRenderHint(QPainter::Antialiasing, false);
    QPen white(Qt::white, 1);
    white.setCosmetic(true);
    painter.setPen(white);
    painter.drawRect(QRectF(m_rect));
    QPen dashed(Qt::black, 1, Qt::DashLine);
    dashed.setCosmetic(true);
    painter.setPen(dashed);
    painter.drawRect(QRectF(m_rect));
}