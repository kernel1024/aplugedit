#include "includes/softvoldialog.h"
#include "ui_softvoldialog.h"

ZSoftvolDialog::ZSoftvolDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ZSoftvolDialog)
{
    ui->setupUi(this);

    connect(ui->spinMinDB,qOverload<double>(&QDoubleSpinBox::valueChanged),this,&ZSoftvolDialog::updateMaxDBLimits);
    connect(ui->checkMuteSwitch,&QCheckBox::toggled,this,&ZSoftvolDialog::updateResolution);
}

ZSoftvolDialog::~ZSoftvolDialog()
{
    delete ui;
}

void ZSoftvolDialog::setParams(const QString &name, double min_dB, double max_dB, int resolution, int channels)
{
    ui->editName->setText(name);

    ui->spinMinDB->setValue(min_dB);
    ui->spinMaxDB->setValue(max_dB);
    updateMaxDBLimits(min_dB);

    if (resolution>1) {
        ui->spinResolution->setValue(resolution);
        ui->checkMuteSwitch->setChecked(false);
        updateResolution(false);
    } else {
        ui->spinResolution->setValue(2);
        ui->checkMuteSwitch->setChecked(true);
        updateResolution(true);
    }

    ui->spinChannels->setValue(channels);
}

void ZSoftvolDialog::getParams(QString &name, double &min_dB, double &max_dB, int &resolution, int &channels)
{
    name = ui->editName->text();
    min_dB = ui->spinMinDB->value();
    max_dB = ui->spinMaxDB->value();
    if (ui->checkMuteSwitch->isChecked()) {
        resolution = 1;
    } else {
        resolution = ui->spinResolution->value();
    }
    channels = ui->spinChannels->value();
}

void ZSoftvolDialog::updateMaxDBLimits(double value)
{
    ui->spinMaxDB->setMinimum(value + 0.01);
}

void ZSoftvolDialog::updateResolution(bool state)
{
    ui->spinResolution->setEnabled(!state);
}
