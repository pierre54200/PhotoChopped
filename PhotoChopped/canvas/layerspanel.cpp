#include "layerspanel.h"
#include "canvas.h"

#include <QListWidget>
#include <QToolButton>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QGridLayout>
#include <QEvent>

static const QSize THUMB_SIZE(40, 40);

static QIcon makeThumbnail(const QImage &image)
{
    QPixmap pix(THUMB_SIZE);
    pix.fill(Qt::transparent);

    const QImage scaled = image.scaled(THUMB_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    const QRect area((THUMB_SIZE.width()  - scaled.width())  / 2,
                     (THUMB_SIZE.height() - scaled.height()) / 2,
                     scaled.width(), scaled.height());

    QPainter p(&pix);
    const int s = 5;
    for (int y = area.top(); y <= area.bottom(); y += s)
        for (int x = area.left(); x <= area.right(); x += s)
            p.fillRect(QRect(x, y, s, s).intersected(area),
                       (((x - area.left()) / s + (y - area.top()) / s) % 2) ? QColor(0x99, 0x99, 0x99)
                                                                            : QColor(0xcc, 0xcc, 0xcc));
    p.drawImage(area.topLeft(), scaled);
    p.setPen(QColor(0x55, 0x55, 0x55));
    p.drawRect(area.adjusted(0, 0, -1, -1));
    return QIcon(pix);
}

static QToolButton *makeButton(const QString &icon, const QString &tip)
{
    auto *b = new QToolButton;
    b->setIcon(QIcon(":/icons/dark/" + icon + ".svg"));
    b->setToolTip(tip);
    b->setAutoRaise(true);
    return b;
}

LayersPanel::LayersPanel(Canvas *canvas, QWidget *parent)
    : QWidget(parent)
    , m_canvas(canvas)
{
    auto makeSlider = [](int min, int max) {
        auto *s = new QSlider(Qt::Horizontal);
        s->setRange(min, max);
        return s;
    };
    auto makeValueLabel = [] {
        auto *l = new QLabel;
        l->setMinimumWidth(40);
        l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        l->setToolTip("Double-clic : valeur par défaut");
        return l;
    };
    m_opacity    = makeSlider(0, 100);     m_opacityLabel    = makeValueLabel();
    m_brightness = makeSlider(-100, 100);  m_brightnessLabel = makeValueLabel();
    m_saturation = makeSlider(-100, 100);  m_saturationLabel = makeValueLabel();

    auto *settings = new QGridLayout;
    settings->addWidget(new QLabel("Opacité"),    0, 0);
    settings->addWidget(m_opacity,                0, 1);
    settings->addWidget(m_opacityLabel,           0, 2);
    settings->addWidget(new QLabel("Luminosité"), 1, 0);
    settings->addWidget(m_brightness,             1, 1);
    settings->addWidget(m_brightnessLabel,        1, 2);
    settings->addWidget(new QLabel("Saturation"), 2, 0);
    settings->addWidget(m_saturation,             2, 1);
    settings->addWidget(m_saturationLabel,        2, 2);

    m_list = new QListWidget;
    m_list->setIconSize(THUMB_SIZE);
    m_list->setEditTriggers(QAbstractItemView::DoubleClicked);   // double-clic pour renommer

    auto *addBtn  = makeButton("layer-new",       "Nouveau calque");
    auto *dupBtn  = makeButton("layer-duplicate", "Dupliquer le calque");
    auto *upBtn   = makeButton("layer-up",        "Monter le calque");
    auto *downBtn = makeButton("layer-down",      "Descendre le calque");
    auto *delBtn  = makeButton("layer-delete",    "Supprimer le calque");

    auto *buttons = new QHBoxLayout;
    for (auto *b : { addBtn, dupBtn, upBtn, downBtn, delBtn })
        buttons->addWidget(b);
    buttons->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addLayout(settings);
    layout->addWidget(m_list);
    layout->addLayout(buttons);

    connect(addBtn,  &QToolButton::clicked, m_canvas, &Canvas::addLayer);
    connect(dupBtn,  &QToolButton::clicked, m_canvas, &Canvas::duplicateLayer);
    connect(upBtn,   &QToolButton::clicked, m_canvas, &Canvas::moveLayerUp);
    connect(downBtn, &QToolButton::clicked, m_canvas, &Canvas::moveLayerDown);
    connect(delBtn,  &QToolButton::clicked, m_canvas, &Canvas::removeLayer);

    connect(m_list, &QListWidget::currentRowChanged, this, &LayersPanel::onRowChanged);
    connect(m_list, &QListWidget::itemChanged,       this, &LayersPanel::onItemChanged);

    connect(m_opacity, &QSlider::valueChanged, this, [this](int value) {
        m_opacityLabel->setText(QString("%1 %").arg(value));
        if (!m_updating)
            m_canvas->setLayerOpacity(m_canvas->currentLayer(), value / 100.0);
    });
    connect(m_brightness, &QSlider::valueChanged, this, [this](int value) {
        m_brightnessLabel->setText(QString::number(value));
        if (!m_updating)
            m_canvas->setLayerBrightness(m_canvas->currentLayer(), value);
    });
    connect(m_saturation, &QSlider::valueChanged, this, [this](int value) {
        m_saturationLabel->setText(QString::number(value));
        if (!m_updating)
            m_canvas->setLayerSaturation(m_canvas->currentLayer(), value);
    });
    m_opacityLabel->installEventFilter(this);
    m_brightnessLabel->installEventFilter(this);
    m_saturationLabel->installEventFilter(this);

    connect(m_canvas, &Canvas::layersChanged,       this, &LayersPanel::rebuild, Qt::QueuedConnection);
    connect(m_canvas, &Canvas::currentLayerChanged, this, &LayersPanel::selectCurrent);
    connect(m_canvas, &Canvas::layerEdited,         this, &LayersPanel::updateThumbnail);

    rebuild();
}

int LayersPanel::rowToLayer(int row) const   { return m_canvas->layerCount() - 1 - row; }
int LayersPanel::layerToRow(int index) const { return m_canvas->layerCount() - 1 - index; }

void LayersPanel::rebuild()
{
    m_updating = true;
    m_list->clear();
    for (int i = m_canvas->layerCount() - 1; i >= 0; --i) {
        const Layer &layer = m_canvas->layer(i);
        auto *item = new QListWidgetItem(makeThumbnail(m_canvas->renderedLayer(i)), layer.name);
        item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);
        item->setCheckState(layer.visible ? Qt::Checked : Qt::Unchecked);
        item->setToolTip("Case : afficher / masquer\nDouble-clic : renommer");
        item->setSizeHint(QSize(0, THUMB_SIZE.height() + 8));
        m_list->addItem(item);
    }
    m_updating = false;
    selectCurrent();
}

