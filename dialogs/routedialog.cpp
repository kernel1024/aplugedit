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

#include "includes/generic.h"
#include "includes/routedialog.h"

ZRouteDialog::ZRouteDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    alRouteTable->setColumnCount(3);
    alRouteTable->setHorizontalHeaderLabels({ tr("From input"), tr("To output"), tr("Multiplier") });

    alChannelsInput->addItem(tr("1 - mono"));
    alChannelsInput->addItem(tr("2 - stereo"));
    alChannelsInput->addItem(tr("4 - quadro"));
    alChannelsInput->addItem(tr("6 - 5.1 speakers"));
    alChannelsInput->addItem(tr("8 - 7.1 speakers"));

    alChannelsOutput->addItem(tr("1 - mono"));
    alChannelsOutput->addItem(tr("2 - stereo"));
    alChannelsOutput->addItem(tr("4 - quadro"));
    alChannelsOutput->addItem(tr("6 - 5.1 speakers"));
    alChannelsOutput->addItem(tr("8 - 7.1 speakers"));

    alChannelsInput->setCurrentIndex(1);
    alChannelsOutput->setCurrentIndex(1);

    channelsChanged(1);

    connect(alChannelsInput,qOverload<int>(&QComboBox::currentIndexChanged),this,&ZRouteDialog::channelsChanged);
    connect(alChannelsOutput,qOverload<int>(&QComboBox::currentIndexChanged),this,&ZRouteDialog::channelsChanged);
}

ZRouteDialog::~ZRouteDialog() = default;

int ZRouteDialog::getOutChannels() const
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

int ZRouteDialog::getInChannels() const
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

QVector<CRouteItem> ZRouteDialog::getTable() const
{
    QVector<CRouteItem> res;
    res.reserve(alChannelsOut);
    for (int i=0;i<alChannelsOut;i++) {
        res << CRouteItem(alRouteTable->item(i,0)->text().toInt(),
                          alRouteTable->item(i,2)->text().toDouble());
    }
    return res;
}

void ZRouteDialog::setParams(int inChannels, const QVector<CRouteItem> &alTable)
{
    switch (inChannels) {
        case 1: alChannelsInput->setCurrentIndex(0); break;
        case 2: alChannelsInput->setCurrentIndex(1); break;
        case 4: alChannelsInput->setCurrentIndex(2); break;
        case 6: alChannelsInput->setCurrentIndex(3); break;
        case 8: alChannelsInput->setCurrentIndex(4); break;
        default: alChannelsInput->setCurrentIndex(2);
    }
    switch (alTable.count()) {
        case 1: alChannelsOutput->setCurrentIndex(0); break;
        case 2: alChannelsOutput->setCurrentIndex(1); break;
        case 4: alChannelsOutput->setCurrentIndex(2); break;
        case 6: alChannelsOutput->setCurrentIndex(3); break;
        case 8: alChannelsOutput->setCurrentIndex(4); break;
        default: alChannelsOutput->setCurrentIndex(2);
    }
    channelsChanged(1);

    alRouteTable->clearContents();
    alRouteTable->setRowCount(alChannelsOut);

    for (int i=0;i<alTable.length();i++)
    {
        auto a = new QTableWidgetItem(tr("%1").arg(alTable.at(i).from));
        auto b = new QTableWidgetItem(tr("%1").arg(i));
        b->setFlags(b->flags() & ~Qt::ItemIsEditable);
        auto c = new QTableWidgetItem(tr("%1").arg(alTable.at(i).coeff,0,'f',1));
        alRouteTable->setItem(i,0,a);
        alRouteTable->setItem(i,1,b);
        alRouteTable->setItem(i,2,c);
    }
}

void ZRouteDialog::channelsChanged(int index)
{
    Q_UNUSED(index)

    alChannelsIn=getInChannels();
    alChannelsOut=getOutChannels();

    alRouteTable->setRowCount(alChannelsOut);

    int j=0;
    for (int i=0;i<alChannelsOut;i++)
    {
        if (alRouteTable->item(i,0)==nullptr) {
            auto a = new QTableWidgetItem(QSL("%1").arg(j));
            alRouteTable->setItem(i,0,a);
        }
        if (alRouteTable->item(i,1)==nullptr) {
            auto b = new QTableWidgetItem(QSL("%1").arg(i));
            b->setFlags(b->flags() & ~Qt::ItemIsEditable);
            alRouteTable->setItem(i,1,b);
        }
        if (alRouteTable->item(i,2)==nullptr) {
            auto c = new QTableWidgetItem(QSL("1.0"));
            alRouteTable->setItem(i,2,c);
        }
        j++;
        if (j>=alChannelsIn) j=0;
    }
}
