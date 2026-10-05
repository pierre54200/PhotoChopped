#ifndef TOOL_H
#define TOOL_H

#include <QPoint>
#include <QCursor>

class Canvas;
class QPainter;

// Classe de base de tous les outils.
// Pour ajouter un outil : cree une classe qui herite de Tool
// et redefinis les fonctions dont tu as besoin.
class Tool
{
public:
    virtual ~Tool() = default;

    // appelees par le Canvas quand la souris agit
    virtual void press(Canvas &canvas, const QPoint &pos) = 0;
    virtual void move(Canvas &canvas, const QPoint &pos) = 0;
    virtual void release(Canvas &canvas, const QPoint &pos) { Q_UNUSED(canvas); Q_UNUSED(pos); }

    // curseur affiche quand l'outil est actif
    virtual QCursor cursor() const { return Qt::CrossCursor; }

    // dessin par-dessus l'image, sans la modifier (ex : cercle de la gomme)
    virtual void drawOverlay(QPainter &painter, const Canvas &canvas, const QPoint &cursorPos) const
    {
        Q_UNUSED(painter); Q_UNUSED(canvas); Q_UNUSED(cursorPos);
    }
};

#endif // TOOL_H
