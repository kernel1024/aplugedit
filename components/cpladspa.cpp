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
#include "includes/cpladspa.h"
#include "includes/cphw.h"
#include "includes/cprate.h"

ZCPLADSPA::ZCPLADSPA(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this,this);
    fInp->pinName=QSL("in");
    registerInput(fInp);
    fOut=new ZCPOutput(this,this);
    fOut->pinName=QSL("out");
    registerOutput(fOut);

    m_plugName=tr("<please select filter!!!>");
}

ZCPLADSPA::~ZCPLADSPA() = default;

QSize ZCPLADSPA::minimumSizeHint() const
{
    return QSize(200,50);
}

void ZCPLADSPA::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/2);
}

void ZCPLADSPA::doInfoGenerate(QTextStream & stream) const
{
    stream << QSL("pcm.") << objectName() << QSL(" {") << endl;
    stream << QSL("  type ladspa") << endl;
    if (fOut->toFilter)
    {
        stream << QSL("  slave {") << endl;
        stream << QSL("    pcm \"") << fOut->toFilter->objectName() << QSL("\"") << endl;
        stream << QSL("  }") << endl;
        if (!m_plugLabel.isEmpty())
        {
            QFileInfo fi(m_plugLibrary);
            stream << QSL("  path \"") << fi.path() << QSL("\"") << endl;
            stream << QSL("  plugins [") << endl;
            stream << QSL("    {") << endl;
            stream << QSL("      label ") << m_plugLabel << endl;
            stream << QSL("      input {") << endl;
            stream << QSL("        controls [ ");
            for (int i=0;i<m_plugControls.count();i++)
            {
                switch (m_plugControls.at(i).aatType)
                {
                    case ZLADSPA::aacToggle:
                        if (m_plugControls.at(i).aasToggle) {
                            stream << QSL("1");
                        } else {
                            stream << QSL("0");
                        }
                        break;
                    case ZLADSPA::aacFreq: stream << m_plugControls.at(i).aasFreq; break;
                    case ZLADSPA::aacInteger: stream << m_plugControls.at(i).aasInt; break;
                    default: stream << m_plugControls.at(i).aasValue;
                }
                stream << QSL(" ");
            }
            stream << QSL("]") << endl
                          << QSL("      }") << endl
                          << QSL("    }") << endl
                          << QSL("  ]") << endl;
        }
    }
    ZCPBase::doInfoGenerate(stream);
    stream << QSL("}") << endl;
    stream << endl;
    if (fOut->toFilter)
        fOut->toFilter->doGenerate(stream);
}

void ZCPLADSPA::paintEvent(QPaintEvent * event)
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
    p.drawText(rect(),Qt::AlignCenter,QSL("LADSPA"));

    n.setBold(false);
    n.setPointSize(n.pointSize()-3);
    p.setPen(QPen(Qt::gray));
    p.setFont(n);
    
    p.drawText(QRect(0,2*height()/3,width(),height()/3),Qt::AlignCenter,m_plugName);

    p.setFont(of);
    p.setBrush(ob);
    p.setPen(op);
}

void ZCPLADSPA::readFromStreamLegacy( QDataStream & stream )
{
    ZCPBase::readFromStreamLegacy(stream);
    stream >> m_plugName >> m_plugID >> m_plugLabel >> m_plugLibrary;
    int cn;
    stream >> cn;
    for (int i=0;i<cn;i++)
        m_plugControls << ZLADSPAControlItem(stream);
}

void ZCPLADSPA::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));
    m_plugName = json.toObject().value(QSL("plugName")).toString();
    m_plugID = json.toObject().value(QSL("plugID")).toString();
    m_plugLabel = json.toObject().value(QSL("plugLabel")).toString();
    m_plugLibrary = json.toObject().value(QSL("plugLibrary")).toString();

    const QJsonArray controls = json.toObject().value(QSL("controls")).toArray();
    for (const auto& item : controls)
        m_plugControls << ZLADSPAControlItem(item);
}

QJsonValue ZCPLADSPA::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());
    data.insert(QSL("plugName"),m_plugName);
    data.insert(QSL("plugID"),m_plugID);
    data.insert(QSL("plugLabel"),m_plugLabel);
    data.insert(QSL("plugLibrary"),m_plugLibrary);

    QJsonArray controls;
    for (int i=0;i<m_plugControls.count();i++)
        controls.append(m_plugControls[i].storeToJson());
    data.insert(QSL("controls"),controls);

    return data;
}

int ZCPLADSPA::searchSampleRate()
{
    static bool sampleRateWarn=false;

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
        if (!sampleRateWarn) {
            QMessageBox::warning(topLevelWidget(),tr("LADSPA settings"),
                                 tr("Samplerate not defined!\n\n"
                                    "Please connect this LADSPA filter to SRC or HW component\n"
                                    "and specify Rate parameter in theirs settings.\n"
                                    "Samplerate adjusted to default 48000 Hz.\n"
                                    "This setting is used by LADSPA plugins in frequency calculations."));
        }
        sampleRateWarn=true;
        sampleRate=48000;
        // TODO: add checks for FLOAT data type. Only FLOAT is supported by LADSPA. Suggest plug plugins. We need to add plug plugin too.
    }
    return sampleRate;
}

void ZCPLADSPA::showSettingsDlg()
{
    ZLADSPADialog d(topLevelWidget(),searchSampleRate());

    d.setParams(m_plugLabel,m_plugID,m_plugControls);

    if (d.exec()==QDialog::Rejected) return;

    Q_EMIT componentChanged(this);

    d.getParams(m_plugLabel,m_plugID,m_plugName,m_plugLibrary,m_plugControls);
    update();
}

