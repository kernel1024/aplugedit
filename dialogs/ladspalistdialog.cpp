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

ZLADSPAListDialog::ZLADSPAListDialog(QWidget *parent, int sampleRate) :
    QDialog(parent),
    ui(new Ui::ZLADSPAListDialog),
    m_sampleRate(sampleRate)
{
    ui->setupUi(this);
    model = new ZLADSPAListModel(this);
    ui->listPlugins->setModel(model);
    ui->listPlugins->setItemDelegate(new ZDescListItemDelegate());
    ui->listPlugins->viewport()->setAcceptDrops(true);

    connect(ui->buttonAdd,&QPushButton::clicked,this,&ZLADSPAListDialog::addPlugin);
    connect(ui->buttonDelete,&QPushButton::clicked,this,&ZLADSPAListDialog::deletePlugin);
    connect(ui->buttonEdit,&QPushButton::clicked,this,&ZLADSPAListDialog::editPlugin);
    connect(ui->listPlugins,&QListView::doubleClicked,this,&ZLADSPAListDialog::showEditPluginDialog);
}

ZLADSPAListDialog::~ZLADSPAListDialog()
{
    delete ui;
}

void ZLADSPAListDialog::setParams(int channels, const QVector<CLADSPAPlugItem> &plugins)
{
    ui->spinChannels->setValue(channels);
    model->setItems(plugins);
}

void ZLADSPAListDialog::getParams(int &channels, QVector<CLADSPAPlugItem> &plugins)
{
    channels = ui->spinChannels->value();
    plugins = model->items();
}

void ZLADSPAListDialog::addPlugin()
{
    ZLADSPADialog dlg(topLevelWidget(),ui->spinChannels->value(),m_sampleRate);

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

    ZLADSPADialog dlg(topLevelWidget(),ui->spinChannels->value(),m_sampleRate);
    dlg.setPlugItem(model->items().at(idx));

    if (dlg.exec()==QDialog::Rejected) return;

    model->setItem(idx,dlg.getPlugItem());
}
