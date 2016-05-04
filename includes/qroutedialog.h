/***************************************************************************
*   Copyright (C) 2006 by Kernel                                          *
*   kernelonline@bk.ru                                                    *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
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

#ifndef QROUTEDLG_H
#define QROUTEDLG_H
#include <QtGui>
#include <QtCore>
#include "ui_qroutedlg.h"
#include "includes/cproute.h"

class QRouteDialog : public QDialog, public Ui::QRouteDialog
{
    Q_OBJECT

public:
    QRouteDialog(QWidget *parent = 0);
    ~QRouteDialog();
    int alChannelsIn, alChannelsOut;
    int getOutChannels();
    int getInChannels();
    void setParams(int inChannels, int outChannels, TalRoute &alTable);
    void getTable(TalRoute &alTable);
public slots:
    void channelsChanged(int index);
};

#endif
