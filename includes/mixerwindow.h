#ifndef MIXERWINDOW_H
#define MIXERWINDOW_H

#include <QDialog>

namespace Ui {
class ZMixerWindow;
}

class CMixerItem;

class ZMixerWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ZMixerWindow(QWidget *parent = nullptr);
    ~ZMixerWindow();

public Q_SLOTS:
    void reloadControls();

private:
    Ui::ZMixerWindow *ui;
    QVector<QVector<CMixerItem> > m_controls;

    void addMixerItemWidget(QLayout* layout, QWidget* itemWidget);

private Q_SLOTS:
    void volumeChanged(int value);
    void switchClicked(bool state);
};

#endif // MIXERWINDOW_H
