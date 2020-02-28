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

#include "includes/cpbase.h"
#include "includes/cpnull.h"

ZCPNull::ZCPNull(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this,this);
    fInp->pinName=QSL("in");
    registerInput(fInp);
}

ZCPNull::~ZCPNull() = default;

QSize ZCPNull::minimumSizeHint() const
{
    return QSize(150,50);
}

void ZCPNull::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
}

void ZCPNull::doInfoGenerate(QTextStream & stream) const
{
    stream << QSL("pcm.") << objectName() << QSL(" {") << endl;
    stream << QSL("  type null") << endl;
    stream << QSL("}") << endl;
    stream << endl;
}

void ZCPNull::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event)

    QPainter p(this);
    QPen pn=QPen(Qt::black);
    QPen op=p.pen();
    QBrush ob=p.brush();
    QFont of=p.font();
    pn.setWidth(2);
    p.setPen(pn);
    p.setBrush(QBrush(Qt::white,Qt::SolidPattern));

    p.drawRect(rect());

    redrawPins(p);

    QFont n=of;
    n.setBold(true);
    n.setPointSize(n.pointSize()+1);
    p.setFont(n);
    p.drawText(rect(),Qt::AlignCenter,QSL("null"));
    p.setFont(of);
    p.setBrush(ob);
    p.setPen(op);
}

