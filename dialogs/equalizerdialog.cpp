/***************************************************************************
*   Copyright (C) 2006 - 2021 by kernelonline@gmail.com                   *
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

#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include "includes/generic.h"
#include "includes/equalizerdialog.h"
#include "includes/ladspadialog.h"
#include "ui_equalizerdialog.h"

ZEqualizerDialog::ZEqualizerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ZEqualizerDialog)
{
    ui->setupUi(this);
    connect(ui->buttonControlsBrowse,&QPushButton::clicked,this,&ZEqualizerDialog::browseControls);
    connect(ui->buttonPluginClear,&QPushButton::clicked,this,&ZEqualizerDialog::resetPlugin);
    connect(ui->buttonPluginSelect,&QPushButton::clicked,this,&ZEqualizerDialog::selectPlugin);
    updatePluginLabel();
}

ZEqualizerDialog::~ZEqualizerDialog()
{
    delete ui;
}

void ZEqualizerDialog::setParams(int channels, const QString &controls, const CLADSPAPlugItem &plugin)
{
    if (channels <= 0) {
        ui->spinChannels->setValue(0);
    } else {
        ui->spinChannels->setValue(channels);
    }
    ui->editControls->setText(controls);
    m_plugin = plugin;
    updatePluginLabel();
}

void ZEqualizerDialog::getParams(int &channels, QString &controls, CLADSPAPlugItem &plugin)
{
    if (ui->spinChannels->value() < 1) {
        channels = -1;
    } else {
        channels = ui->spinChannels->value();
    }
    controls = ui->editControls->text();
    plugin = m_plugin;
}

void ZEqualizerDialog::updatePluginLabel()
{
    if (m_plugin.isEmpty()) {
        ui->labelPlugin->setText(tr("(default)"));
        return;
    }
    QFileInfo fi(m_plugin.plugLibrary);
    ui->labelPlugin->setText(QSL("%1 (%2)").arg(m_plugin.plugLabel,fi.absoluteFilePath()));
}

void ZEqualizerDialog::fixControls()
{
    QString fileName = ui->editControls->text();

    QFile fconf;
    if (fileName.startsWith(QChar('/'))) {
        fconf.setFileName(fileName);
    } else {
        fconf.setFileName(QDir::home().absoluteFilePath(fileName));
    }

    if (fconf.exists() &&
            (QMessageBox::question(this,QGuiApplication::applicationDisplayName(),
                                   tr("Plugin changed. Remove old controls file?")) == QMessageBox::Yes)) {
        fconf.remove();
    }
}

void ZEqualizerDialog::browseControls()
{
    QString s = QFileDialog::getSaveFileName(this,tr("Choose a equalizer controls file"));
    if (s.isEmpty()) return;
    ui->editControls->setText(s);
}

void ZEqualizerDialog::selectPlugin()
{
    const int defaultSampleRate = 44100;
    int channels = 2;
    if (ui->spinChannels->value() > 0)
        channels = ui->spinChannels->value();
    qint64 oldPluginID = m_plugin.plugID;

    ZLADSPADialog dlg(this,channels,defaultSampleRate);
    dlg.setSimpleParamsMode(true);
    dlg.setPlugItem(m_plugin);

    if (dlg.exec()==QDialog::Rejected) return;

    m_plugin = dlg.getPlugItem();
    updatePluginLabel();

    if (oldPluginID != m_plugin.plugID)
        fixControls();
}

void ZEqualizerDialog::resetPlugin()
{
    m_plugin = CLADSPAPlugItem();
    updatePluginLabel();
    fixControls();
}
