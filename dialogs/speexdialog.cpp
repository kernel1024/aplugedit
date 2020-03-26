#include "includes/speexdialog.h"
#include "includes/cpspeex.h"
#include "ui_speexdialog.h"

ZSpeexDialog::ZSpeexDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ZSpeexDialog)
{
    ui->setupUi(this);

    connect(ui->checkFrames,&QCheckBox::stateChanged,this,&ZSpeexDialog::updateState);
    connect(ui->checkAGCLevel,&QCheckBox::stateChanged,this,&ZSpeexDialog::updateState);
    connect(ui->checkFilterLength,&QCheckBox::stateChanged,this,&ZSpeexDialog::updateState);
}

ZSpeexDialog::~ZSpeexDialog()
{
    delete ui;
}

void ZSpeexDialog::setParams(Qt::CheckState denoise, Qt::CheckState agc, Qt::CheckState dereverb,
                             Qt::CheckState echo, int frames, int agcLevel, int filterLength)
{
    ui->checkDenoise->setCheckState(denoise);
    ui->checkAGC->setCheckState(agc);
    ui->checkDereverb->setCheckState(dereverb);
    ui->checkEcho->setCheckState(echo);

    if (frames>0) {
        ui->spinFrames->setValue(frames);
        ui->checkFrames->setChecked(true);
    } else {
        ui->spinFrames->setValue(CDefaults::speexFrames);
        ui->checkFrames->setChecked(false);
    }
    if (agcLevel>0) {
        ui->spinAGCLevel->setValue(agcLevel);
        ui->checkAGCLevel->setChecked(true);
    } else {
        ui->spinAGCLevel->setValue(CDefaults::speexAGCLevel);
        ui->checkAGCLevel->setChecked(false);
    }
    if (filterLength>0) {
        ui->spinFilterLength->setValue(filterLength);
        ui->checkFilterLength->setChecked(true);
    } else {
        ui->spinFilterLength->setValue(CDefaults::speexFilterLength);
        ui->checkFilterLength->setChecked(false);
    }
    updateState(0);
}

void ZSpeexDialog::getParams(Qt::CheckState &denoise, Qt::CheckState &agc, Qt::CheckState &dereverb,
                             Qt::CheckState &echo, int &frames, int &agcLevel, int &filterLength)
{
    denoise = ui->checkDenoise->checkState();
    agc = ui->checkAGC->checkState();
    dereverb = ui->checkDereverb->checkState();
    echo = ui->checkEcho->checkState();

    frames = ui->spinFrames->value();
    if (!ui->checkFrames->isChecked())
        frames = 0;

    agcLevel = ui->spinAGCLevel->value();
    if (!ui->checkAGCLevel->isChecked())
        agcLevel = 0;

    filterLength = ui->spinFilterLength->value();
    if (!ui->checkFilterLength->isChecked())
        filterLength = 0;
}

void ZSpeexDialog::updateState(int state)
{
    Q_UNUSED(state)

    ui->spinFrames->setEnabled(ui->checkFrames->isChecked());
    ui->spinAGCLevel->setEnabled(ui->checkAGCLevel->isChecked());
    ui->spinFilterLength->setEnabled(ui->checkFilterLength->isChecked());
}
