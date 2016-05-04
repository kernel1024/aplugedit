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


#include <QtGui>
#include <QtCore>
#include "includes/qroutedialog.h"

QRouteDialog::QRouteDialog(QWidget *parent)
  : QDialog(parent)
{
  setupUi(this);
  
  alChannelsInput->addItem("1 - mono");
  alChannelsInput->addItem("2 - stereo");
  alChannelsInput->addItem("4 - quadro");
  alChannelsInput->addItem("6 - 5.1 speakers");
  alChannelsInput->addItem("8 - 7.1 speakers");
  
  alChannelsOutput->addItem("1 - mono");
  alChannelsOutput->addItem("2 - stereo");
  alChannelsOutput->addItem("4 - quadro");
  alChannelsOutput->addItem("6 - 5.1 speakers");
  alChannelsOutput->addItem("8 - 7.1 speakers");
  
  alChannelsInput->setCurrentIndex(1);
  alChannelsOutput->setCurrentIndex(1);
  
  channelsChanged(1);
  
  connect(alChannelsInput,SIGNAL(currentIndexChanged(int)),this,SLOT(channelsChanged(int)));
  connect(alChannelsOutput,SIGNAL(currentIndexChanged(int)),this,SLOT(channelsChanged(int)));
}

QRouteDialog::~QRouteDialog()
{
}

int QRouteDialog::getOutChannels()
{
  switch (alChannelsOutput->currentIndex()) {
    case 0: return 1;
    case 1: return 2;
    case 2: return 4;
    case 3: return 6;
    case 4: return 8;
  }
  return 2;
}

int QRouteDialog::getInChannels()
{
  switch (alChannelsInput->currentIndex()) {
    case 0: return 1;
    case 1: return 2;
    case 2: return 4;
    case 3: return 6;
    case 4: return 8;
  }
  return 2;
}

void QRouteDialog::getTable(TalRoute &alTable)
{
  for (int i=0;i<alChannelsOut;i++)
  {
    int j=alRouteTable->item(i,1)->text().toInt();
    alTable[j].from=alRouteTable->item(i,0)->text().toInt();
    alTable[j].coeff=alRouteTable->item(i,2)->text().toFloat();
  }
}

void QRouteDialog::setParams(int inChannels, int outChannels, TalRoute &alTable)
{
  switch (inChannels) {
    case 1: alChannelsInput->setCurrentIndex(0); break;
    case 2: alChannelsInput->setCurrentIndex(1); break;
    case 4: alChannelsInput->setCurrentIndex(2); break;
    case 6: alChannelsInput->setCurrentIndex(3); break;
    case 8: alChannelsInput->setCurrentIndex(4); break;
    default: alChannelsInput->setCurrentIndex(2);
  }
  switch (outChannels) {
    case 1: alChannelsOutput->setCurrentIndex(0); break;
    case 2: alChannelsOutput->setCurrentIndex(1); break;
    case 4: alChannelsOutput->setCurrentIndex(2); break;
    case 6: alChannelsOutput->setCurrentIndex(3); break;
    case 8: alChannelsOutput->setCurrentIndex(4); break;
    default: alChannelsOutput->setCurrentIndex(2);
  }
  channelsChanged(1);
  
  alRouteTable->clear();
  alRouteTable->setColumnCount(3);
  alRouteTable->setRowCount(alChannelsOut);
 
  QStringList s;
  s << "From input" << "To output" << "Multiplier";
  alRouteTable->setHorizontalHeaderLabels(s);
  for (int i=0;i<outChannels;i++)
  {
    QTableWidgetItem *a=new QTableWidgetItem(tr("%1").arg(alTable[i].from));
    QTableWidgetItem *b=new QTableWidgetItem(tr("%1").arg(i));
    QTableWidgetItem *c=new QTableWidgetItem(tr("%1").arg(alTable[i].coeff,0,'f',1));
    alRouteTable->setItem(i,0,a);
    alRouteTable->setItem(i,1,b);
    alRouteTable->setItem(i,2,c);
  }
}

void QRouteDialog::channelsChanged(int )
{
  alChannelsIn=getInChannels();
  alChannelsOut=getOutChannels();
  
  alRouteTable->clear();
  alRouteTable->setColumnCount(3);
  alRouteTable->setRowCount(alChannelsOut);
 
  QStringList s;
  s << "From input" << "To output" << "Multiplier";
  alRouteTable->setHorizontalHeaderLabels(s);

  int j=0;
  for (int i=0;i<alChannelsOut;i++)
  {
    QTableWidgetItem *a=new QTableWidgetItem(tr("%1").arg(j));
    QTableWidgetItem *b=new QTableWidgetItem(tr("%1").arg(i));
    QTableWidgetItem *c=new QTableWidgetItem("1.0");
    alRouteTable->setItem(i,0,a);
    alRouteTable->setItem(i,1,b);
    alRouteTable->setItem(i,2,c);
    j++;
    if (j>=alChannelsIn) j=0;
  }
}
