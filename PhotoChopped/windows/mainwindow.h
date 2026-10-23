#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

#include "canvas.h"
#include "brush.h"
#include "eraser.h"
#include "pencil.h"
#include "bucket.h"
#include "pipette.h"

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
    void newImage();
    void openImage();
    void save();
    void saveAs();
    void exportImage();
    bool saveProjectTo(const QString &path);
    bool exportTo(const QString &path);
    void updateTitle();
    QString defaultFolder() const;

    void chooseColor();
    void updateColorButton();

    Ui::MainWindow *ui;
    Canvas *m_canvas;

    Brush   m_brush;
    Eraser  m_eraser;
    Pencil  m_pencil;
    Bucket  m_bucket;
    Pipette m_pipette;

    QPushButton *m_colorButton;

    QString m_projectName = "Sans titre";
    QString m_filePath;
    QString m_exportPath;
};

#endif // MAINWINDOW_H