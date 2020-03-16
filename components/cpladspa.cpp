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
#include "includes/cpladspa.h"
#include "includes/cphw.h"
#include "includes/cprate.h"
#include "includes/cpplug.h"
#include "includes/cpconv.h"

ZCPLADSPA::ZCPLADSPA(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this,this);
    fInp->pinName=QSL("in");
    registerInput(fInp);
    fOut=new ZCPOutput(this,this);
    fOut->pinName=QSL("out");
    registerOutput(fOut);

    setMinimumHeight(60);
}

ZCPLADSPA::~ZCPLADSPA() = default;

QSize ZCPLADSPA::minimumSizeHint() const
{
    return QSize(200,minimumHeight());
}

void ZCPLADSPA::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/2);
}

void ZCPLADSPA::doInfoGenerate(QTextStream & stream) const
{
    if (!isConverterPresent()) {
        QMessageBox::warning(topLevelWidget(),tr("LADSPA plugin configuration"),
                             tr("FLOAT converter not connected to the output of LADSPA plugin.\n"
                                "Consider to use PLUG or FLOAT CONVERTER plugin at the output of LADSPA."));
    }
    stream << QSL("pcm.") << objectName() << QSL(" {") << endl;
    stream << QSL("  type ladspa") << endl;
    if (fOut->toFilter) {
        stream << QSL("  slave {") << endl;
        stream << QSL("    pcm \"%1\"").arg(fOut->toFilter->objectName()) << endl;
        stream << QSL("  }") << endl;
        stream << QSL("  channels %1").arg(m_channels) << endl;
        stream << QSL("  path \"%1\"").arg(ZGenericFuncs::getLADSPAPath()) << endl;

        if (!m_plugins.isEmpty()) {
            stream << QSL("  plugins {") << endl;
            int idx = 0;
            for (const auto& plug : qAsConst(m_plugins)) {
                QFileInfo fi(plug.plugLibrary);
                QString policy = QSL("none");
                if (plug.policy == ZLADSPA::Policy::plDuplicate)
                    policy = QSL("duplicate");

                stream << QSL("    %1 {").arg(idx++) << endl;
                stream << QSL("      id %1  # Label: '%2'").arg(plug.plugID).arg(plug.plugLabel) << endl;
                stream << QSL("      filename \"%1\"").arg(fi.absoluteFilePath()) << endl;
                if (plug.usePolicy)
                    stream << QSL("      policy %1").arg(policy) << endl;
                for (const auto& inp : qAsConst(plug.inputBindings)) {
                    stream << QSL("      input.bindings.%1 \"%2\"")
                              .arg(inp.first).arg(inp.second) << endl;
                }
                for (const auto& out : qAsConst(plug.outputBindings)) {
                    stream << QSL("      output.bindings.%1 \"%2\"")
                              .arg(out.first).arg(out.second) << endl;
                }
                stream << QSL("      input {") << endl;
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
                stream << QSL("]") << endl
                       << QSL("      }") << endl
                       << QSL("    }") << endl;
            }
        }
    }
    stream << QSL("  }") << endl;
    ZCPBase::doInfoGenerate(stream);
    stream << QSL("}") << endl;
    stream << endl;
}

void ZCPLADSPA::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event)

    QPainter p(this);
    QPen op=p.pen();
    QBrush ob=p.brush();
    QFont of=p.font();

    int hintHeight = paintBase(p,true);

    QFont n=of;
    n.setBold(true);
    n.setPointSize(n.pointSize()+1);
    p.setFont(n);
    QRect trect(0,hintHeight,width(),p.fontMetrics().height()+5);
    p.drawText(trect,Qt::AlignCenter,QSL("LADSPA"));

    n.setBold(false);
    n.setPointSize(n.pointSize()-3);
    p.setPen(QPen(Qt::gray));
    p.setFont(n);
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

    p.setFont(of);
    p.setBrush(ob);
    p.setPen(op);
}

void ZCPLADSPA::readFromStreamLegacy( QDataStream & stream )
{
    ZCPBase::readFromStreamLegacy(stream);
    QString plugName;
    QString plugID;
    QString plugLabel;
    QString plugLibrary;

    // read legacy info
    stream >> plugName >> plugID >> plugLabel >> plugLibrary;
    int cn;
    stream >> cn;
    QVector<CLADSPAControlItem> controls;
    controls.reserve(cn);
    for (int i=0;i<cn;i++)
        controls << CLADSPAControlItem(stream);

    // create simple instance plugin
    m_plugins.clear();
    bool ok;
    qint64 pID = plugID.toLong(&ok);
    if (ok)
        m_plugins.append(CLADSPAPlugItem(plugLabel,pID,plugName,plugLibrary,controls));
}

void ZCPLADSPA::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));

    m_channels = json.toObject().value(QSL("channels")).toInt(2);

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

    QJsonArray plugs;
    for (const auto& plug : qAsConst(m_plugins))
        plugs.append(plug.storeToJson());
    data.insert(QSL("plugins"),plugs);

    return data;
}

int ZCPLADSPA::searchSampleRate() const
{
    ZCPBase* w=fOut->toFilter;
    int sampleRate=-1;
    while (w)
    {
        if (auto hw=qobject_cast<ZCPHW*>(w))
        {
            sampleRate=hw->getRate();
            break;
        }
        if (auto hw=qobject_cast<ZCPRate*>(w))
        {
            sampleRate=hw->getRate();
            break;
        }
        ZCPOutput* out = w->getMainOutput();
        if (out==nullptr) break;
        w=out->toFilter;
    }
    if (sampleRate==-1)
    {
        qWarning() << "LADSPA samplerate not defined by destination module (hw or rate). "
                      "Defaulting to 48kHz.";
        sampleRate=48000;
    }
    return sampleRate;
}

bool ZCPLADSPA::isConverterPresent() const
{
    ZCPBase* w=fOut->toFilter;
    while (w)
    {
        if (qobject_cast<ZCPPlug*>(w) != nullptr) return true;

        if (auto hw=qobject_cast<ZCPConv*>(w))
        {
            if (hw->getConverterType() == ZCPConv::ConverterType::alcFloat)
                return true;
        }

        ZCPOutput* out = w->getMainOutput();
        if (out==nullptr) break;
        w=out->toFilter;
    }

    return false;
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
    ZLADSPAListDialog d(topLevelWidget(),searchSampleRate());

    d.setParams(m_channels,m_plugins);

    if (d.exec()==QDialog::Rejected) return;

    Q_EMIT componentChanged(this);

    d.getParams(m_channels,m_plugins);
    update();
}

