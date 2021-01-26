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
#include "includes/cpladspa.h"
#include "includes/cphw.h"

ZCPLADSPA::ZCPLADSPA(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this, QSL("in"));
    registerInput(fInp);

    fOut=new ZCPOutput(this, QSL("out"));
    registerOutput(fOut);

    fCtlOut=new ZCPOutput(this, QSL("ctl"),CStructures::PinClass::pcCTL);
    registerOutput(fCtlOut);

    setMinimumHeight(65);
}

ZCPLADSPA::~ZCPLADSPA() = default;

QSize ZCPLADSPA::minimumSizeHint() const
{
    return QSize(200,minimumHeight());
}

void ZCPLADSPA::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/3);
    fCtlOut->relCoord=QPoint(width()-zcpPinSize/2,2*height()/3);
}

void ZCPLADSPA::doInfoGenerate(QTextStream & stream, QStringList &warnings) const
{
    if (!isFloatConverterPresent()) {
        warnings.append(tr("LADSPA plugin: FLOAT converter not connected to the output of LADSPA plugin. "
                           "Consider to use PLUG or FLOAT CONVERTER plugin at the output of LADSPA."));
    }
    stream << QSL("pcm.") << objectName() << QSL(" {") << Qt::endl;
    stream << QSL("  type ladspa") << Qt::endl;
    if (fOut->toFilter) {
        stream << QSL("  slave {") << Qt::endl;
        stream << QSL("    pcm \"%1\"").arg(fOut->toFilter->objectName()) << Qt::endl;
        stream << QSL("  }") << Qt::endl;
        stream << QSL("  channels %1").arg(m_channels) << Qt::endl;
        stream << QSL("  path \"%1\"").arg(ZGenericFuncs::getLADSPAPath()) << Qt::endl;

        if (!m_plugins.isEmpty()) {
            stream << QSL("  plugins {") << Qt::endl;
            int idx = 0;
            for (const auto& plug : qAsConst(m_plugins)) {
                QFileInfo fi(plug.plugLibrary);
                QString policy = QSL("none");
                if (plug.policy == ZLADSPA::Policy::plDuplicate)
                    policy = QSL("duplicate");

                stream << QSL("    %1 {").arg(idx++) << Qt::endl;
                stream << QSL("      id %1  # Label: '%2'").arg(plug.plugID).arg(plug.plugLabel) << Qt::endl;
                stream << QSL("      filename \"%1\"").arg(fi.absoluteFilePath()) << Qt::endl;
                if (plug.usePolicy)
                    stream << QSL("      policy %1").arg(policy) << Qt::endl;
                for (const auto& inp : qAsConst(plug.inputBindings)) {
                    stream << QSL("      input.bindings.%1 \"%2\"")
                              .arg(inp.first).arg(inp.second) << Qt::endl;
                }
                for (const auto& out : qAsConst(plug.outputBindings)) {
                    stream << QSL("      output.bindings.%1 \"%2\"")
                              .arg(out.first).arg(out.second) << Qt::endl;
                }
                stream << QSL("      input {") << Qt::endl;
                stream << QSL("        controls [ ");
                for (const auto& ctl : qAsConst(plug.plugControls)) {
                    switch (ctl.aatType)
                    {
                        case ZLADSPA::aacToggle:
                            if (ctl.aasToggle) {
                                stream << QSL("1");
                            } else {
                                stream << QSL("0");
                            }
                            break;
                        case ZLADSPA::aacFreq: stream << ctl.aasFreq; break;
                        case ZLADSPA::aacInteger: stream << ctl.aasInt; break;
                        default: stream << ctl.aasValue;
                    }
                    stream << QSL(" ");
                }
                stream << QSL("]") << Qt::endl
                       << QSL("      }") << Qt::endl
                       << QSL("    }") << Qt::endl;
            }
        }
    }
    stream << QSL("  }") << Qt::endl;
    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << Qt::endl;
    stream << Qt::endl;
    if (fCtlOut->toFilter) {
        stream << QSL("ctl.") << objectName() << QSL(" {") << Qt::endl;
        fCtlOut->toFilter->doCtlGenerate(stream,warnings);
        stream << QSL("}") << Qt::endl;
        stream << Qt::endl;
    }
}

void ZCPLADSPA::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    int hintHeight = paintBase(p,true);

    setBaseFont(p,ftTitle);
    QRect trect(0,hintHeight,width(),p.fontMetrics().height()+5);
    p.drawText(trect,Qt::AlignCenter,QSL("LADSPA"));

    setBaseFont(p,ftDesc);
    QRect drect(0,hintHeight+trect.height(),
                width(),height()-hintHeight-trect.height());
    QString filters = tr("<please select filter!!!>");
    if (!m_plugins.isEmpty())
        filters = getPlugNames().join(QSL(", "));
    filters.prepend(tr("channels: %1\n").arg(m_channels));

    int minHeight = hintHeight+trect.height()+
            p.fontMetrics().boundingRect(0,0,width(),0,Qt::AlignCenter | Qt::TextWordWrap,filters).height()+2;
    if (minimumHeight()!=minHeight)
        setMinimumHeight(minHeight);
    
    p.drawText(drect,Qt::AlignCenter | Qt::TextWordWrap,filters);

    p.restore();
}

void ZCPLADSPA::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));

    m_channels = json.toObject().value(QSL("channels")).toInt(2);
    m_sampleRate = json.toObject().value(QSL("sampleRate")).toInt(44100);

    m_plugins.clear();
    const QJsonArray plugins = json.toObject().value(QSL("plugins")).toArray();
    for (const auto& plug : plugins)
        m_plugins.append(CLADSPAPlugItem(plug));
}

QJsonValue ZCPLADSPA::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());
    data.insert(QSL("channels"),m_channels);
    data.insert(QSL("sampleRate"),m_sampleRate);

    QJsonArray plugs;
    for (const auto& plug : qAsConst(m_plugins))
        plugs.append(plug.storeToJson());
    data.insert(QSL("plugins"),plugs);

    return data;
}

QStringList ZCPLADSPA::getPlugNames() const
{
    QStringList res;
    res.reserve(m_plugins.count());
    for (const auto& plug : qAsConst(m_plugins))
        res.append(plug.plugName);

    return res;
}

void ZCPLADSPA::showSettingsDlg()
{
    ZLADSPAListDialog d(window());

    d.setParams(m_channels,m_sampleRate,m_plugins);

    if (d.exec()==QDialog::Rejected) return;

    Q_EMIT componentChanged(this);

    d.getParams(m_channels,m_sampleRate,m_plugins);
    update();
}

