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
#include "includes/cprate.h"
#include "includes/ratedialog.h"

int ZCPRate::getRate() const
{
    return m_rate;
}

ZCPRate::ZCPRate(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this,this);
    fInp->pinName=QSL("in");
    registerInput(fInp);
    fOut=new ZCPOutput(this,this);
    fOut->pinName=QSL("out");
    registerOutput(fOut);
}

ZCPRate::~ZCPRate() = default;

QSize ZCPRate::minimumSizeHint() const
{
    return QSize(140,50);
}

void ZCPRate::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/2);
}

void ZCPRate::doInfoGenerate(QTextStream & stream) const
{
    stream << QSL("pcm.") << objectName() << QSL(" {") << endl;
    stream << QSL("  type rate") << endl;
    if (fOut->toFilter) {
        stream << QSL("  slave {") << endl;
        stream << QSL("    pcm \"") << fOut->toFilter->objectName() << QSL("\"") << endl;
        stream << QSL("    rate ") << m_rate << endl;
        stream << QSL("  }") << endl;
    }
    if (!m_converter.isEmpty())
        stream << QSL("  converter \"") << m_converter << QSL("\"") << endl;
    stream << QSL("}") << endl;
    stream << endl;
    if (fOut->toFilter)
        fOut->toFilter->doGenerate(stream);
}

void ZCPRate::paintEvent(QPaintEvent * event)
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
    p.drawText(rect(),Qt::AlignCenter,QSL("SRC"));

    n.setBold(false);
    n.setPointSize(n.pointSize()-3);
    p.setPen(QPen(Qt::gray));
    p.setFont(n);
    QString s = QSL("%1 Hz").arg(m_rate);
    p.drawText(QRect(0,height()/3,width(),height()),Qt::AlignCenter,s);

    p.setFont(of);
    p.setBrush(ob);
    p.setPen(op);
}

void ZCPRate::readFromStreamLegacy( QDataStream & stream )
{
    ZCPBase::readFromStreamLegacy(stream);
    stream >> m_rate;
    stream >> m_converter;
}

void ZCPRate::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));
    m_rate = json.toObject().value(QSL("rate")).toInt();
    m_converter = json.toObject().value(QSL("converter")).toString();
}

QJsonValue ZCPRate::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());
    data.insert(QSL("rate"),m_rate);
    data.insert(QSL("converter"),m_converter);
    return data;
}

void ZCPRate::showSettingsDlg()
{
    ZRateDialog d(topLevelWidget());
    d.setParams(m_rate,m_converter);

    if (d.exec()==QDialog::Rejected) return;

    Q_EMIT componentChanged(this);

    m_rate=d.getRate();
    m_converter=d.getConverter();
    update();
}

