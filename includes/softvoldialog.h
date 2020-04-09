#ifndef SOFTVOLDIALOG_H
#define SOFTVOLDIALOG_H

#include <QDialog>

namespace Ui {
class ZSoftvolDialog;
}

class ZSoftvolDialog : public QDialog
{
    Q_OBJECT

private:
    Ui::ZSoftvolDialog *ui;

public:
    explicit ZSoftvolDialog(QWidget *parent = nullptr);
    ~ZSoftvolDialog();

    void setParams(const QString& name, double min_dB, double max_dB, int resolution, int channels);
    void getParams(QString& name, double& min_dB, double& max_dB, int& resolution, int& channels);

private Q_SLOTS:
    void updateMaxDBLimits(double value);
    void updateResolution(bool state);
};

#endif // SOFTVOLDIALOG_H
