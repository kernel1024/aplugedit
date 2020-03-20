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

#include "includes/cpblacklist.h"
#include "includes/generic.h"
#include "includes/blacklistdialog.h"

ZCPBlacklist::ZCPBlacklist(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent, aOwner)
{
}

ZCPBlacklist::~ZCPBlacklist() = default;

QSize ZCPBlacklist::minimumSizeHint() const
{
    return QSize(140,50);
}

void ZCPBlacklist::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    paintBase(p);

    setBaseFont(p,ftTitle);
    p.drawText(rect(),Qt::AlignCenter,QSL("Blacklist"));

    setBaseFont(p,ftDesc);
    p.drawText(QRect(0,2*height()/3,width(),height()/3),Qt::AlignCenter,
               tr("%1 PCMs").arg(m_PCMs.count()));

    p.restore();
}

void ZCPBlacklist::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));

    const QJsonArray jpcms = json.toObject().value(QSL("pcms")).toArray();
    for (const auto& jpcm : jpcms) {
        QString name = jpcm.toObject().value(QSL("name")).toString();
        QStringList desc;
        const QJsonArray jpcmdesc = jpcm.toObject().value(QSL("desc")).toArray();
        desc.reserve(jpcmdesc.count());
        for (const auto& js : jpcmdesc) {
            desc.append(js.toString());
        }
        if (!name.isEmpty())
            m_PCMs.append(CPCMItem(name,desc));
    }
}

QJsonValue ZCPBlacklist::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());

    QJsonArray jpcms;
    for (const auto &pcm : qAsConst(m_PCMs)) {
        QJsonObject jpcm;
        jpcm.insert(QSL("name"),pcm.name);

        QJsonArray jpcmdesc;
        for (const auto &desc : qAsConst(pcm.description)) {
            jpcmdesc.append(desc);
        }
        jpcm.insert(QSL("desc"),jpcmdesc);

        jpcms.append(jpcm);
    }
    data.insert(QSL("pcms"),jpcms);

    return data;
}

void ZCPBlacklist::doInfoGenerate(QTextStream &stream, QStringList &warnings) const
{
    Q_UNUSED(warnings)

    QStringList pcms;
    pcms.reserve(m_PCMs.count());
    for (const auto &pcm : qAsConst(m_PCMs)) {
        QString name = pcm.name;
        int scpos = name.indexOf(QChar(':'));
        if (scpos >= 0) name.truncate(scpos);

        if (!name.isEmpty() && !pcms.contains(name)) {
            pcms.append(name);

            stream << QSL("pcm.!%1 0").arg(name) << endl;
        }
    }
    stream << endl;
}

void ZCPBlacklist::showSettingsDlg()
{
    ZBlacklistDialog dlg(window());

    dlg.setBlacklist(m_PCMs);

    if (dlg.exec() == QDialog::Rejected) return;

    m_PCMs = dlg.getBlacklist();

    Q_EMIT componentChanged(this);
    update();
}
