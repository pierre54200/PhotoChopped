#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "newimagedialog.h"
#include "layerspanel.h"
#include "projectfile.h"

#include <QAction>
#include <QActionGroup>
#include <QColorDialog>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QStandardPaths>
#include <QStatusBar>
#include <QComboBox>
#include <QCheckBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto *actionExport = new QAction(QIcon(":/icons/dark/file-export.svg"), "Exporter...", this);
    for (QObject *obj : ui->actionSave_as->associatedObjects()) {
        if (auto *menu = qobject_cast<QMenu *>(obj)) {
            if (menu->actions().contains(ui->actionQuit))
                menu->insertAction(ui->actionQuit, actionExport);
            else
                menu->addAction(actionExport);
        }
    }

    // Fichier
    connect(ui->actionNew,     &QAction::triggered, this, &MainWindow::newImage);
    connect(ui->actionOpen,    &QAction::triggered, this, &MainWindow::openImage);
    connect(ui->actionSave,    &QAction::triggered, this, &MainWindow::save);
    connect(ui->actionSave_as, &QAction::triggered, this, &MainWindow::saveAs);
    connect(actionExport,      &QAction::triggered, this, &MainWindow::exportImage);
    connect(ui->actionQuit,    &QAction::triggered, this, &MainWindow::close);

    ui->actionNew->setShortcut(QKeySequence::New);
    ui->actionOpen->setShortcut(QKeySequence::Open);
    ui->actionSave->setShortcut(QKeySequence::Save);
    ui->actionSave_as->setShortcut(QKeySequence("Ctrl+Shift+S"));
    actionExport->setShortcut(QKeySequence("Ctrl+E"));
    ui->actionQuit->setShortcut(QKeySequence("Ctrl+Q"));


    m_canvas = new Canvas(this);
    setCentralWidget(m_canvas);
    m_canvas->setTool(&m_brush);
    m_canvas->setFocus();

    auto *tools = new QActionGroup(this);
    for (QAction *a : { ui->actionBrush, ui->actionEraser, ui->actionPencil,
                       ui->actionBucket, ui->actionPipette }) {
        a->setCheckable(true);
        tools->addAction(a);
    }
    ui->actionBrush->setChecked(true);

    connect(ui->actionBrush,   &QAction::triggered, this, [this] { m_canvas->setTool(&m_brush); });
    connect(ui->actionEraser,  &QAction::triggered, this, [this] { m_canvas->setTool(&m_eraser); });
    connect(ui->actionPencil,  &QAction::triggered, this, [this] { m_canvas->setTool(&m_pencil); });
    connect(ui->actionBucket,  &QAction::triggered, this, [this] { m_canvas->setTool(&m_bucket); });
    connect(ui->actionPipette, &QAction::triggered, this, [this] { m_canvas->setTool(&m_pipette); });

    // Forme
    auto *actionShape = new QAction(QIcon(":/icons/dark/tool-shape.svg"), "Forme", this);
    actionShape->setCheckable(true);
    actionShape->setToolTip("Forme (Maj : carré / cercle, Ctrl : depuis le centre)");
    tools->addAction(actionShape);            // le même groupe que les autres outils
    ui->toolBar->addAction(actionShape);
    connect(actionShape, &QAction::triggered, this, [this] { m_canvas->setTool(&m_shape); });

    // options de la forme, dans la barre d'outils
    auto *shapeType = new QComboBox;
    shapeType->addItem("Rectangle");
    shapeType->addItem("Ellipse");
    shapeType->setToolTip("Type de forme");

    auto *shapeFilled = new QCheckBox("Rempli");
    shapeFilled->setToolTip("Coché : forme pleine ; décoché : seulement le contour");

    connect(shapeType, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_shape.setShape(index == 0 ? Shape::ShapeType::Rectangle : Shape::ShapeType::Cercle);
    });
    connect(shapeFilled, &QCheckBox::toggled, this, [this](bool on) { m_shape.setFilled(on); });

    // Historique
    connect(ui->actionUndo, &QAction::triggered, m_canvas, &Canvas::undo);
    connect(ui->actionRedo, &QAction::triggered, m_canvas, &Canvas::redo);
    ui->actionUndo->setShortcut(QKeySequence::Undo);
    ui->actionRedo->setShortcut(QKeySequence("Ctrl+Shift+Z"));
    ui->toolBar->addWidget(shapeType);
    ui->toolBar->addWidget(shapeFilled);

    auto updateHistoryActions = [this] {
        ui->actionUndo->setEnabled(m_canvas->canUndo());
        ui->actionRedo->setEnabled(m_canvas->canRedo());
    };
    connect(m_canvas, &Canvas::historyChanged, this, updateHistoryActions);
    updateHistoryActions();

    // Layers
    auto *layersDock = new QDockWidget("Calques", this);
    layersDock->setObjectName("layersDock");
    layersDock->setWidget(new LayersPanel(m_canvas));
    addDockWidget(Qt::RightDockWidgetArea, layersDock);

    QMenu *windowsMenu = menuBar()->addMenu("Fenêtres");
    windowsMenu->addAction(layersDock->toggleViewAction());
    layersDock->toggleViewAction()->setShortcut(QKeySequence("Ctrl+L"));

    // Size
    auto *sizeBox = new QSpinBox;
    sizeBox->setRange(1, 200);
    sizeBox->setValue(m_canvas->brushSize());
    sizeBox->setSuffix(" px");
    sizeBox->setToolTip("Taille de l'outil");
    ui->toolBar->addWidget(sizeBox);
    connect(sizeBox, &QSpinBox::valueChanged, m_canvas, &Canvas::setBrushSize);

    // Color
    m_colorButton = new QPushButton;
    m_colorButton->setFixedSize(28, 28);
    m_colorButton->setToolTip("Couleur");
    ui->toolBar->addWidget(m_colorButton);
    connect(m_colorButton, &QPushButton::clicked, this, &MainWindow::chooseColor);
    connect(m_canvas, &Canvas::colorChanged, this, &MainWindow::updateColorButton);
    updateColorButton();

    // Titre
    updateTitle();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::chooseColor()
{
    const QColor color = QColorDialog::getColor(m_canvas->color(), this, "Choisir une couleur");
    if (!color.isValid())
        return;
    m_canvas->setColor(color);
}

