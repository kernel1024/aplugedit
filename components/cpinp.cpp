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
#include "includes/renderarea.h"
#include "includes/cpinp.h"

QString ZCPInp::dspName() const
{
    return m_dspName;
}

ZCPInp::ZCPInp(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fOut=new ZCPOutput(this, QSL("out"));
    registerOutput(fOut);

    fCtlOut=new ZCPOutput(this, QSL("ctl"),CStructures::PinClass::pcCTL);
    registerOutput(fCtlOut);

    m_dspName=QSL("!default");
}

ZCPInp::~ZCPInp() = default;

void ZCPInp::showSettingsDlg()
{
    QStringList items { QSL("!default") };
    items.reserve(8);
    for (int i=0;i<8;i++)
        items << QSL("dsp%1").arg(i);

    int c=0;
    if (!(items.contains(m_dspName))) {
        items << m_dspName;
        c=items.count()-1;
    } else {
        c=items.indexOf(m_dspName);
    }

    bool ok = false;
    QString item = QInputDialog::getItem(window(), tr("PCM device"),
                                         tr("Unique ALSA PCM ID number"), items, c, true, &ok);
    if (ok && !item.isEmpty())
    {
        m_dspName=item;
        const auto cplist = ownerArea()->findComponents<ZCPInp*>();
        for (const auto &cp : cplist) {
            if ((cp->objectName()!=objectName()) && (cp->m_dspName==m_dspName))
            {
                QMessageBox::warning(window(),tr("Duplicated ID"),
                                     tr("You have entered duplicated identifier for this Plug PCM device,\n"
                                        "that is already used in another component.\n"
                                        "Please, recheck your Plug PCM IDs!"));
                break;
            }
        }
        update();
        Q_EMIT componentChanged(this);
    }
}

void ZCPInp::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));
    m_dspName = json.toObject().value(QSL("dspName")).toString();
}

QJsonValue ZCPInp::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());
    data.insert(QSL("dspName"),m_dspName);
    return data;
}

QSize ZCPInp::minimumSizeHint() const
{
    return QSize(180,65);
}

void ZCPInp::realignPins()
{
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/3);
    fCtlOut->relCoord=QPoint(width()-zcpPinSize/2,2*height()/3);
}

void ZCPInp::doInfoGenerate(QTextStream & stream, QStringList &warnings) const
{
    stream << QSL("pcm.") << m_dspName << QSL(" {") << Qt::endl;
    if (fOut->toFilter)
    {
        stream << QSL("  type plug") << Qt::endl;
        stream << QSL("  slave.pcm \"") << fOut->toFilter->objectName() << QSL("\"") << Qt::endl;
    } else {
        stream << QSL("  type hw") << Qt::endl;
        stream << QSL("  card 0") << Qt::endl;
    }
    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << Qt::endl;
    stream << Qt::endl;
    if (fCtlOut->toFilter) {
        stream << QSL("ctl.") << m_dspName << QSL(" {") << Qt::endl;
        fCtlOut->toFilter->doCtlGenerate(stream,warnings);
        stream << QSL("}") << Qt::endl;
        stream << Qt::endl;
    }
}

void ZCPInp::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    paintBase(p);

    setBaseFont(p,ftTitle);
    p.drawText(rect(),Qt::AlignCenter,QSL("PCM device"));

    setBaseFont(p,ftDesc);
    p.drawText(QRect(0,2*height()/3,width(),height()/3),Qt::AlignCenter,m_dspName);

    p.restore();
}
