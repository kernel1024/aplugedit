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

#ifndef QHWDLG_H
#define QHWDLG_H
#include <QtGui>
#include <QtCore>
#include "ui_qhwdlg.h"

class tlDevices;
class tlCards;
  
class QHWDialog : public QDialog, public Ui::QHWDialog
{
    Q_OBJECT

public:
    QList<tlCards *> hwCnt;
    
    QHWDialog(QWidget *parent = 0);
    ~QHWDialog();
public slots:
    void cardSelected(int index);
    void devSelected(int index);
};

class tlDevices : public QObject
{
  Q_OBJECT
public:
  int devNum;
  int subdevices;
};

class tlCards : public QObject
{
  Q_OBJECT
public:
  QString cardName;
  int cardNum;
  QList<tlDevices *> devices;
};

#endif
