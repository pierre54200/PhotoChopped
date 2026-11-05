#pragma once

#include <QDialog>
#include <QSize>
#include <QColor>

class QLineEdit;
class QSpinBox;
class QComboBox;

class NewImageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewImageDialog(QWidget *parent = nullptr);

    // Name
    QString projectName() const;

    // Size
    QSize imageSize() const;

    // Customization
    QColor background() const;

private: 
    // Name
    QLineEdit *m_name;

    // Size
    QSpinBox *m_width;
    QSpinBox *m_height;

    // Background
    QComboBox *m_background;

    // Preset
    QComboBox *m_preset;
    bool m_applyingPreset = false;
    void applyPreset(int index);

    void markCustom();
};
