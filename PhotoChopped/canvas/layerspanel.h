#pragma once

#include <QWidget>

class Canvas;
class QListWidget;
class QListWidgetItem;
class QSlider;
class QLabel;

class LayersPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LayersPanel(Canvas *canvas, QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void rebuild();
    void selectCurrent();
    void updateThumbnail(int layerIndex);
    void onItemChanged(QListWidgetItem *item);
    void onRowChanged(int row);

    int rowToLayer(int row) const;
    int layerToRow(int index) const;

    Canvas      *m_canvas;
    QListWidget *m_list;
    QSlider     *m_opacity;
    QLabel      *m_opacityLabel;
    QSlider     *m_brightness;
    QLabel      *m_brightnessLabel;
    QSlider     *m_saturation;
    QLabel      *m_saturationLabel;
    bool         m_updating = false;
};
