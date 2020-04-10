/***************************************************************************
*   Copyright (C) 2006 - 2020 by kernelonline@gmail.com                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

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
