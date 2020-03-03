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

#include "includes/cpplug.h"

ZCPPlug::ZCPPlug(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this,this);
    fInp->pinName=QSL("in");
    registerInput(fInp);
    fOut=new ZCPOutput(this,this);
    fOut->pinName=QSL("out");
    registerOutput(fOut);
}

ZCPPlug::~ZCPPlug() = default;

QSize ZCPPlug::minimumSizeHint() const
{
    return QSize(150,50);
}

void ZCPPlug::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    QPen op=p.pen();
    QBrush ob=p.brush();
    QFont of=p.font();

    paintBase(p);

    QFont n=of;
    n.setBold(true);
    n.setPointSize(n.pointSize()+1);
    p.setFont(n);
    p.drawText(rect(),Qt::AlignCenter,QSL("Plug"));

    p.setFont(of);
    p.setBrush(ob);
    p.setPen(op);}

void ZCPPlug::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/2);
}

void ZCPPlug::doInfoGenerate(QTextStream &stream) const
{
    stream << QSL("pcm.") << objectName() << QSL(" {") << endl;
    stream << QSL("  type plug") << endl;
    stream << QSL("  slave.pcm \"%1\"").arg(fOut->toFilter->objectName()) << endl;
    ZCPBase::doInfoGenerate(stream);
    stream << QSL("}") << endl;
    stream << endl;
    if (fOut->toFilter)
        fOut->toFilter->doGenerate(stream);
}
