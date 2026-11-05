#pragma once

#include <QMainWindow>
#include <QString>

#include "canvas.h"
#include "brush.h"
#include "eraser.h"
#include "pencil.h"
#include "bucket.h"
#include "pipette.h"
#include "shape.h"

class QPushButton;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    // Bouton
    void newImage();
    void openImage();
    void save();
    void saveAs();
    void exportImage();
    bool saveProjectTo(const QString &path);
    bool exportTo(const QString &path);

    // Titre
    void updateTitle();

    // Pour Open et Save
    QString defaultFolder() const;

    // Palette
    void chooseColor();
    void updateColorButton();

    // Affichage
    Ui::MainWindow  *ui;

    // L'endroit ou dessiner
    Canvas          *m_canvas;

    // Tool
    Brush           m_brush;
    Eraser          m_eraser;
    Pencil          m_pencil;
    Bucket          m_bucket;
    Pipette         m_pipette;
    Shape           m_shape;

    QPushButton     *m_colorButton;

    // Name
    QString         m_projectName = "Sans titre";

    // Path
    QString         m_filePath;
    QString         m_exportPath;
};
