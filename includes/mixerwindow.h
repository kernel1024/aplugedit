#ifndef MIXERWINDOW_H
#define MIXERWINDOW_H

#include <QtWidgets>

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
    void reloadControls(int cardNum);
    void updateControlsState(int cardNum);

private:
    Ui::ZMixerWindow *ui;
    QVector<QVector<CMixerItem> > m_controls;

    void addSeparatedWidgetToLayout(QLayout* layout, QWidget* itemWidget);
    bool getMixerItemIDs(QWidget *widget, int *card, unsigned int *numid);
    void clearTab(int cardNum);
    void clearLayout(QLayout *layout);

private Q_SLOTS:
    void reloadAllCards();
    void volumeChanged(int value);
    void switchClicked(bool state);
    void switchListClicked(QListWidgetItem* item);
    void enumClicked(int index);
    void deleteClicked();
};

#endif // MIXERWINDOW_H
