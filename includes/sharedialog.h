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

#ifndef SHAREDIALOG_H
#define SHAREDIALOG_H

#include <QDialog>
#include <QStyledItemDelegate>
#include "cpshare.h"

namespace Ui {
class ZShareDialog;
}

class ZShareDialog : public QDialog
{
    Q_OBJECT
private:
    Ui::ZShareDialog *ui;

public:
    explicit ZShareDialog(QWidget *parent = nullptr);
    ~ZShareDialog() override;

    void setBindings(const QVector<int> &bindings);
    QVector<int> getBindigs() const;

    void setIPC(bool configurable, const QString& key, const QString& permissions,
                const QStringList& usedKeys = QStringList());
    void getIPC(QString& key, QString& permissions);

    void setPtrParams(Qt::CheckState slowPtr, ZCPShare::HWPtrAlignment ptrAlignment);
    void getPtrParams(Qt::CheckState &slowPtr, ZCPShare::HWPtrAlignment &ptrAlignment);

private Q_SLOTS:
    void addBinding();
    void deleteBinding();

};

class ZNumberDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ZNumberDelegate(QObject *parent = nullptr);
    ~ZNumberDelegate() override;

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem & option,
                      const QModelIndex & index) const override;
};

#endif // SHAREDIALOG_H
