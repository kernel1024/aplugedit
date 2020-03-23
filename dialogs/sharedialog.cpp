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

#include "includes/sharedialog.h"
#include "includes/generic.h"
#include "ui_sharedialog.h"

ZShareDialog::ZShareDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ZShareDialog)
{
    ui->setupUi(this);

    ui->comboIPCKey->setValidator(new QIntValidator(1024,INT_MAX));

    QRegularExpression rx(QSL("\\d{4}"));
    QValidator *permValidator = new QRegularExpressionValidator(rx, this);
    ui->comboIPCPerm->setValidator(permValidator);

    ui->comboIPCPerm->addItem(QSL("0660"));
    ui->comboIPCPerm->addItem(QSL("0666"));
    ui->comboIPCPerm->setEditText(QSL("0660"));

    ui->groupIPC->setEnabled(false);

    ui->tableBindings->setItemDelegate(new ZNumberDelegate(this));

    ui->comboHWPtrAlignment->insertItem(static_cast<int>(ZCPShare::HWPtrAlignment::haEmpty),tr("-not specified-"));
    ui->comboHWPtrAlignment->insertItem(static_cast<int>(ZCPShare::HWPtrAlignment::haAuto),tr("Auto"));
    ui->comboHWPtrAlignment->insertItem(static_cast<int>(ZCPShare::HWPtrAlignment::haNo),tr("No"));
    ui->comboHWPtrAlignment->insertItem(static_cast<int>(ZCPShare::HWPtrAlignment::haRoundUp),tr("Round up"));
    ui->comboHWPtrAlignment->insertItem(static_cast<int>(ZCPShare::HWPtrAlignment::haRoundDown),tr("Round down"));

    connect(ui->buttonAddBinding,&QPushButton::clicked,this,&ZShareDialog::addBinding);
    connect(ui->buttonDeleteBinding,&QPushButton::clicked,this,&ZShareDialog::deleteBinding);
}

ZShareDialog::~ZShareDialog()
{
    delete ui;
}

void ZShareDialog::setBindings(const QVector<int> &bindings)
{
    const static QStringList headers({ QSL("Channel"), QSL("Slave channel") });

    ui->tableBindings->clear();
    ui->tableBindings->setColumnCount(1);
    ui->tableBindings->setRowCount(bindings.count());
    ui->tableBindings->setHorizontalHeaderLabels(headers);
    for (int i=0;i<bindings.count();i++) {
        auto h = new QTableWidgetItem(QSL("%1").arg(i));
        ui->tableBindings->setVerticalHeaderItem(i,h);
        auto w = new QTableWidgetItem(QSL("%1").arg(bindings.at(i)));
        ui->tableBindings->setItem(i,0,w);
    }
}

QVector<int> ZShareDialog::getBindigs() const
{
    QVector<int> res;
    res.reserve(ui->tableBindings->rowCount());
    for (int i=0;i<ui->tableBindings->rowCount();i++) {
        bool ok;
        int c = ui->tableBindings->item(i,0)->text().toInt(&ok);
        if (ok) {
            res.append(c);
        } else {
            res.append(i);
        }
    }
    return res;
}

void ZShareDialog::setIPC(bool configurable, const QString &key, const QString &permissions, const QStringList &usedKeys)
{
    ui->groupIPC->setEnabled(configurable);
    ui->comboIPCKey->setEnabled(configurable);
    ui->comboIPCPerm->setEnabled(configurable);

    ui->comboIPCKey->addItems(usedKeys);
    ui->comboIPCKey->setEditText(key);
    ui->comboIPCPerm->setEditText(permissions);
}

void ZShareDialog::getIPC(QString &key, QString &permissions)
{
    key = ui->comboIPCKey->lineEdit()->text();
    permissions = ui->comboIPCPerm->lineEdit()->text();
}

void ZShareDialog::setPtrParams(Qt::CheckState slowPtr, ZCPShare::HWPtrAlignment ptrAlignment)
{
    ui->checkSlowPtr->setCheckState(slowPtr);
    int idx = static_cast<int>(ptrAlignment);
    if ((idx>=0) && (idx<ui->comboHWPtrAlignment->count()))
            ui->comboHWPtrAlignment->setCurrentIndex(idx);
}

void ZShareDialog::getPtrParams(Qt::CheckState &slowPtr, ZCPShare::HWPtrAlignment &ptrAlignment)
{
    slowPtr = ui->checkSlowPtr->checkState();
    int idx = ui->comboHWPtrAlignment->currentIndex();
    if ((idx>=0) && (idx<static_cast<int>(ZCPShare::HWPtrAlignment::ha_Max)))
        ptrAlignment = static_cast<ZCPShare::HWPtrAlignment>(idx);
}

void ZShareDialog::addBinding()
{
    int idx = ui->tableBindings->rowCount();
    ui->tableBindings->insertRow(idx);

    auto h = new QTableWidgetItem(QSL("%1").arg(idx));
    ui->tableBindings->setVerticalHeaderItem(idx,h);
    auto w = new QTableWidgetItem(QSL("%1").arg(idx));
    ui->tableBindings->setItem(idx,0,w);
}

void ZShareDialog::deleteBinding()
{
    const int row = ui->tableBindings->currentRow();
    if ((row>=0) && (row<ui->tableBindings->rowCount()))
        ui->tableBindings->removeRow(row);
}

ZNumberDelegate::ZNumberDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

ZNumberDelegate::~ZNumberDelegate() = default;

QWidget *ZNumberDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    auto lineEdit = new QLineEdit(parent);
    auto validator = new QIntValidator(0, 99, lineEdit);
    lineEdit->setValidator(validator);
    return lineEdit;
}
