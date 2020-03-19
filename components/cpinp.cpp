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
    fOut=new ZCPOutput(this,this);
    fOut->pinName=QSL("out");
    registerOutput(fOut);
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

    bool ok;
    QString item = QInputDialog::getItem(this, tr("DSP input device"),
                                         tr("User-land device number"), items, c, true, &ok);
    if (ok && !item.isEmpty())
    {
        m_dspName=item;
        for (int i=0;i<ownerArea()->children().count();i++) {
            if (auto base=qobject_cast<ZCPInp*>(ownerArea()->children().at(i))) {
                if ((base->objectName()!=objectName()) && (base->m_dspName==m_dspName))
                {
                    QMessageBox::warning(topLevelWidget(),tr("Duplicated DSP"),
                                         tr("You have entered duplicated identifier for this DSP input,\n"
                                            "that is already used in another component.\n"
                                            "Please, recheck your DSP inputs!"));
                    break;
                }
            }
        }
        update();
        Q_EMIT componentChanged(this);
    }
}

void ZCPInp::readFromStreamLegacy( QDataStream & stream )
{
    ZCPBase::readFromStreamLegacy(stream);
    stream >> m_dspName;
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
    return QSize(180,50);
}

void ZCPInp::realignPins()
{
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/2);
}

void ZCPInp::doInfoGenerate(QTextStream & stream, QStringList &warnings) const
{
    stream << QSL("pcm.") << m_dspName << QSL(" {") << endl;
    if (fOut->toFilter)
    {
        stream << QSL("  type plug") << endl;
        stream << QSL("  slave.pcm \"") << fOut->toFilter->objectName() << QSL("\"") << endl;
    } else {
        stream << QSL("  type hw") << endl;
        stream << QSL("  card 0") << endl;
    }
    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << endl;
    stream << endl;
}

void ZCPInp::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    paintBase(p);

    setBaseFont(p,ftTitle);
    p.drawText(rect(),Qt::AlignCenter,QSL("Input DSP"));

    setBaseFont(p,ftDesc);
    p.drawText(QRect(0,2*height()/3,width(),height()/3),Qt::AlignCenter,m_dspName);

    p.restore();
}
