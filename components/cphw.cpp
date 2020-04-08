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
#include "includes/cphw.h"
#include "includes/hwdialog.h"

ZCPHW::ZCPHW(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this, QSL("in"));
    registerInput(fInp);

    fCtlInp=new ZCPInput(this, QSL("ctl"), CStructures::PinClass::pcCTL);
    registerInput(fCtlInp);

    m_format=QSL("<NONE>");
}

ZCPHW::~ZCPHW() = default;

QSize ZCPHW::minimumSizeHint() const
{
    return QSize(180,65);
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
    fInp->relCoord=QPoint(zcpPinSize/2,height()/3);
    fCtlInp->relCoord=QPoint(zcpPinSize/2,2*height()/3);
}

void ZCPHW::doInfoGenerate(QTextStream & stream, QStringList &warnings) const
{
    stream << QSL("pcm.%1 {").arg(objectName()) << endl;
    stream << QSL("  type hw") << endl;
    if (m_card>=0) {
        if (m_preferSymbolicName && !m_cardSymbolic.isEmpty()) {
            stream << QSL("  card \"%1\"").arg(m_cardSymbolic) << endl;
        } else {
            stream << QSL("  card %1").arg(m_card) << endl;
        }
        if (m_device>=0) {
            stream << QSL("  device %1").arg(m_device) << endl;
            if (m_subdevice>=-1)
                stream << QSL("  subdevice %1").arg(m_subdevice) << endl;
        }
    } else {
        stream << QSL("  pcm \"hw:0,0\"") << endl;
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
        stream << QSL("  format \"%1\"").arg(m_format) << endl;

    if (m_channels!=-1)
        stream << QSL("  channels %1").arg(m_channels) << endl;

    if (m_rate!=-1)
        stream << QSL("  rate %1").arg(m_rate) << endl;

    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << endl;
    stream << endl;
}

void ZCPHW::doCtlGenerate(QTextStream &stream, QStringList &warnings) const
{
    int card = m_card;
    if (card<0) {
        warnings.append(tr("HW plugin: incorrect card number for ctl"));
        card = 0;
    }
    stream << QSL("  type hw") << endl;
    if (m_preferSymbolicName && !m_cardSymbolic.isEmpty()) {
        stream << QSL("  card \"%1\"").arg(m_cardSymbolic) << endl;
    } else {
        stream << QSL("  card %1").arg(card) << endl;
    }
}

void ZCPHW::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));
    m_card = json.toObject().value(QSL("card")).toInt(0);
    m_cardSymbolic = json.toObject().value(QSL("cardSymbolic")).toString(QString());
    m_preferSymbolicName = json.toObject().value(QSL("preferSymbolicName")).toBool(false);
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
    data.insert(QSL("cardSymbolic"),m_cardSymbolic);
    data.insert(QSL("preferSymbolicName"),m_preferSymbolicName);
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
    p.save();

    paintBase(p);

    setBaseFont(p,ftTitle);
    p.drawText(rect(),Qt::AlignCenter,QSL("HW output"));

    QString str = QSL("hw:default");
    if (m_card>=0) {
        if (m_preferSymbolicName && !m_cardSymbolic.isEmpty()) {
            str = QSL("hw:%1").arg(m_cardSymbolic);
        } else {
            str = QSL("hw:%1").arg(m_card);
        }
        if (m_device>=0)
            str.append(QSL(",%1").arg(m_device));
    }
    if (m_channels>0)
        str.append(QSL(" ch:%1").arg(m_channels));
    if (m_rate>0) {
        if (m_rate<10000) {
            str.append(QSL(" %1 Hz").arg(m_rate));
        } else {
            str.append(QSL(" %1 kHz").arg(static_cast<double>(m_rate)/1000,1,'f',1));
        }
    }
    setBaseFont(p,ftDesc);
    p.drawText(QRect(0,2*height()/3,width(),height()/3),Qt::AlignCenter,str);

    p.restore();
}

void ZCPHW::showSettingsDlg()
{
    ZHWDialog d(window());
    d.setParams(m_card,m_cardSymbolic,m_preferSymbolicName,m_device,m_subdevice,
                m_mmap_emulation,m_sync_ptr_ioctl,m_nonblock,m_channels,m_rate,m_format);

    if (d.exec()==QDialog::Rejected)
        return;

    Q_EMIT componentChanged(this);

    d.getParams(m_card,m_cardSymbolic,m_preferSymbolicName,m_device,m_subdevice,
                m_mmap_emulation,m_sync_ptr_ioctl,m_nonblock,m_channels,m_rate,m_format);
    update();
}
