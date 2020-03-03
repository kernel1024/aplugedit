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
#include "includes/cpmeter.h"
#include "includes/meterdialog.h"

ZCPMeter::ZCPMeter(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this,this);
    fInp->pinName=QSL("in");
    registerInput(fInp);
    fOut=new ZCPOutput(this,this);
    fOut->pinName=QSL("out");
    registerOutput(fOut);
}

ZCPMeter::~ZCPMeter() = default;

QSize ZCPMeter::minimumSizeHint() const
{
    return QSize(180,50);
}

void ZCPMeter::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/2);
}

void ZCPMeter::doInfoGenerate(QTextStream & stream) const
{
    stream << QSL("pcm_scope_type.") << objectName() << QSL(" {") << endl;
    if (!m_meterLib.isEmpty())
        stream << QSL("  lib \"") << m_meterLib << QSL("\"") << endl;
    if (!m_meterFunc.isEmpty())
        stream << QSL("  open \"") << m_meterFunc << QSL("\"") << endl;
    stream << QSL("}") << endl;
    stream << endl;

    stream << QSL("pcm_scope.") << objectName() << QSL(" {") << endl;
    stream << QSL("  type ") << objectName() << endl;
    stream << QSL("}") << endl;
    stream << endl;

    stream << QSL("pcm.") << objectName() << QSL(" {") << endl;
    stream << QSL("  type meter") << endl;
    if (fOut->toFilter) {
        stream << QSL("  slave {") << endl;
        stream << QSL("    pcm \"") << fOut->toFilter->objectName() << QSL("\"") << endl;
        stream << QSL("  }") << endl;
    }
    stream << QSL("  frequency ") << m_refreshRate << endl;
    stream << QSL("  scopes.0 ") << objectName() << endl;
    stream << QSL("}") << endl;
    stream << endl;
    if (fOut->toFilter)
        fOut->toFilter->doGenerate(stream);
}

void ZCPMeter::paintEvent (QPaintEvent * event)
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
    p.drawText(rect(),Qt::AlignCenter,QSL("VU Meter"));

    n.setBold(false);
    n.setPointSize(n.pointSize()-3);
    p.setPen(QPen(Qt::gray));
    p.setFont(n);
    
    QFileInfo fi(m_meterLib);
    p.drawText(QRect(0,height()/3,width(),height()),Qt::AlignCenter,fi.fileName());

    p.setFont(of);
    p.setBrush(ob);
    p.setPen(op);
}

void ZCPMeter::readFromStreamLegacy( QDataStream & stream )
{
    ZCPBase::readFromStreamLegacy(stream);
    stream >> m_meterLib;
    stream >> m_meterFunc;
    stream >> m_refreshRate;
}

void ZCPMeter::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));
    m_meterLib = json.toObject().value(QSL("meterLib")).toString();
    m_meterFunc = json.toObject().value(QSL("meterFunc")).toString();
    m_refreshRate = json.toObject().value(QSL("refreshRate")).toInt();
}

QJsonValue ZCPMeter::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());
    data.insert(QSL("meterLib"),m_meterLib);
    data.insert(QSL("meterFunc"),m_meterFunc);
    data.insert(QSL("refreshRate"),m_refreshRate);
    return data;
}

void ZCPMeter::showSettingsDlg()
{
    ZMeterDialog d(topLevelWidget());
    d.setParams(m_meterLib,m_meterFunc,m_refreshRate);

    if (d.exec()==QDialog::Rejected) return;

    Q_EMIT componentChanged(this);

    d.getParams(m_meterLib,m_meterFunc,m_refreshRate);
    update();
}
