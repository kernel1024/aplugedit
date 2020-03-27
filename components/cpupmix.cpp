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
#include "includes/cpupmix.h"
#include "ui_upmixdlg.h"

ZCPUpmix::ZCPUpmix(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp = new ZCPInput(this, QSL("in"));
    registerInput(fInp);

    fOut = new ZCPOutput(this, QSL("out"));
    registerOutput(fOut);
}

ZCPUpmix::~ZCPUpmix() = default;

void ZCPUpmix::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));

    m_channels = json.toObject().value(QSL("channels")).toInt(0);
    m_delay = json.toObject().value(QSL("delay")).toInt(0);
}

QJsonValue ZCPUpmix::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());

    data.insert(QSL("channels"),m_channels);
    data.insert(QSL("delay"),m_delay);

    return data;
}

QSize ZCPUpmix::minimumSizeHint() const
{
    return QSize(180,50);
}

void ZCPUpmix::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    paintBase(p);

    setBaseFont(p,ftTitle);
    p.drawText(rect(),Qt::AlignCenter,QSL("Upmix"));

    QString conf = QSL("-> auto");
    if (m_channels>0)
        conf = QSL("-> %1").arg(m_channels);
    if (m_delay>0)
        conf.append(QSL(" (%1 ms)").arg(m_delay));
    setBaseFont(p,ftDesc);
    p.drawText(QRect(0,2*height()/3,width(),height()/3),Qt::AlignCenter,conf);

    p.restore();
}

void ZCPUpmix::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/2);
}

void ZCPUpmix::doInfoGenerate(QTextStream &stream, QStringList &warnings) const
{
    stream << QSL("pcm.") << objectName() << QSL(" {") << endl;
    stream << QSL("  type upmix") << endl;
    if (fOut->toFilter) {
        stream << QSL("  slave {") << endl;
        stream << QSL("    pcm \"") << fOut->toFilter->objectName() << QSL("\"") << endl;
        stream << QSL("  }") << endl;
    }
    if (m_channels>0)
        stream << QSL("  channels %1").arg(m_channels) << endl;
    if (m_delay>0)
        stream << QSL("  delay %1").arg(m_delay) << endl;
    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << endl;
    stream << endl;
}

void ZCPUpmix::showSettingsDlg()
{
    QDialog dlg(window());
    Ui::ZUpmixDialog ui;
    ui.setupUi(&dlg);

    switch (m_channels) {
        case 1: ui.comboChannels->setCurrentIndex(1); break;
        case 2: ui.comboChannels->setCurrentIndex(2); break;
        case 4: ui.comboChannels->setCurrentIndex(3); break;
        case 6: ui.comboChannels->setCurrentIndex(4); break;
        case 8: ui.comboChannels->setCurrentIndex(5); break;
        default: ui.comboChannels->setCurrentIndex(0);
    }

    ui.spinDelay->setValue(m_delay);

    if (dlg.exec()==QDialog::Rejected) return;

    switch (ui.comboChannels->currentIndex()) {
        case 1: m_channels = 1; break;
        case 2: m_channels = 2; break;
        case 3: m_channels = 4; break;
        case 4: m_channels = 6; break;
        case 5: m_channels = 8; break;
        default: m_channels = 0; break;
    }

    m_delay = ui.spinDelay->value();

    Q_EMIT componentChanged(this);
    update();
}
