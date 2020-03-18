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
#include "includes/cpfile.h"

ZCPFile::ZCPFile(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this,this);
    fInp->pinName=QSL("in");
    registerInput(fInp);
    m_fileName=QSL("unnamed");
}

ZCPFile::~ZCPFile() = default;

QSize ZCPFile::minimumSizeHint() const
{
    return QSize(180,50);
}

void ZCPFile::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
}

void ZCPFile::doInfoGenerate(QTextStream & stream, QStringList &warnings) const
{
    stream << QSL("pcm.") << objectName() << QSL(" {") << endl;
    stream << QSL("  type file") << endl;
    stream << QSL("  slave.pcm null") << endl;
    stream << QSL("  file \"") << m_fileName << QSL("\"") << endl;
    stream << QSL("  format \"raw\"") << endl;
    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << endl;
    stream << endl;
}

void ZCPFile::paintEvent (QPaintEvent * event)
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
    p.drawText(rect(),Qt::AlignCenter,QSL("File Writer"));

    n.setBold(false);
    n.setPointSize(n.pointSize()-3);
    p.setPen(QPen(Qt::gray));
    p.setFont(n);
    QFileInfo fi(m_fileName);
    p.drawText(QRect(0,2*height()/3,width(),height()/3),Qt::AlignCenter,fi.fileName());

    p.setFont(of);
    p.setBrush(ob);
    p.setPen(op);
}

void ZCPFile::readFromStreamLegacy( QDataStream & stream )
{
    ZCPBase::readFromStreamLegacy(stream);
    stream >> m_fileName;
}

void ZCPFile::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));
    m_fileName = json.toObject().value(QSL("fileName")).toString();
}

QJsonValue ZCPFile::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());
    data.insert(QSL("fileName"),m_fileName);

    return data;
}

void ZCPFile::showSettingsDlg()
{
    QFileDialog d(topLevelWidget(),tr("Choose a filename to save stream under"),QString(),
                  tr("RAW file [*.raw] (*.raw)"));
    d.setDefaultSuffix(QSL("raw"));
    d.setAcceptMode(QFileDialog::AcceptSave);
    if (d.exec()==QDialog::Rejected) return;
    const auto selectedFiles = d.selectedFiles();
    if (selectedFiles.isEmpty()) return;
    m_fileName=selectedFiles.first();
    update();
    Q_EMIT componentChanged(this);
}

