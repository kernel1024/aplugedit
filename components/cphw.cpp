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
#include "includes/cphw.h"
#include "includes/hwdialog.h"

ZCPHW::ZCPHW(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this,this);
    fInp->pinName=QSL("in");
    registerInput(fInp);
    m_format=QSL("<NONE>");
}

ZCPHW::~ZCPHW() = default;

QSize ZCPHW::minimumSizeHint() const
{
    return QSize(180,50);
}

int ZCPHW::getRate() const
{
    return m_rate;
}

int ZCPHW::getChannels() const
{
    return m_channels;
}

void ZCPHW::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
}

void ZCPHW::doInfoGenerate(QTextStream & stream) const
{
    stream << QSL("pcm.") << objectName() << QSL(" {") << endl;
    stream << QSL("  type hw") << endl;
    if (m_card==-1) {
        stream << QSL("  pcm \"hw:0,0\"") << endl;
    } else {
        stream << QSL("  card ") << m_card << endl;
        if (m_device!=-1) {
            stream << QSL("  device ") << m_device << endl;
            if (m_subdevice!=-1)
                stream << QSL("  subdevice ") << m_subdevice << endl;
        }
    }
    if (m_mmap_emulation==0) {
        stream << QSL("  mmap_emulation false") << endl;
    } else if (m_mmap_emulation==1) {
        stream << QSL("  mmap_emulation true") << endl;
    }

    if (m_sync_ptr_ioctl==0) {
        stream << QSL("  sync_ptr_ioctl false") << endl;
    } else if (m_sync_ptr_ioctl==1) {
        stream << QSL("  sync_ptr_ioctl true") << endl;
    }

    if (m_nonblock==0) {
        stream << QSL("  nonblock false") << endl;
    } else if (m_nonblock==1) {
        stream << QSL("  nonblock true") << endl;
    }

    if (!m_format.startsWith(QSL("<NONE>")))
        stream << QSL("  format ") << m_format << endl;

    if (m_channels!=-1)
        stream << QSL("  channels ") << m_channels << endl;

    if (m_rate!=-1)
        stream << QSL("  rate ") << m_rate << endl;

    stream << QSL("}") << endl;
    stream << endl;
}

void ZCPHW::readFromStreamLegacy( QDataStream & stream )
{
    ZCPBase::readFromStreamLegacy(stream);
    stream >> m_card >> m_device >> m_subdevice;
    stream >> m_mmap_emulation >> m_sync_ptr_ioctl >> m_nonblock;
    stream >> m_format;
    stream >> m_channels >> m_rate;
}

void ZCPHW::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));
    m_card = json.toObject().value(QSL("card")).toInt(0);
    m_device = json.toObject().value(QSL("device")).toInt(-1);
    m_subdevice = json.toObject().value(QSL("subdevice")).toInt(-1);
    m_format = json.toObject().value(QSL("format")).toString();
    m_channels = json.toObject().value(QSL("channels")).toInt(-1);
    m_rate = json.toObject().value(QSL("rate")).toInt(-1);
    m_mmap_emulation = json.toObject().value(QSL("mmap_emulation")).toInt(-1);
    m_sync_ptr_ioctl = json.toObject().value(QSL("sync_ptr_ioctl")).toInt(-1);
    m_nonblock = json.toObject().value(QSL("nonblock")).toInt(-1);
}

QJsonValue ZCPHW::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());
    data.insert(QSL("card"),m_card);
    data.insert(QSL("device"),m_device);
    data.insert(QSL("subdevice"),m_subdevice);
    data.insert(QSL("format"),m_format);
    data.insert(QSL("channels"),m_channels);
    data.insert(QSL("rate"),m_rate);
    data.insert(QSL("mmap_emulation"),m_mmap_emulation);
    data.insert(QSL("sync_ptr_ioctl"),m_sync_ptr_ioctl);
    data.insert(QSL("nonblock"),m_nonblock);
    return data;
}

void ZCPHW::paintEvent(QPaintEvent * event)
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
    p.drawText(rect(),Qt::AlignCenter,QSL("HW output"));

    n.setBold(false);
    n.setPointSize(n.pointSize()-3);
    p.setPen(QPen(Qt::gray));
    p.setFont(n);
    QString rate;
    if (m_rate==-1) {
        rate=QSL("* Hz");
    } else if (m_rate<10000) {
        rate=QSL("%1 Hz").arg(m_rate);
    } else {
        rate=QSL("%1 kHz").arg(static_cast<double>(m_rate)/1000,1,'f',1);
    }
    QString s=QSL("hw:%1,%2 ch:%3, %4")
              .arg(m_card)
              .arg(m_device)
              .arg(m_channels)
              .arg(rate);
    p.drawText(QRect(0,height()/3,width(),height()),Qt::AlignCenter,s);

    p.setFont(of);
    p.setBrush(ob);
    p.setPen(op);
}

void ZCPHW::showSettingsDlg()
{
    ZHWDialog d(topLevelWidget());
    d.setParams(m_card,m_device,m_subdevice,m_mmap_emulation,m_sync_ptr_ioctl,m_nonblock,
                m_channels,m_rate,m_format);

    if (d.exec()==QDialog::Rejected)
        return;

    Q_EMIT componentChanged(this);

    d.getParams(m_card,m_device,m_subdevice,m_mmap_emulation,m_sync_ptr_ioctl,m_nonblock,
                m_channels,m_rate,m_format);
    update();
}