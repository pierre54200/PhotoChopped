#pragma once

#include <QImage>
#include <QString>
#include <QVector>

struct Layer
{
    QString name;
    QImage  image;
    bool    visible = true;

    double  opacity = 1.0;
    int     brightness = 0;
    int     saturation = 0;

    QVector<QImage> undoStack;
    QVector<QImage> redoStack;

    mutable QImage cache;
    mutable qint64 cacheSourceKey = 0;
    mutable int    cacheBrightness = 0;
    mutable int    cacheSaturation = 0;
};
