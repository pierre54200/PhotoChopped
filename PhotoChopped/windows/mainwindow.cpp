#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSpinBox>

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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionBrush_triggered()
{
    m_canvas->setTool(&m_brush);
}

void MainWindow::on_actionEraser_triggered()
{
    m_canvas->setTool(&m_eraser);
}



