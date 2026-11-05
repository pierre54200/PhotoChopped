#pragma once

#include "tool.h"
#include <QImage>
#include <QRect>

// Outil Sélection (rectangle) :
//   tirer        -> sélectionne une zone du calque actif
//   tirer dedans -> déplace les pixels sélectionnés
//   Échap        -> enlève la sélection
// Un déplacement s'annule avec Ctrl+Z comme un coup de pinceau.
class Select : public Tool
{
public:
    void press(Canvas &canvas, const QPoint &pos) override;
    void move(Canvas &canvas, const QPoint &pos) override;
    void release(Canvas &canvas, const QPoint &pos) override;
    bool keyPress(Canvas &canvas, QKeyEvent *event) override;

    void drawOverlay(QPainter &painter, const Canvas &canvas, const QPoint &cursorPos) const override;
    bool overlayFollowsCursor() const override { return false; }   // la sélection reste visible

    bool hasSelection() const { return m_hasRect; }
    QRect selection() const { return m_rect; }
    void clear();

private:
    enum class Mode { None, Creating, Moving };

    void cut(Canvas &canvas);        // enlève les pixels du calque et les garde dans m_floating

    Mode   m_mode = Mode::None;
    bool   m_hasRect = false;
    QRect  m_rect;                   // la sélection, en pixels de l'image
    QPoint m_start;                  // début du rectangle quand on sélectionne
    QPoint m_grab;                   // point attrapé quand on déplace
    QRect  m_rectAtGrab;             // la sélection au début du déplacement

    bool   m_cut = false;            // les pixels ont-ils déjà été enlevés du calque ?
    QImage m_floating;               // les pixels en cours de déplacement
};