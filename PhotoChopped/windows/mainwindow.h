#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSpinBox>
#include <QPushButton>
#include <QColorDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QStatusBar>
#include <QPainter>
#include <QImageReader>

#include "canvas.h"
#include "tools/eraser.h"
#include "tools/brush.h"
#include "tools/pipette.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_actionEraser_triggered();

    void on_actionBrush_triggered();

    void on_actionPipette_triggered();

    void on_actionNew_triggered();

    void on_actionSave_triggered();

    void on_actionSave_as_triggered();

    void on_actionOpen_triggered();

    void on_actionQuit_triggered();

private:
    Ui::MainWindow *ui;
    Canvas *m_canvas;
    Brush m_brush;
    Eraser m_eraser;
    Pipette m_pipette;

    QPushButton *m_colorButton;
    void chooseColor();
    void updateColorButton();

    QString m_projectName = "Sans titre";
    QString m_filePath;

    bool saveTo(const QString &path);
    void updateTitle();
};
#endif // MAINWINDOW_H
