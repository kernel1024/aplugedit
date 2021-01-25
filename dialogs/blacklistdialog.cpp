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

#include <QtWidgets>
#include "includes/blacklistdialog.h"
#include "includes/generic.h"
#include "ui_blacklistdialog.h"

ZBlacklistDialog::ZBlacklistDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ZBlacklistDialog)
{
    ui->setupUi(this);

    ui->comboPCMs->setItemDelegate(new ZDescListItemDelegate(this));
    ui->listBlacklist->setItemDelegate(new ZDescListItemDelegate(this));

    connect(ui->buttonAdd,&QPushButton::clicked,this,&ZBlacklistDialog::addPCM);
    connect(ui->buttonDelete,&QPushButton::clicked,this,&ZBlacklistDialog::deletePCM);

    updatePCMList();
}

ZBlacklistDialog::~ZBlacklistDialog()
{
    delete ui;
}

void ZBlacklistDialog::setBlacklist(const QVector<CPCMItem> &blacklistPCMs)
{
    ui->listBlacklist->clear();
    for (const auto& pcm : blacklistPCMs) {
        auto *item = new QListWidgetItem();
        item->setText(pcm.name);
        item->setData(Qt::UserRole,pcm.description);
        ui->listBlacklist->addItem(item);
    }
}

QVector<CPCMItem> ZBlacklistDialog::getBlacklist()
{
    QVector<CPCMItem> res;
    res.reserve(ui->listBlacklist->count());
    for (int i=0;i<ui->listBlacklist->count();i++) {
        auto *item = ui->listBlacklist->item(i);
        if ((item!=nullptr) && (item->data(Qt::UserRole).canConvert<QStringList>())) {
            res.append(CPCMItem(item->text(),
                                item->data(Qt::UserRole).toStringList()));
        }
    }

    return res;
}

void ZBlacklistDialog::addPCM()
{
    QString pcm = ui->comboPCMs->currentText();
    if (!pcm.isEmpty() && ui->comboPCMs->currentData().canConvert<QStringList>()) {
        QStringList desc = ui->comboPCMs->currentData().toStringList();

        auto *item = new QListWidgetItem();
        item->setText(pcm);
        item->setData(Qt::UserRole,desc);
        ui->listBlacklist->addItem(item);
    }
}

void ZBlacklistDialog::deletePCM()
{
    const auto items = ui->listBlacklist->selectedItems();
    for (const auto &idx : items) {
        auto *item = ui->listBlacklist->takeItem(ui->listBlacklist->row(idx));
        delete item;
    }
}

void ZBlacklistDialog::updatePCMList()
{
    ui->comboPCMs->clear();
    const auto alsaPCMs = gAlsa->pcmList();
    if (gAlsa->isWarnings())
        QMessageBox::warning(this,tr("ALSA warnings"),gAlsa->getAlsaWarnings().join(QChar('\n')));
    for (const auto& pcm : alsaPCMs) {
        ui->comboPCMs->addItem(pcm.name,pcm.description);
        int lastIdx = ui->comboPCMs->count()-1;
        ui->comboPCMs->setItemData(lastIdx,pcm.description.join(QSL("\n")),Qt::ToolTipRole);
    }
}
