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

#ifndef ROUTEDLG_H
#define ROUTEDLG_H

#include <QDialog>
#include "includes/cproute.h"
#include "ui_routedlg.h"

class ZRouteDialog : public QDialog, public Ui::ZRouteDialog
{
    Q_OBJECT

private:
    int alChannelsIn { 2 };
    int alChannelsOut { 2 };

public:
    ZRouteDialog(QWidget *parent = nullptr);
    ~ZRouteDialog() override;
    int getOutChannels() const;
    int getInChannels() const;
    void setParams(int inChannels, const QVector<CRouteItem> &alTable);
    QVector<CRouteItem> getTable() const;

public Q_SLOTS:
    void channelsChanged(int index);
};

#endif
