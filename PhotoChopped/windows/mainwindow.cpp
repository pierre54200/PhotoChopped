#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_canvas = new Canvas(this);
    setCentralWidget(m_canvas);
    m_canvas->setTool(&m_brush);


    ui->toolBar->addSeparator();

    auto *sizeBox = new QSpinBox;
    sizeBox->setRange(1, 200);
    sizeBox->setValue(m_canvas->brushSize());
    sizeBox->setSuffix(" px");
    sizeBox->setToolTip("Taille de l'outil");
    ui->toolBar->addWidget(sizeBox);

    connect(sizeBox, &QSpinBox::valueChanged, m_canvas, &Canvas::setBrushSize);

    m_colorButton = new QPushButton;
    m_colorButton->setFixedSize(28, 28);
    m_colorButton->setToolTip("Couleur");
    ui->toolBar->addWidget(m_colorButton);

    connect(m_colorButton, &QPushButton::clicked, this, &MainWindow::chooseColor);
    updateColorButton();

    connect(m_canvas, &Canvas::colorChanged, this, &MainWindow::updateColorButton);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::chooseColor()
{
    QColor color = QColorDialog::getColor(m_canvas->color(), this, "Choisir une couleur");
    if (!color.isValid())        // l'utilisateur a cliqué sur Annuler
        return;

    m_canvas->setColor(color);
    updateColorButton();
}

void MainWindow::updateColorButton()
{
    m_colorButton->setStyleSheet(
        QString("background-color: %1; border: 1px solid #555; border-radius: 3px;")
            .arg(m_canvas->color().name()));
}

void MainWindow::on_actionBrush_triggered()
{
    m_canvas->setTool(&m_brush);
}

void MainWindow::on_actionEraser_triggered()
{
    m_canvas->setTool(&m_eraser);
}

void MainWindow::on_actionPipette_triggered()
{
    m_canvas->setTool(&m_pipette);
}

