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

#ifndef BLACKLISTDIALOG_H
#define BLACKLISTDIALOG_H

#include <QDialog>
#include "alsabackend.h"

namespace Ui {
class ZBlacklistDialog;
}

class ZBlacklistDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ZBlacklistDialog(QWidget *parent = nullptr);
    ~ZBlacklistDialog();

    void setBlacklist(const QVector<CPCMItem> &blacklistPCMs);
    QVector<CPCMItem> getBlacklist();

private Q_SLOTS:
    void addPCM();
    void deletePCM();
    void updatePCMList();

private:
    Ui::ZBlacklistDialog *ui;
};

#endif // BLACKLISTDIALOG_H
