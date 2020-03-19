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

#include <QRandomGenerator>
#include "includes/generic.h"
#include "includes/cpdmix.h"
#include "includes/cphw.h"

ZCPDMix::ZCPDMix(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this,this);
    fInp->pinName=QSL("in");
    registerInput(fInp);
    fOut=new ZCPOutput(this,this);
    fOut->pinName=QSL("out");
    registerOutput(fOut);
}

ZCPDMix::~ZCPDMix() = default;

QSize ZCPDMix::minimumSizeHint() const
{
    return QSize(140,50);
}

void ZCPDMix::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/2);
}

void ZCPDMix::doInfoGenerate(QTextStream & stream, QStringList &warnings) const
{
    auto hw=qobject_cast<ZCPHW*>(fOut->toFilter);
    stream << QSL("pcm.") << objectName() << QSL(" {") << endl;
    stream << QSL("  type dmix") << endl;
    stream << QSL("  ipc_key ") << QRandomGenerator::global()->bounded(1024,INT_MAX) << endl;
    if (fOut->toFilter) {
        stream << QSL("  slave {") << endl;
        stream << QSL("    pcm \"") << fOut->toFilter->objectName() << QSL("\"") << endl;
        if (hw) {
            if (hw->getChannels()!=-1)
                stream << QSL("    channels ") << hw->getChannels() << endl;
            if (hw->getRate()!=-1)
                stream << QSL("    rate ") << hw->getRate() << endl;
        }
        stream << QSL("  }") << endl;
        if (hw) {
            if (hw->getChannels()!=-1) {
                stream << QSL("  bindings {") << endl;
                for (int i=0;i<hw->getChannels();i++)
                    stream << QSL("    ") << i << QSL(" ") << i << endl;
                stream << QSL("  }") << endl;
            }
        }
    }
    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << endl;
    stream << endl;
}

bool ZCPDMix::canConnectOut(ZCPBase * toFilter)
{
    const auto base=qobject_cast<ZCPHW*>(toFilter);
    return (base!=nullptr);
}

void ZCPDMix::paintEvent (QPaintEvent * event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    paintBase(p);

    setBaseFont(p,ftTitle);
    p.drawText(rect(),Qt::AlignCenter,QSL("DMix"));

    p.restore();
}
