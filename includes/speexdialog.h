#ifndef SPEEXDIALOG_H
#define SPEEXDIALOG_H

#include <QDialog>

namespace Ui {
class ZSpeexDialog;
}

class ZSpeexDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ZSpeexDialog(QWidget *parent = nullptr);
    ~ZSpeexDialog() override;

    void setParams(Qt::CheckState denoise, Qt::CheckState agc, Qt::CheckState dereverb,
                   Qt::CheckState echo, int frames, int agcLevel, int filterLength);
    void getParams(Qt::CheckState &denoise, Qt::CheckState &agc, Qt::CheckState &dereverb,
                   Qt::CheckState &echo, int &frames, int &agcLevel, int &filterLength);

private Q_SLOTS:
    void updateState(int state);

private:
    Ui::ZSpeexDialog *ui;
};

#endif // SPEEXDIALOG_H
