#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "newimagedialog.h"
#include "layerspanel.h"
#include <QDockWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_canvas = new Canvas(this);
    setCentralWidget(m_canvas);
    m_canvas->setTool(&m_brush);

    ui->actionUndo->setShortcut(QKeySequence::Undo);    // Ctrl+Z
    ui->actionRedo->setShortcut(QKeySequence("Ctrl+Shift+Z"));

    // les boutons sont grisés quand il n'y a rien à annuler / rétablir
    auto updateHistoryActions = [this] {
        ui->actionUndo->setEnabled(m_canvas->canUndo());
        ui->actionRedo->setEnabled(m_canvas->canRedo());
    };
    connect(m_canvas, &Canvas::historyChanged, this, updateHistoryActions);
    updateHistoryActions();

    auto *layersDock = new QDockWidget("Calques", this);
    layersDock->setWidget(new LayersPanel(m_canvas));
    addDockWidget(Qt::RightDockWidgetArea, layersDock);

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
    if (!color.isValid())
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


void MainWindow::on_actionNew_triggered()
{
    NewImageDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    m_canvas->newImage(dialog.imageSize(), dialog.background());
    m_projectName = dialog.projectName();
    m_filePath.clear();
    updateTitle();
}


bool MainWindow::saveTo(const QString &path)
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
        QMessageBox::warning(this, "Erreur",
                             "Impossible d'enregistrer l'image :\n" + path);
        return false;
    }

    m_filePath = path;
    m_projectName = QFileInfo(path).completeBaseName();
    updateTitle();
    statusBar()->showMessage("Image enregistrée : " + path, 3000);
    return true;
}

void MainWindow::updateTitle()
{
    setWindowTitle(m_projectName + " - PhotoChopped");
}


void MainWindow::on_actionSave_triggered()
{
    if (m_filePath.isEmpty())
        on_actionSave_as_triggered();
    else
        saveTo(m_filePath);
}


void MainWindow::on_actionSave_as_triggered()
{
    QString startPath = m_filePath;
    if (startPath.isEmpty()) {
        QDir pictures(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
        startPath = pictures.filePath(m_projectName + ".png");
    }

    QString selectedFilter;
    QString path = QFileDialog::getSaveFileName(
        this, "Enregistrer l'image", startPath,
        "Image PNG (*.png);;Image JPEG (*.jpg *.jpeg);;Image BMP (*.bmp)",
        &selectedFilter);

    if (path.isEmpty())
        return;

    if (QFileInfo(path).suffix().isEmpty()) {
        if (selectedFilter.contains("JPEG"))     path += ".jpg";
        else if (selectedFilter.contains("BMP")) path += ".bmp";
        else                                     path += ".png";
    }

    saveTo(path);
}


void MainWindow::on_actionOpen_triggered()
{
    QString startDir = m_filePath.isEmpty()
                           ? QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                           : QFileInfo(m_filePath).absolutePath();

    QString path = QFileDialog::getOpenFileName(
        this, "Ouvrir une image", startDir,
        "Images (*.png *.jpg *.jpeg *.bmp *.gif *.webp);;Tous les fichiers (*)");

    if (path.isEmpty())
        return;

    QImageReader reader(path);
    reader.setAutoTransform(true);
    QImage image = reader.read();

    if (image.isNull()) {
        QMessageBox::warning(this, "Erreur",
                             "Impossible d'ouvrir l'image :\n" + path +
                                 "\n\n" + reader.errorString());
        return;
    }

    m_canvas->setImage(image);
    m_filePath = path;
    m_projectName = QFileInfo(path).completeBaseName();
    updateTitle();
    statusBar()->showMessage(QString("Image ouverte : %1 × %2 px")
                                 .arg(image.width()).arg(image.height()), 3000);
}


void MainWindow::on_actionQuit_triggered()
{
    close();
}


void MainWindow::on_actionPencil_triggered()
{
    m_canvas->setTool(&m_pencil);
}


void MainWindow::on_actionBucket_triggered()
{
    m_canvas->setTool(&m_bucket);
}


void MainWindow::on_actionUndo_triggered()
{
    m_canvas->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    m_canvas->redo();
}
