#pragma once

#include <QImage>
#include <QString>
#include <QVector>

struct Layer
{
    // Image
    QString name;
    QImage  image;
    bool    visible = true;

    // Option
    double  opacity = 1.0;
    int     brightness = 0;
    int     saturation = 0;

    // Historique
    QVector<QImage> undoStack;
    QVector<QImage> redoStack;

    // Cache
    mutable QImage cache;
    mutable qint64 cacheSourceKey = 0;
    mutable int    cacheBrightness = 0;
    mutable int    cacheSaturation = 0;
};
