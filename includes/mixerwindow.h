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
    void reloadControls(const QString& ctlName);
    void updateControlsState(const QString& ctlName);
    void reloadAllCards();

private:
    Ui::ZMixerWindow *ui;
    QHash<QString, QVector<CMixerItem> > m_controls;

    void addSeparatedWidgetToLayout(QLayout* layout, QWidget* itemWidget);
    void reloadControlsQueued(const QString &ctlName);

protected:
    bool event(QEvent* event) override;

private Q_SLOTS:
    void volumeChanged(int value);
    void switchClicked(bool state);
    void switchListClicked(QListWidgetItem* item);
    void enumClicked(int index);
    void mixerCtxMenuClicked();
};

#endif // MIXERWINDOW_H
