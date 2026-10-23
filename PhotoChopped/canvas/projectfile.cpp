#include "projectfile.h"
#include "canvas.h"

#include <QDataStream>
#include <QFile>
#include <QSaveFile>

static constexpr char    MAGIC[4] = { 'P', 'C', 'H', 'P' };
static constexpr quint32 VERSION  = 1;

static void setError(QString *error, const QString &message)
{
    if (error)
        *error = message;
}

bool ProjectFile::save(const Canvas &canvas, const QString &path, QString *error)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        setError(error, file.errorString());
        return false;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_0);

    out.writeRawData(MAGIC, 4);
    out << VERSION;
    out << qint32(canvas.imageSize().width()) << qint32(canvas.imageSize().height());
    out << qint32(canvas.currentLayer()) << qint32(canvas.layerCounter());
    out << qint32(canvas.layerCount());

    for (int i = 0; i < canvas.layerCount(); ++i) {
        const Layer &layer = canvas.layer(i);
        out << layer.name
            << layer.visible
            << layer.opacity
            << qint32(layer.brightness)
            << qint32(layer.saturation)
            << layer.image;
    }

    if (out.status() != QDataStream::Ok) {
        file.cancelWriting();
        setError(error, QStringLiteral("Erreur pendant l'écriture des données."));
        return false;
    }
    if (!file.commit()) {
        setError(error, file.errorString());
        return false;
    }
    return true;
}

bool ProjectFile::load(Canvas &canvas, const QString &path, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(error, file.errorString());
        return false;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0);

    char magic[4];
    if (in.readRawData(magic, 4) != 4 || memcmp(magic, MAGIC, 4) != 0) {
        setError(error, QStringLiteral("Ce fichier n'est pas un projet PhotoChopped."));
        return false;
    }

    quint32 version = 0;
    in >> version;
    if (version > VERSION) {
        setError(error, QStringLiteral("Ce projet a été créé avec une version plus récente de PhotoChopped."));
        return false;
    }

    qint32 width = 0, height = 0, current = 0, counter = 1, count = 0;
    in >> width >> height >> current >> counter >> count;
    if (in.status() != QDataStream::Ok || width <= 0 || height <= 0 || count <= 0 || count > 1000) {
        setError(error, QStringLiteral("Le fichier est abîmé (en-tête invalide)."));
        return false;
    }

    QVector<Layer> layers;
    layers.reserve(count);
    for (int i = 0; i < count; ++i) {
        Layer layer;
        qint32 brightness = 0, saturation = 0;
        in >> layer.name >> layer.visible >> layer.opacity >> brightness >> saturation >> layer.image;

        if (in.status() != QDataStream::Ok || layer.image.isNull()
            || layer.image.size() != QSize(width, height)) {
            setError(error, QString("Le fichier est abîmé (calque %1).").arg(i + 1));
            return false;
        }
        layer.brightness = brightness;
        layer.saturation = saturation;
        layer.image = layer.image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        layers.append(layer);
    }

    canvas.setLayers(layers, current, counter);
    return true;
}
