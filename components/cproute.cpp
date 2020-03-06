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
#include "includes/cproute.h"
#include "includes/routedialog.h"

ZCPRoute::ZCPRoute(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this,this);
    fInp->pinName=QSL("in");
    registerInput(fInp);
    fOut=new ZCPOutput(this,this);
    fOut->pinName=QSL("out");
    registerOutput(fOut);

    m_routeTable.reserve(2);
    m_routeTable << CRouteItem(0, 1.0);
    m_routeTable << CRouteItem(1, 1.0);
    m_channelsIn = 2;
}

ZCPRoute::~ZCPRoute() = default;

QSize ZCPRoute::minimumSizeHint() const
{
    return QSize(140,50);
}

int ZCPRoute::getChannelsOut() const
{
    return m_routeTable.count();
}

void ZCPRoute::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/2);
}

void ZCPRoute::doInfoGenerate(QTextStream & stream) const
{
    stream << QSL("pcm.") << objectName() << QSL(" {") << endl;
    stream << QSL("  type route") << endl;
    if (fOut->toFilter)
    {
        stream << QSL("  slave {") << endl;
        stream << QSL("    pcm \"") << fOut->toFilter->objectName() << QSL("\"") << endl;
        stream << QSL("    channels ") << getChannelsOut() << endl;
        stream << QSL("  }") << endl;
    }
    for (int i=0;i<getChannelsOut();i++)
    {
        stream << QSL("  ttable.") << m_routeTable.at(i).from << QSL(".") << i << QSL(" ")
               << QSL("%1").arg(m_routeTable.at(i).coeff,1,'f',1) << endl;
    }
    ZCPBase::doInfoGenerate(stream);
    stream << QSL("}") << endl;
    stream << endl;
}

void ZCPRoute::paintEvent(QPaintEvent * event)
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
    p.drawText(rect(),Qt::AlignCenter,QSL("Route"));

    n.setBold(false);
    n.setPointSize(n.pointSize()-3);
    p.setPen(QPen(Qt::gray));
    p.setFont(n);
    QString s=QSL("%1 -> %2").arg(m_channelsIn).arg(getChannelsOut());
    p.drawText(QRect(0,2*height()/3,width(),height()/3),Qt::AlignCenter,s);

    p.setFont(of);
    p.setBrush(ob);
    p.setPen(op);
}

void ZCPRoute::readFromStreamLegacy( QDataStream & stream )
{
    struct LegacyRouteItem {
      int from;
      float coeff;
    };

    ZCPBase::readFromStreamLegacy(stream);
    stream >> m_channelsIn;

    int tChannelsOut;
    stream >> tChannelsOut;
    m_routeTable.clear();
    m_routeTable.reserve(tChannelsOut);
    LegacyRouteItem legacyTable[8];
    stream.readRawData(reinterpret_cast<char*>(&legacyTable),sizeof(legacyTable));
    for (int i=0;i<tChannelsOut;i++)
        m_routeTable.append(CRouteItem(legacyTable[i].from,static_cast<double>(legacyTable[i].coeff)));
}

void ZCPRoute::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));
    m_channelsIn = json.toObject().value(QSL("channelsIn")).toInt();

    const QJsonArray table = json.toObject().value(QSL("table")).toArray();
    m_routeTable.clear();
    m_routeTable.reserve(table.count());
    for (const auto &item : table) {
        m_routeTable.append(CRouteItem(item.toObject().value(QSL("from")).toInt(),
                                       item.toObject().value(QSL("coeff")).toDouble()));
    }
}

QJsonValue ZCPRoute::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());
    data.insert(QSL("channelsIn"),m_channelsIn);

    QJsonArray table;
    for (int i=0;i<m_routeTable.count();i++) {
        QJsonObject item;
        item.insert(QSL("from"),m_routeTable.at(i).from);
        item.insert(QSL("coeff"),m_routeTable.at(i).coeff);
        table.insert(i,item);
    }
    data.insert(QSL("table"),table);

    return data;
}

void ZCPRoute::showSettingsDlg()
{
    ZRouteDialog d(topLevelWidget());
    d.setParams(m_channelsIn,m_routeTable);

    if (d.exec()==QDialog::Rejected) return;

    Q_EMIT componentChanged(this);

    m_channelsIn = d.getInChannels();
    m_routeTable = d.getTable();
    update();
}


CRouteItem::CRouteItem(const CRouteItem &other)
{
    from = other.from;
    coeff = other.coeff;
}

CRouteItem::CRouteItem(int aFrom, double aCoeff)
{
    from = aFrom;
    coeff = aCoeff;
}
