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

#include "includes/cpasym.h"
#include "includes/generic.h"

ZCPAsym::ZCPAsym(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp = new ZCPInput(this, QSL("in"));
    registerInput(fInp);

    fOutPlayback = new ZCPOutput(this, QSL("play"));
    registerOutput(fOutPlayback);

    fOutCapture = new ZCPOutput(this, QSL("rec"));
    registerOutput(fOutCapture);
}

ZCPAsym::~ZCPAsym() = default;

QSize ZCPAsym::minimumSizeHint() const
{
    return QSize(180,50);
}

void ZCPAsym::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    fOutPlayback->relCoord=QPoint(width()-zcpPinSize/2,height()/3);
    fOutCapture->relCoord=QPoint(width()-zcpPinSize/2,2*height()/3);
}

void ZCPAsym::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    paintBase(p);

    setBaseFont(p,ftTitle);
    p.drawText(rect(),Qt::AlignCenter,QSL("Asym"));

    p.restore();
}

void ZCPAsym::doInfoGenerate(QTextStream &stream, QStringList &warnings) const
{
    stream << QSL("pcm.") << objectName() << QSL(" {") << Qt::endl;
    stream << QSL("  type asym") << Qt::endl;
    if (fOutPlayback->toFilter)
        stream << QSL("  playback.pcm \"%1\"").arg(fOutPlayback->toFilter->objectName()) << Qt::endl;
    if (fOutCapture->toFilter)
        stream << QSL("  capture.pcm \"%1\"").arg(fOutCapture->toFilter->objectName()) << Qt::endl;

    if ((fOutPlayback->toFilter==nullptr) ||
            (fOutCapture->toFilter==nullptr))
        warnings.append(tr("Asym plugin: both slave PCMs (playback and capture) must be connected."));

    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << Qt::endl;
    stream << Qt::endl;
}
