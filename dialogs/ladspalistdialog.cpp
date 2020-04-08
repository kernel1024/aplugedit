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

#include "includes/ladspalistdialog.h"
#include "includes/generic.h"
#include "ui_ladspalistdialog.h"

ZLADSPAListDialog::ZLADSPAListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ZLADSPAListDialog)
{
    ui->setupUi(this);
    model = new ZLADSPAListModel(this);
    ui->listPlugins->setModel(model);
    ui->listPlugins->setItemDelegate(new ZDescListItemDelegate(this));
    ui->listPlugins->viewport()->setAcceptDrops(true);

    ui->comboSampleRate->addItem(tr("8000 Hz"));
    ui->comboSampleRate->addItem(tr("11025 Hz"));
    ui->comboSampleRate->addItem(tr("22050 Hz"));
    ui->comboSampleRate->addItem(tr("44,1 kHz"));
    ui->comboSampleRate->addItem(tr("48 kHz"));
    ui->comboSampleRate->addItem(tr("96 kHz"));
    ui->comboSampleRate->addItem(tr("192 kHz"));
    ui->comboSampleRate->setCurrentIndex(3);

    connect(ui->buttonAdd,&QPushButton::clicked,this,&ZLADSPAListDialog::addPlugin);
    connect(ui->buttonDelete,&QPushButton::clicked,this,&ZLADSPAListDialog::deletePlugin);
    connect(ui->buttonEdit,&QPushButton::clicked,this,&ZLADSPAListDialog::editPlugin);
    connect(ui->listPlugins,&QListView::doubleClicked,this,&ZLADSPAListDialog::showEditPluginDialog);
}

ZLADSPAListDialog::~ZLADSPAListDialog()
{
    delete ui;
}

void ZLADSPAListDialog::setParams(int channels, int sampleRate, const QVector<CLADSPAPlugItem> &plugins)
{
    ui->spinChannels->setValue(channels);
    model->setItems(plugins);

    switch (sampleRate)
    {
        case 8000: ui->comboSampleRate->setCurrentIndex(0); break;
        case 11025: ui->comboSampleRate->setCurrentIndex(1); break;
        case 22050: ui->comboSampleRate->setCurrentIndex(2); break;
        case 44100: ui->comboSampleRate->setCurrentIndex(3); break;
        case 48000: ui->comboSampleRate->setCurrentIndex(4); break;
        case 96000: ui->comboSampleRate->setCurrentIndex(5); break;
        case 192000: ui->comboSampleRate->setCurrentIndex(6); break;
        default: ui->comboSampleRate->setCurrentIndex(3);
    }
}

void ZLADSPAListDialog::getParams(int &channels, int &sampleRate, QVector<CLADSPAPlugItem> &plugins)
{
    channels = ui->spinChannels->value();
    plugins = model->items();
    sampleRate = getSampleRate();
}

void ZLADSPAListDialog::addPlugin()
{
    ZLADSPADialog dlg(topLevelWidget(),ui->spinChannels->value(),getSampleRate());

    if (dlg.exec()==QDialog::Rejected) return;

    int idx = model->getRowIndex(ui->listPlugins->currentIndex());
    CLADSPAPlugItem item = dlg.getPlugItem();
    if (item.isEmpty()) return;

    if (idx<0) {
        model->addItem(item);
    } else {
        model->insertItem(idx,item);
    }
}

void ZLADSPAListDialog::deletePlugin()
{
    int idx = model->getRowIndex(ui->listPlugins->currentIndex());
    if (idx>=0)
        model->removeRow(idx);
}

void ZLADSPAListDialog::editPlugin()
{
    showEditPluginDialog(ui->listPlugins->currentIndex());
}

void ZLADSPAListDialog::showEditPluginDialog(const QModelIndex &index)
{
    int idx = model->getRowIndex(index);
    if (idx<0) return;

    ZLADSPADialog dlg(topLevelWidget(),ui->spinChannels->value(),getSampleRate());
    dlg.setPlugItem(model->items().at(idx));

    if (dlg.exec()==QDialog::Rejected) return;

    model->setItem(idx,dlg.getPlugItem());
}

int ZLADSPAListDialog::getSampleRate() const
{
    switch (ui->comboSampleRate->currentIndex())
    {
        case 0: return 8000;
        case 1: return 11025;
        case 2: return 22050;
        case 3: return 44100;
        case 4: return 48000;
        case 5: return 96000;
        case 6: return 192000;
    }
    return 44100;
}
