#include "newimagedialog.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>

// tailles proposees (la premiere entree "Personnalisee" n'a pas de taille)
struct Preset { const char *label; int w; int h; };
static const Preset PRESETS[] = {
    { "Personnalisée",            0,    0 },
    { "800 × 600",              800,  600 },
    { "HD (1280 × 720)",       1280,  720 },
    { "Full HD (1920 × 1080)", 1920, 1080 },
    { "Carré (1080 × 1080)",   1080, 1080 },
    { "A4 300 dpi (2480 × 3508)", 2480, 3508 },
    };

NewImageDialog::NewImageDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Nouvelle image");

    m_name = new QLineEdit("Sans titre");
    m_name->selectAll();

    m_preset = new QComboBox;
    for (const Preset &p : PRESETS)
        m_preset->addItem(p.label);

    m_width = new QSpinBox;
    m_width->setRange(1, 10000);
    m_width->setSuffix(" px");

    m_height = new QSpinBox;
    m_height->setRange(1, 10000);
    m_height->setSuffix(" px");

    auto *sizeRow = new QHBoxLayout;
    sizeRow->addWidget(m_width);
    sizeRow->addWidget(new QLabel("×"));
    sizeRow->addWidget(m_height);

    m_background = new QComboBox;
    m_background->addItem("Blanc");
    m_background->addItem("Noir");
    m_background->addItem("Transparent");

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText("Créer");
    buttons->button(QDialogButtonBox::Cancel)->setText("Annuler");
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *form = new QFormLayout(this);
    form->addRow("Nom du projet :", m_name);
    form->addRow("Modèle :", m_preset);
    form->addRow("Taille :", sizeRow);
    form->addRow("Fond :", m_background);
    form->addRow(buttons);

    // choisir un modele remplit la taille ; modifier la taille repasse en "Personnalisee"
    connect(m_preset, &QComboBox::currentIndexChanged, this, &NewImageDialog::applyPreset);
    connect(m_width,  &QSpinBox::valueChanged, this, &NewImageDialog::markCustom);
    connect(m_height, &QSpinBox::valueChanged, this, &NewImageDialog::markCustom);

    m_preset->setCurrentIndex(1);              // 800 x 600 par defaut
}

void NewImageDialog::applyPreset(int index)
{
    if (index <= 0)
        return;
    m_applyingPreset = true;
    m_width->setValue(PRESETS[index].w);
    m_height->setValue(PRESETS[index].h);
    m_applyingPreset = false;
}

void NewImageDialog::markCustom()
{
    if (!m_applyingPreset)
        m_preset->setCurrentIndex(0);
}

QString NewImageDialog::projectName() const
{
    const QString name = m_name->text().trimmed();
    return name.isEmpty() ? QStringLiteral("Sans titre") : name;
}

QSize NewImageDialog::imageSize() const
{
    return QSize(m_width->value(), m_height->value());
}

QColor NewImageDialog::background() const
{
    switch (m_background->currentIndex()) {
    case 1:  return Qt::black;
    case 2:  return Qt::transparent;
    default: return Qt::white;
    }
}