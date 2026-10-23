#pragma once

#include <QString>

class Canvas;

namespace ProjectFile
{
inline const QString Extension = QStringLiteral("pcp");

bool save(const Canvas &canvas, const QString &path, QString *error = nullptr);
bool load(Canvas &canvas, const QString &path, QString *error = nullptr);
}
