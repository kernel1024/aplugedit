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

#include <QRandomGenerator>
#include "includes/generic.h"
#include "includes/cpshare.h"
#include "includes/cphw.h"
#include "includes/sharedialog.h"

ZCPShare::ZCPShare(QWidget *parent, ZRenderArea *aOwner, ZCPShare::SharePlugin mode)
    : ZCPBase(parent,aOwner)
    , m_mode(mode)
{
    fInp=new ZCPInput(this,this);
    fInp->pinName=QSL("in");
    registerInput(fInp);
    fOut=new ZCPOutput(this,this);
    fOut->pinName=QSL("out");
    registerOutput(fOut);

    m_IPCkey = QSL("%1").arg(QRandomGenerator::global()->bounded(1024,INT_MAX));
    m_IPCpermissions = QSL("0660");
}

void ZCPShare::setPluginMode(ZCPShare::SharePlugin mode)
{
    m_mode = mode;

    update();
    Q_EMIT componentChanged(this);
}

void ZCPShare::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));

    QString mode = json.toObject().value(QSL("mode")).toString().toLower();
    if (mode == QSL("dsnoop")) {
        m_mode = spDSnoop;
    } else if (mode == QSL("dshare")) {
        m_mode = spDShare;
    } else {
        m_mode = spDMix;
    }

    QString slowPtr = json.toObject().value(QSL("slowPtr")).toString().toLower();
    if (slowPtr == QSL("yes")) {
        m_slowPtr = Qt::CheckState::Checked;
    } else if (slowPtr == QSL("no")) {
        m_slowPtr = Qt::CheckState::Unchecked;
    } else {
        m_slowPtr = Qt::CheckState::PartiallyChecked;
    }

    QString ptrAlignment = json.toObject().value(QSL("ptrAlignment")).toString().toLower();
    if (ptrAlignment == QSL("auto")) {
        m_hwPtrAlignment = haAuto;
    } else if (ptrAlignment == QSL("no")) {
        m_hwPtrAlignment = haNo;
    } else if (ptrAlignment == QSL("roundup")) {
        m_hwPtrAlignment = haRoundUp;
    } else if (ptrAlignment == QSL("rounddown")) {
        m_hwPtrAlignment = haRoundDown;
    } else {
        m_hwPtrAlignment = haEmpty;
    }

    m_IPCkey = json.toObject().value(QSL("IPCkey")).toString();
    m_IPCpermissions = json.toObject().value(QSL("IPCperm")).toString();

    const QJsonArray jbindings = json.toObject().value(QSL("bindings")).toArray();
    int idx = 0;
    for (const auto& i : jbindings)
        m_bindings.append(i.toInt(idx++));
}

QJsonValue ZCPShare::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());

    switch (m_mode) {
        case spDMix: data.insert(QSL("mode"), QSL("dmix")); break;
        case spDSnoop: data.insert(QSL("mode"), QSL("dsnoop")); break;
        case spDShare: data.insert(QSL("mode"), QSL("dshare")); break;
    }
    switch (m_slowPtr) {
        case Qt::CheckState::Checked: data.insert(QSL("slowPtr"), QSL("yes")); break;
        case Qt::CheckState::Unchecked: data.insert(QSL("slowPtr"), QSL("no")); break;
        case Qt::CheckState::PartiallyChecked: data.insert(QSL("slowPtr"), QSL("default")); break;
    }
    switch (m_hwPtrAlignment) {
        case haAuto: data.insert(QSL("ptrAlignment"), QSL("auto")); break;
        case haNo: data.insert(QSL("ptrAlignment"), QSL("no")); break;
        case haRoundUp: data.insert(QSL("ptrAlignment"), QSL("roundup")); break;
        case haRoundDown: data.insert(QSL("ptrAlignment"), QSL("rounddown")); break;
        default: break;
    }

    data.insert(QSL("IPCkey"), m_IPCkey);
    data.insert(QSL("IPCperm"), m_IPCpermissions);

    QJsonArray jbindings;
    for (const auto& i : qAsConst(m_bindings))
        jbindings.append(i);
    data.insert(QSL("bindings"),jbindings);

    return data;
}

ZCPShare::~ZCPShare() = default;

QSize ZCPShare::minimumSizeHint() const
{
    return QSize(180,50);
}

