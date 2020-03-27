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

#include "includes/cpmulti.h"
#include "includes/generic.h"
#include "includes/multidlg.h"
#include "includes/cproute.h"
#include "includes/cpplug.h"

ZCPMulti::ZCPMulti(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this, QSL("in"));
    registerInput(fInp);

    regenerateOutputs(1);
    regenerateCapacity(2);
}

ZCPMulti::~ZCPMulti() = default;

const int zcpMultiPinStep = 20;
const int zcpMultiPinMargin = 25;

QSize ZCPMulti::minimumSizeHint() const
{
    return QSize(180,(2*zcpMultiPinMargin)
                 + (fOutputs.count()-1)*zcpMultiPinStep);
}

void ZCPMulti::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    for (int i=0;i<fOutputs.count();i++) {
        fOutputs[i]->relCoord = QPoint(width()-zcpPinSize/2,
                                       zcpMultiPinMargin + i*zcpMultiPinStep);
    }
}

void ZCPMulti::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    paintBase(p);

    setBaseFont(p,ftTitle);
    p.drawText(rect(),Qt::AlignCenter,QSL("Multi"));

    QChar separator(' ');
    if (fOutputs.count()>2)
        separator = QChar('\n');
    QString desc = QSL("IN: %1 ch,%2OUT: %3 dev")
                   .arg(m_bindings.count())
                   .arg(separator)
                   .arg(fOutputs.count());
    setBaseFont(p,ftDesc);
    p.drawText(QRect(0,2*height()/3,width(),height()/3),Qt::AlignCenter,desc);

    p.restore();
}

void ZCPMulti::readFromJson(const QJsonValue &json)
{
    // reallocate outputs
    const QJsonArray joutputs = json.toObject().value(QSL("outputs")).toArray();
    regenerateOutputs(joutputs.count());
    int idx = 0;
    for (const auto &item : joutputs) {
        m_slaveChannels[idx] = item.toObject().value(QSL("channels")).toInt(2);
        idx++;
    }

    // and now read link info for outputs
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));

    const QJsonArray jbindings = json.toObject().value(QSL("bindings")).toArray();
    regenerateCapacity(jbindings.count());
    idx = 0;
    for (const auto &item : jbindings) {
        m_bindings[idx].first = item.toObject().value(QSL("slave")).toInt(0);
        m_bindings[idx].second = item.toObject().value(QSL("channel")).toInt(0);
        idx++;
    }
}

QJsonValue ZCPMulti::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());

    QJsonArray joutputs;
    for (int i=0;i<m_slaveChannels.count();i++) {
        QJsonObject item;
        item.insert(QSL("channels"),m_slaveChannels.at(i));
        joutputs.insert(i,item);
    }
    data.insert(QSL("outputs"),joutputs);

    QJsonArray jbindings;
    for (int i=0;i<m_bindings.count();i++) {
        QJsonObject item;
        item.insert(QSL("slave"),m_bindings.at(i).first);
        item.insert(QSL("channel"),m_bindings.at(i).second);
        jbindings.insert(i,item);
    }
    data.insert(QSL("bindings"),jbindings);

    return data;
}

void ZCPMulti::doInfoGenerate(QTextStream &stream, QStringList &warnings) const
{
    if (!isConverterPresent()) {
        warnings.append(tr("Multi plugin: PLUG or ROUTE plugin not connected to the input of MULTI plugin. "
                           "Consider to use some interleaved <-> complex converter at the input of MULTI."));
    }
    stream << QSL("pcm.") << objectName() << QSL(" {") << endl;
    stream << QSL("  type multi") << endl;
    stream << QSL("  slaves {") << endl;
    for (int i=0;i<fOutputs.count();i++) {
        stream << QSL("    s%1 {").arg(i) << endl;
        if (fOutputs.at(i)->toFilter) {
            stream << QSL("      pcm \"%1\"").arg(fOutputs.at(i)->toFilter->objectName()) << endl;
            stream << QSL("      channels ") << m_slaveChannels.at(i) << endl;
        }
        stream << QSL("    }") << endl;

    }
    stream << QSL("  }") << endl;
    stream << QSL("  bindings {") << endl;
    for (int i=0;i<m_bindings.count();i++)
    {
        stream << QSL("    %1 {").arg(i) << endl;
        stream << QSL("      slave \"s%1\"").arg(m_bindings.at(i).first) << endl;
        stream << QSL("      channel %1").arg(m_bindings.at(i).second) << endl;
        stream << QSL("    }") << endl;
    }
    stream << QSL("  }") << endl;
    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << endl;
    stream << endl;
}

void ZCPMulti::showSettingsDlg()
{
    ZMultiDialog dlg(window());
    dlg.setParams(m_bindings,m_slaveChannels);

    if (dlg.exec() == QDialog::Rejected) return;

    QVector<int> slaveChannels;
    QVector<CMultiBinding> bindings;
    dlg.getParams(bindings,slaveChannels);
    regenerateOutputs(slaveChannels.count());
    regenerateCapacity(bindings.count());
    m_slaveChannels = slaveChannels;
    m_bindings = bindings;

    Q_EMIT componentChanged(this);
    update();
    resize(minimumSizeHint());
}

void ZCPMulti::regenerateOutputs(int outputsCount)
{
    while (fOutputs.count() > outputsCount) {
        deleteOutput(fOutputs.count()-1);
        m_slaveChannels.removeLast();
    }

    while (fOutputs.count() < outputsCount) {
        auto out=new ZCPOutput(this, QString());
        registerOutput(out);
        m_slaveChannels.append(2); // default stereo output
    }
    for (int i=0;i<fOutputs.count();i++) {
        fOutputs[i]->pinName = ZMultiDialog::formatOutputName(i);
    }

    for (int i=0;i<m_bindings.count();i++) {
        if (m_bindings.at(i).first >= fOutputs.count()) { // binding to deleted output
            m_bindings[i].first = 0; // change to first slave
            m_bindings[i].second = 0;
        }
    }

    Q_EMIT componentChanged(this);
    update();
}

void ZCPMulti::regenerateCapacity(int inputChannelsCount)
{
    while (m_bindings.count() > inputChannelsCount)
        m_bindings.removeLast();

    while (m_bindings.count() < inputChannelsCount) {
        int slaveIdx = 0;
        int slaveChannel = 0;
        m_bindings.append(qMakePair(slaveIdx,slaveChannel));
    }

    Q_EMIT componentChanged(this);
    update();
}

bool ZCPMulti::isConverterPresent() const
{
    return ((searchPluginBackward(ZCPPlug::staticMetaObject.className()) != nullptr) ||
            (searchPluginBackward(ZCPRoute::staticMetaObject.className()) != nullptr));
}
