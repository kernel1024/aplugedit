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
    ~ZMixerWindow() override;

public Q_SLOTS:
    void reloadControls(int cardNum);
    void updateControlsState(int cardNum);

private:
    Ui::ZMixerWindow *ui;
    QVector<QVector<CMixerItem> > m_controls;

    void addSeparatedWidgetToLayout(QLayout* layout, QWidget* itemWidget);
    bool getMixerItemIDs(QWidget *widget, int *card, unsigned int *numid);
    void reloadControlsQueued(int cardNum);

protected:
    bool event(QEvent* event) override;

private Q_SLOTS:
    void reloadAllCards();
    void volumeChanged(int value);
    void switchClicked(bool state);
    void switchListClicked(QListWidgetItem* item);
    void enumClicked(int index);
    void mixerCtxMenuClicked();
};

#endif // MIXERWINDOW_H