void ZCPShare::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/2);
}

void ZCPShare::doInfoGenerate(QTextStream & stream, QStringList &warnings) const
{
    QString type;
    switch (m_mode) {
        case spDMix: type = QSL("dmix"); break;
        case spDSnoop: type = QSL("dsnoop"); break;
        case spDShare: type = QSL("dshare"); break;
    }
    stream << QSL("pcm.%1 {").arg(objectName()) << endl;
    stream << QSL("  type %1").arg(type) << endl;
    stream << QSL("  ipc_key %1").arg(m_IPCkey) << endl;
    if (!m_IPCpermissions.isEmpty())
        stream << QSL("  ipc_perm %1").arg(m_IPCpermissions) << endl;

    QString ptr;
    switch (m_hwPtrAlignment) {
        case haAuto: ptr = QSL("auto"); break;
        case haNo: ptr = QSL("no"); break;
        case haRoundUp: ptr = QSL("roundup"); break;
        case haRoundDown: ptr = QSL("rounddown"); break;
        default: break;
    }
    if (!ptr.isEmpty())
        stream << QSL("  hw_ptr_alignment %1").arg(ptr) << endl;

    if (fOut->toFilter) {
        stream << QSL("  slave {") << endl;
        stream << QSL("    pcm \"") << fOut->toFilter->objectName() << QSL("\"") << endl;
        auto hw=qobject_cast<ZCPHW*>(fOut->toFilter);
        if (hw) {
            if (hw->getChannels()!=-1)
                stream << QSL("    channels ") << hw->getChannels() << endl;
            if (hw->getRate()!=-1)
                stream << QSL("    rate ") << hw->getRate() << endl;
        }
        stream << QSL("  }") << endl;
    }
    for (int i=0;i<m_bindings.count();i++) {
        stream << QSL("  bindings.%1 %2").arg(i).arg(m_bindings.at(i)) << endl;
    }
    if (m_slowPtr == Qt::CheckState::Checked) {
        stream << QSL("  slowptr true") << endl;
    } else if (m_slowPtr == Qt::CheckState::Unchecked) {
        stream << QSL("  slowptr false") << endl;
    }

    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << endl;
    stream << endl;
}

void ZCPShare::showSettingsDlg()
{
    ZShareDialog dlg(window());

    QStringList ipcList;
    const auto cplist = ownerArea()->findComponents<ZCPShare *>();
    for (const auto& cp : cplist) {
        if (cp->m_mode == spDShare)
            ipcList.append(cp->m_IPCkey);
    }

    dlg.setBindings(m_bindings);
    dlg.setIPC((m_mode == spDShare),m_IPCkey,m_IPCpermissions,ipcList);
    dlg.setPtrParams(m_slowPtr,m_hwPtrAlignment);

    if (dlg.exec() == QDialog::Rejected) return;

    m_bindings = dlg.getBindigs();
    dlg.getIPC(m_IPCkey,m_IPCpermissions);
    dlg.getPtrParams(m_slowPtr,m_hwPtrAlignment);

    update();
    Q_EMIT componentChanged(this);
}

bool ZCPShare::canConnectOut(ZCPBase * toFilter)
{
    const auto toHW=qobject_cast<ZCPHW*>(toFilter);
    if (m_mode == spDMix)
        return (toHW!=nullptr);

    return true;
}

void ZCPShare::paintEvent (QPaintEvent * event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    paintBase(p);

    QString type;
    switch (m_mode) {
        case spDMix: type = QSL("DMix"); break;
        case spDSnoop: type = QSL("DSnoop"); break;
        case spDShare: type = QSL("DShare"); break;
    }

    setBaseFont(p,ftTitle);
    p.drawText(rect(),Qt::AlignCenter,type);

    QString desc;
    if (!m_bindings.isEmpty())
        desc = tr("%1 ch").arg(m_bindings.count());
    if (m_mode == spDShare) {
        if (!desc.isEmpty())
            desc.append(QSL(", "));
        desc.append(QSL("IPC: %1").arg(m_IPCkey));
    }

    setBaseFont(p,ftDesc);
    p.drawText(QRect(0,2*height()/3,width(),height()/3),Qt::AlignCenter,desc);

    p.restore();
}
