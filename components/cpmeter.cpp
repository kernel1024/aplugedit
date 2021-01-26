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

#include <QFileInfo>
#include "includes/generic.h"
#include "includes/cpmeter.h"
#include "includes/meterdialog.h"

ZCPMeter::ZCPMeter(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this, QSL("in"));
    registerInput(fInp);

    fOut=new ZCPOutput(this, QSL("out"));
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

void ZCPMeter::doInfoGenerate(QTextStream & stream, QStringList &warnings) const
{
    stream << QSL("pcm_scope_type.") << objectName() << QSL(" {") << Qt::endl;
    if (!m_meterLib.isEmpty())
        stream << QSL("  lib \"") << m_meterLib << QSL("\"") << Qt::endl;
    if (!m_meterFunc.isEmpty())
        stream << QSL("  open \"") << m_meterFunc << QSL("\"") << Qt::endl;
    stream << QSL("}") << Qt::endl;
    stream << Qt::endl;

    stream << QSL("pcm_scope.") << objectName() << QSL(" {") << Qt::endl;
    stream << QSL("  type ") << objectName() << Qt::endl;
    stream << QSL("}") << Qt::endl;
    stream << Qt::endl;

    stream << QSL("pcm.") << objectName() << QSL(" {") << Qt::endl;
    stream << QSL("  type meter") << Qt::endl;
    if (fOut->toFilter) {
        stream << QSL("  slave {") << Qt::endl;
        stream << QSL("    pcm \"") << fOut->toFilter->objectName() << QSL("\"") << Qt::endl;
        stream << QSL("  }") << Qt::endl;
    }
    stream << QSL("  frequency ") << m_refreshRate << Qt::endl;
    stream << QSL("  scopes.0 ") << objectName() << Qt::endl;
    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << Qt::endl;
    stream << Qt::endl;
}

void ZCPMeter::paintEvent (QPaintEvent * event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    paintBase(p);

    setBaseFont(p,ftTitle);
    p.drawText(rect(),Qt::AlignCenter,QSL("VU Meter"));

    QFileInfo fi(m_meterLib);
    setBaseFont(p,ftDesc);
    p.drawText(QRect(0,2*height()/3,width(),height()/3),Qt::AlignCenter,fi.fileName());

    p.restore();
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
    ZMeterDialog d(window());
    d.setParams(m_meterLib,m_meterFunc,m_refreshRate);

    if (d.exec()==QDialog::Rejected) return;

    Q_EMIT componentChanged(this);

    d.getParams(m_meterLib,m_meterFunc,m_refreshRate);
    update();
}