void LayersPanel::selectCurrent()
{
    m_updating = true;
    const int current = m_canvas->currentLayer();
    m_list->setCurrentRow(layerToRow(current));
    const Layer &layer = m_canvas->layer(current);
    m_opacity->setValue(qRound(layer.opacity * 100));
    m_brightness->setValue(layer.brightness);
    m_saturation->setValue(layer.saturation);
    m_opacityLabel->setText(QString("%1 %").arg(m_opacity->value()));
    m_brightnessLabel->setText(QString::number(layer.brightness));
    m_saturationLabel->setText(QString::number(layer.saturation));
    m_updating = false;
}

void LayersPanel::updateThumbnail(int layerIndex)
{
    if (QListWidgetItem *item = m_list->item(layerToRow(layerIndex))) {
        m_updating = true;
        item->setIcon(makeThumbnail(m_canvas->renderedLayer(layerIndex)));
        m_updating = false;
    }
    if (layerIndex == m_canvas->currentLayer())
        selectCurrent();
}

void LayersPanel::onRowChanged(int row)
{
    if (!m_updating && row >= 0)
        m_canvas->setCurrentLayer(rowToLayer(row));
}

void LayersPanel::onItemChanged(QListWidgetItem *item)
{
    if (m_updating)
        return;
    const int index = rowToLayer(m_list->row(item));
    const Layer &layer = m_canvas->layer(index);
    const bool visible = item->checkState() == Qt::Checked;
    const QString name = item->text();

    if (name.trimmed().isEmpty()) {
        QMetaObject::invokeMethod(this, &LayersPanel::rebuild, Qt::QueuedConnection);
        return;
    }

    if (visible != layer.visible)
        m_canvas->setLayerVisible(index, visible);
    else if (name != layer.name)
        m_canvas->renameLayer(index, name);
}

bool LayersPanel::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        if (watched == m_opacityLabel)    { m_opacity->setValue(100);  return true; }
        if (watched == m_brightnessLabel) { m_brightness->setValue(0); return true; }
        if (watched == m_saturationLabel) { m_saturation->setValue(0); return true; }
    }
    return QWidget::eventFilter(watched, event);
}
