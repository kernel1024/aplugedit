/***************************************************************************
*   Copyright (C) 2006 - 2021 by kernelonline@gmail.com                   *
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
#include "includes/cpequal.h"
#include "includes/equalizerdialog.h"

ZCPEqual::ZCPEqual(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this, QSL("in"));
    registerInput(fInp);

    fOut=new ZCPOutput(this, QSL("out"));
    registerOutput(fOut);

    fCtlInp=new ZCPInput(this, QSL("ctl"), CStructures::PinClass::pcCTL);
    registerInput(fCtlInp);

    setMinimumHeight(65);
}

void ZCPEqual::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));
    m_channels = json.toObject().value(QSL("channels")).toInt(-1);
    m_controls = json.toObject().value(QSL("controls")).toString();
    m_plugin = CLADSPAPlugItem(json.toObject().value(QSL("plugin")));
}

QJsonValue ZCPEqual::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());
    data.insert(QSL("channels"),m_channels);
    data.insert(QSL("controls"),m_controls);
    data.insert(QSL("plugin"),m_plugin.storeToJson());
    return data;
}

ZCPEqual::~ZCPEqual() = default;

QSize ZCPEqual::minimumSizeHint() const
{
    return QSize(200,minimumHeight());
}

void ZCPEqual::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/3);
    fCtlInp->relCoord=QPoint(zcpPinSize/2,2*height()/3);
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/2);
}

void ZCPEqual::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    int hintHeight = paintBase(p,true);

    setBaseFont(p,ftTitle);
    QRect trect(0,hintHeight,width(),p.fontMetrics().height()+5);
    p.drawText(trect,Qt::AlignCenter,QSL("Equalizer"));

    setBaseFont(p,ftDesc);
    QRect drect(0,hintHeight+trect.height(),
                width(),height()-hintHeight-trect.height());
    QString filters = tr("Eq10");
    if (!m_plugin.isEmpty())
        filters = m_plugin.plugName;
    if (m_channels>0)
        filters.prepend(tr("channels: %1\n").arg(m_channels));

    int minHeight = hintHeight+trect.height()+
            p.fontMetrics().boundingRect(0,0,width(),0,Qt::AlignCenter | Qt::TextWordWrap,filters).height()+2;
    if (minimumHeight()!=minHeight)
        setMinimumHeight(minHeight);

    p.drawText(drect,Qt::AlignCenter | Qt::TextWordWrap,filters);

    p.restore();
}

void ZCPEqual::doInfoGenerate(QTextStream &stream, QStringList &warnings) const
{
    if (!isFloatConverterPresent()) {
        warnings.append(tr("AlsaEqual plugin: FLOAT converter not connected to the output of alsaequal plugin. "
                           "Consider to use PLUG or FLOAT CONVERTER plugin at the output of alsaequal."));
    }
    stream << QSL("pcm.") << objectName() << QSL(" {") << Qt::endl;
    stream << QSL("  type equal") << Qt::endl;
    if (fOut->toFilter) {
        stream << QSL("  slave {") << Qt::endl;
        stream << QSL("    pcm \"%1\"").arg(fOut->toFilter->objectName()) << Qt::endl;
        stream << QSL("  }") << Qt::endl;
    }
    if (m_channels>0)
        stream << QSL("  channels %1").arg(m_channels) << Qt::endl;
    if (!m_controls.isEmpty())
        stream << QSL("  controls \"%1\"").arg(m_controls) << Qt::endl;
    if (!m_plugin.isEmpty()) {
        QFileInfo fi(m_plugin.plugLibrary);
        stream << QSL("  library \"%1\"").arg(fi.absoluteFilePath()) << Qt::endl;
        stream << QSL("  module \"%1\"").arg(m_plugin.plugLabel) << Qt::endl;
    }
    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << Qt::endl;
    stream << Qt::endl;

    if (fCtlInp->links.isEmpty()) {
        stream << QSL("ctl.") << objectName() << QSL(" {") << Qt::endl;
        generateCtl(stream,warnings);
        doHintGenerate(stream);
        stream << QSL("}") << Qt::endl;
        stream << Qt::endl;
    }
}

void ZCPEqual::doCtlGenerate(QTextStream &stream, QStringList &warnings, bool softvol) const
{
    Q_UNUSED(softvol)

    generateCtl(stream,warnings);
}

void ZCPEqual::generateCtl(QTextStream &stream, QStringList &warnings) const
{
    Q_UNUSED(warnings)

    stream << QSL("  type equal") << Qt::endl;
    if (m_channels>0)
        stream << QSL("  channels %1").arg(m_channels) << Qt::endl;
    if (!m_controls.isEmpty())
        stream << QSL("  controls \"%1\"").arg(m_controls) << Qt::endl;
    if (!m_plugin.isEmpty()) {
        QFileInfo fi(m_plugin.plugLibrary);
        stream << QSL("  library \"%1\"").arg(fi.absoluteFilePath()) << Qt::endl;
        stream << QSL("  module \"%1\"").arg(m_plugin.plugLabel) << Qt::endl;
    }
}

void ZCPEqual::showSettingsDlg()
{
    ZEqualizerDialog d(window());

    d.setParams(m_channels,m_controls,m_plugin);

    if (d.exec()==QDialog::Rejected) return;

    Q_EMIT componentChanged(this);

    d.getParams(m_channels,m_controls,m_plugin);
    update();
}