void MainWindow::updateColorButton()
{
    m_colorButton->setStyleSheet(
        QString("background-color: %1; border: 1px solid #555; border-radius: 3px;")
            .arg(m_canvas->color().name()));
}

void MainWindow::updateTitle()
{
    setWindowTitle(m_projectName + " - PhotoChopped");
}

QString MainWindow::defaultFolder() const
{
    if (!m_filePath.isEmpty())
        return QFileInfo(m_filePath).absolutePath();
    return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
}

void MainWindow::newImage()
{
    NewImageDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    m_canvas->newImage(dialog.imageSize(), dialog.background());
    m_projectName = dialog.projectName();
    m_filePath.clear();
    m_exportPath.clear();
    updateTitle();
}

void MainWindow::openImage()
{
    const QString path = QFileDialog::getOpenFileName(
        this, "Ouvrir", defaultFolder(),
        "Projets et images (*.pcp *.png *.jpg *.jpeg *.bmp *.gif *.webp);;"
        "Projets PhotoChopped (*.pcp);;"
        "Images (*.png *.jpg *.jpeg *.bmp *.gif *.webp)");
    if (path.isEmpty())
        return;

    if (QFileInfo(path).suffix().toLower() == ProjectFile::Extension) {
        QString error;
        if (!ProjectFile::load(*m_canvas, path, &error)) {
            QMessageBox::warning(this, "Erreur",
                                 "Impossible d'ouvrir le projet :\n" + path + "\n\n" + error);
            return;
        }
        m_filePath = path;
        m_exportPath.clear();
    } else {
        QImageReader reader(path);
        reader.setAutoTransform(true);
        const QImage image = reader.read();
        if (image.isNull()) {
            QMessageBox::warning(this, "Erreur",
                                 "Impossible d'ouvrir l'image :\n" + path + "\n\n" + reader.errorString());
            return;
        }
        m_canvas->setImage(image);
        m_filePath.clear();
        m_exportPath = path;
    }

    m_projectName = QFileInfo(path).completeBaseName();
    updateTitle();
    statusBar()->showMessage("Ouvert : " + path, 3000);
}

void MainWindow::save()
{
    if (m_filePath.isEmpty())
        saveAs();
    else
        saveProjectTo(m_filePath);
}

void MainWindow::saveAs()
{
    const QString start = m_filePath.isEmpty()
    ? QDir(defaultFolder()).filePath(m_projectName + "." + ProjectFile::Extension)
    : m_filePath;

    QString path = QFileDialog::getSaveFileName(
        this, "Enregistrer le projet", start, "Projet PhotoChopped (*.pcp)");
    if (path.isEmpty())
        return;
    if (QFileInfo(path).suffix().toLower() != ProjectFile::Extension)
        path += "." + ProjectFile::Extension;

    saveProjectTo(path);
}

bool MainWindow::saveProjectTo(const QString &path)
{
    QString error;
    if (!ProjectFile::save(*m_canvas, path, &error)) {
        QMessageBox::warning(this, "Erreur",
                             "Impossible d'enregistrer le projet :\n" + path + "\n\n" + error);
        return false;
    }
    m_filePath = path;
    m_projectName = QFileInfo(path).completeBaseName();
    updateTitle();
    statusBar()->showMessage("Projet enregistré : " + path, 3000);
    return true;
}

void MainWindow::exportImage()
{
    const QString start = m_exportPath.isEmpty()
    ? QDir(defaultFolder()).filePath(m_projectName + ".png")
    : m_exportPath;

    QString selectedFilter;
    QString path = QFileDialog::getSaveFileName(
        this, "Exporter l'image", start,
        "Image PNG (*.png);;Image JPEG (*.jpg *.jpeg);;Image BMP (*.bmp)", &selectedFilter);
    if (path.isEmpty())
        return;

    if (QFileInfo(path).suffix().isEmpty()) {
        if (selectedFilter.contains("JPEG"))     path += ".jpg";
        else if (selectedFilter.contains("BMP")) path += ".bmp";
        else                                     path += ".png";
    }
    exportTo(path);
}

bool MainWindow::exportTo(const QString &path)
{
    QImage image = m_canvas->flattenedImage();

    const QString ext = QFileInfo(path).suffix().toLower();
    if (ext == "jpg" || ext == "jpeg" || ext == "bmp") {
        QImage flat(image.size(), QImage::Format_RGB32);
        flat.fill(Qt::white);
        QPainter painter(&flat);
        painter.drawImage(0, 0, image);
        painter.end();
        image = flat;
    }

    if (!image.save(path)) {
        QMessageBox::warning(this, "Erreur", "Impossible d'exporter l'image :\n" + path);
        return false;
    }
    m_exportPath = path;
    statusBar()->showMessage("Image exportée : " + path, 3000);
    return true;
}