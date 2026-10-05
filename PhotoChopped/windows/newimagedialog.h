#ifndef NEWIMAGEDIALOG_H
#define NEWIMAGEDIALOG_H

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

    QString projectName() const;
    QSize   imageSize() const;
    QColor  background() const;

private:
    void applyPreset(int index);
    void markCustom();

    QLineEdit *m_name;
    QComboBox *m_preset;
    QSpinBox  *m_width;
    QSpinBox  *m_height;
    QComboBox *m_background;
    bool       m_applyingPreset = false;
};

#endif // NEWIMAGEDIALOG_H
