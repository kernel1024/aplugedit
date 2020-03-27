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
#include "includes/cpnull.h"

ZCPNull::ZCPNull(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this, QSL("in"));
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

void ZCPNull::doInfoGenerate(QTextStream & stream, QStringList &warnings) const
{
    stream << QSL("pcm.") << objectName() << QSL(" {") << endl;
    stream << QSL("  type null") << endl;
    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << endl;
    stream << endl;
}

void ZCPNull::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    paintBase(p);

    setBaseFont(p,ftTitle);
    p.drawText(rect(),Qt::AlignCenter,QSL("Null"));

    p.restore();
}

