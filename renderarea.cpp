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

#include "includes/renderarea.h"
#include "includes/mainwindow.h"
#include "includes/cpinp.h"
#include "includes/cphw.h"
#include "includes/cpnull.h"
#include "includes/cpfile.h"
#include "includes/cprate.h"
#include "includes/cproute.h"
#include "includes/cpdmix.h"
#include "includes/cpmeter.h"
#include "includes/cpconv.h"
#include "includes/cpladspa.h"

ZRenderArea::ZRenderArea(QScrollArea *aScroller)
    : QFrame(aScroller)
{
    scroller=aScroller;

    recycle.reset(new QLabel(this));
    recycle->setGeometry(QRect(10, 10, 48, 48));
    recycle->clear();//setText("");
    recycle->setPixmap(QPixmap(QSL(":/images/trashcan_empty.png")));
    recycle->setObjectName(QSL("trashcan"));
}

QSize ZRenderArea::minimumSizeHint() const
{
    int x=3*scroller->width()/2;
    int y=3*scroller->height()/2;
    QRect r(0,0,0,0);
    for (int i=0;i<children().count();i++) {
        if (QWidget* w=qobject_cast<QWidget*>(children().at(i)))
            r=r.united(w->geometry());
    }
    QSize cmSize=QSize(r.width(),r.height());
    return QSize(x,y).expandedTo(cmSize);
}

QSize ZRenderArea::sizeHint() const
{
    return minimumSizeHint();
}

void ZRenderArea::initConnBuilder(int aType, int aPinNum, ZCPInput* aInput, ZCPOutput* aOutput)
{
    if (cbBuilding) {
        cbBuilding=false;
        return;
    }

    cbType=aType;
    cbPinNum=aPinNum;
    cbInput=nullptr;
    cbOutput=nullptr;
    // init connection form input
    if (cbType==ZCPBase::PinType::ptInput) {
        cbInput=aInput;
        if (cbInput) {
            // disconnect old connections from this input
            if ((cbInput->fromPin!=-1) && (cbInput->fromFilter)) {
                cbInput->fromFilter->fOutputs[cbInput->fromPin]->toFilter=nullptr;
                cbInput->fromFilter->fOutputs[cbInput->fromPin]->toPin=-1;
            }
            cbInput->fromFilter=nullptr;
            cbInput->fromPin=-1;
            cbCurrent=cbInput->ownerFilter->pos()+cbInput->relCoord;
        }
    } else {
        // init connection from output
        cbOutput=aOutput;
        if (cbOutput) {
            // disconnect old connections from this output
            if ((cbOutput->toPin!=-1) && (cbOutput->toFilter)) {
                cbOutput->toFilter->fInputs[cbOutput->toPin]->fromFilter=nullptr;
                cbOutput->toFilter->fInputs[cbOutput->toPin]->fromPin=-1;
            }
            cbOutput->toFilter=nullptr;
            cbOutput->toPin=-1;
            cbCurrent=cbOutput->ownerFilter->pos()+cbOutput->relCoord;
        }
    }
    cbBuilding=true;
}

void ZRenderArea::repaintConn()
{
    repaint();
}

void ZRenderArea::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event)
    QPainter p(this);
    cbConnCount=0;
    QPoint c1;
    QPoint c2;
    QPoint c3;
    QPoint c4;

    QPen op=p.pen();
    p.setPen(QPen(Qt::red));
    cbConnCount=0;
    for (const auto &a : children()) {
        auto base=qobject_cast<ZCPBase*>(a);
        if (base==nullptr) continue;
        for (const auto &pin : qAsConst(base->fOutputs)) {
            if (pin->toPin==-1) continue;
            if (pin->toFilter==nullptr) continue;
            ZCPInput* inp=pin->toFilter->fInputs.at(pin->toPin);

            c1=base->pos()+pin->relCoord;
            c2=pin->toFilter->pos()+inp->relCoord;
            c3=QPoint((c1.x()+c2.x())/2,c1.y());
            c4=QPoint(c3.x(),c2.y());

            if (rectLinks) {
                p.drawLine(c1,c3);
                p.drawLine(c3,c4);
                p.drawLine(c4,c2);
            } else {
                p.drawLine(c1,c2);
            }

            cbConnCount++;
        }
    }
    if (cbBuilding) {
        p.setPen(QPen(Qt::darkCyan));
        if (cbType==ZCPBase::PinType::ptInput) {
            c1=cbInput->ownerFilter->pos()+cbInput->relCoord;
        } else {
            c1=cbOutput->ownerFilter->pos()+cbOutput->relCoord;
        }
        c2=cbCurrent;
        p.drawLine(c1,c2);
    }
    p.setPen(op);
}


void ZRenderArea::refreshConnBuilder(const QPoint & atPos)
{
    if (!cbBuilding) return;
    if (cbType==ZCPBase::PinType::ptInput) {
        cbCurrent=QPoint(cbInput->ownerFilter->pos()+atPos);
    } else {
        cbCurrent=QPoint(cbOutput->ownerFilter->pos()+atPos);
    }
    repaintConn();
}

void ZRenderArea::doneConnBuilder(bool aNone, int aType, int aPinNum, ZCPInput* aInput, ZCPOutput* aOutput)
{
    // if we making trace from input to this output...
    if ((cbType==ZCPBase::PinType::ptInput) && cbInput) {
        // and we have new output now...
        if (aOutput) {
            // then we remove old connection to this output to connect our new trace to it
            if ((aOutput->toPin!=-1) && aOutput->toFilter) {
                aOutput->toFilter->fInputs[aOutput->toPin]->fromFilter=nullptr;
                aOutput->toFilter->fInputs[aOutput->toPin]->fromPin=-1;
            }
            aOutput->toFilter=nullptr;
            aOutput->toPin=-1;
        }
        // if our input (from that we making connection) is connected - then disconnect it now
        if ((cbInput->fromPin!=-1) && cbInput->fromFilter) {
            cbInput->fromFilter->fOutputs[cbInput->fromPin]->toFilter=nullptr;
            cbInput->fromFilter->fOutputs[cbInput->fromPin]->toPin=-1;
        }
        cbInput->fromFilter=nullptr;
        cbInput->fromPin=-1;
    }
    // if we making trace from output to this input...
    else if (cbOutput) {
        // and we have new input now...
        if (aInput) {
            // then we remove old connection to this input
            if ((aInput->fromPin!=-1) && aInput->fromFilter) {
                aInput->fromFilter->fOutputs[aInput->fromPin]->toFilter=nullptr;
                aInput->fromFilter->fOutputs[aInput->fromPin]->toPin=-1;
            }
            aInput->fromFilter=nullptr;
            aInput->fromPin=-1;
        }
        // if our output (from that we making connection) is connected - then disconnect it now
        if ((cbOutput->toPin!=-1) && cbOutput->toFilter) {
            cbOutput->toFilter->fInputs[cbOutput->toPin]->fromFilter=nullptr;
            cbOutput->toFilter->fInputs[cbOutput->toPin]->fromPin=-1;
        }
        cbOutput->toFilter=nullptr;
        cbOutput->toPin=-1;
    }
    // if this is simple deletion or incorrect route (in-in, out-out), then delete it
    if ((aNone) || (aType==cbType)) {
        cbBuilding=false;
        repaintConn();
        return;
    }
    // if this output can't possible connect to specified input (np: DMix connecting not to HW), then delete it
    ZCPBase *aTo;
    ZCPBase *aFrom;
    if (cbType==ZCPBase::PinType::ptInput) {
        aTo=cbInput->ownerFilter;
        aFrom=aOutput->ownerFilter;
    } else {
        aTo=aInput->ownerFilter;
        aFrom=cbOutput->ownerFilter;
    }
    if ((!aFrom->canConnectOut(aTo)) || (!aTo->canConnectIn(aFrom))) {
        cbBuilding=false;
        repaintConn();
        return;
    }
    if (cbType==ZCPBase::PinType::ptInput) {
        cbInput->fromFilter=aOutput->ownerFilter;
        cbInput->fromPin=aPinNum;
        aOutput->toFilter=cbInput->ownerFilter;
        aOutput->toPin=cbPinNum;
    } else {
        cbOutput->toFilter=aInput->ownerFilter;
        cbOutput->toPin=aPinNum;
        aInput->fromFilter=cbOutput->ownerFilter;
        aInput->fromPin=cbPinNum;
    }
    cbBuilding=false;
    repaintConn();
}

bool ZRenderArea::postLoadBinding()
{
    for (const auto &a : children()) {
        auto base = qobject_cast<ZCPBase*>(a);
        if (base) {
            if (!(base->postLoadBind()))
                return false;
        }
    }
    return true;
}

void ZRenderArea::doGenerate(QTextStream & stream)
{
    stream << "# This file is automatically generated in ALSA Plugin Editor v2." << endl
           << "# Generated: " << QDateTime::currentDateTime().toString(Qt::ISODate) << endl << endl;

    // Generate plugin tree for all DSP inputs
    for (const auto &a : children()) {
        if (auto base = qobject_cast<ZCPInp*>(a)) {
            stream << "# plugin tree for " << base->objectName() << endl;
            base->doGenerate(stream);
        }
    }
    
    stream << "# Generation successfully completted." << endl;
}

int ZRenderArea::componentCount() const
{
    int c=0;
    for (const auto &a : children())
        if (qobject_cast<ZCPBase*>(a) != nullptr) c++;
    return c;
}

bool ZRenderArea::readSchematicLegacy(QDataStream & stream)
{
    int c;
    stream >> c;
    for (int i=0;i<c;i++)
    {
        QString clName;
        QPoint pos;
        QString instName;
        stream >> clName;
        stream >> pos;
        stream >> instName;
        ZCPBase* cp = createCpInstance(clName,pos,instName);
        if (cp==nullptr) {
            QMessageBox::warning(this,tr("File load error"),tr("Unable to find class \"%1\".").arg(clName));
            return false;
        }
        cp->readFromStreamLegacy(stream);
        cp->show();
    }
    postLoadBinding();
    repaintConn();
    return true;
}

bool ZRenderArea::readSchematic(const QByteArray &json)
{
    QJsonParseError jerr {};
    QJsonDocument doc = QJsonDocument::fromJson(json,&jerr);
    if (doc.isNull()) {
        QMessageBox::warning(this,tr("File load error"),tr("Unable to parse JSON.\n%1").arg(jerr.errorString()));
        return false;
    }

    const QJsonArray components = doc.object().value(QSL("components")).toArray();
    if (components.isEmpty()) {
        QMessageBox::warning(this,tr("File load error"),tr("Empty, incorrect or incompatible JSON data."));
        return false;
    }
    bool warn = false;
    for (const auto &item : components) {
        QString iClassName = item.toObject().value(QSL("className")).toString();
        QString iName = item.toObject().value(QSL("name")).toString();
        if (iClassName.isEmpty() || iName.isEmpty()) {
            qWarning() << "Incorrect JSON or empty component";
            warn = true;
            continue;
        }

        QJsonObject jpos = item.toObject().value(QSL("position")).toObject();
        QPoint iPos(jpos.value(QSL("x")).toInt(200),
                    jpos.value(QSL("y")).toInt(200));

        ZCPBase* cp = createCpInstance(iClassName,iPos,iName);
        if (cp==nullptr) {
            QMessageBox::warning(this,tr("File load error"),tr("Unable to find class \"%1\".").arg(iClassName));
            return false;
        }
        cp->readFromJson(item.toObject().value(QSL("data")));
        cp->show();
    }
    if (warn) {
        QMessageBox::warning(this,tr("File load warning"),
                             tr("Empty, incorrect or incompatible component found in file."));
    }

    postLoadBinding();
    repaintConn();
    return true;
}

QByteArray ZRenderArea::storeSchematic() const
{
    QJsonObject root;

    QJsonArray components;
    int cCnt = 0;
    for (int i=0;i<children().count();i++)
    {
        auto base = qobject_cast<ZCPBase*>(children().at(i));
        if (base==nullptr) continue;

        QJsonObject item;
        item.insert(QSL("className"),base->metaObject()->className());
        item.insert(QSL("name"),base->objectName());

        QJsonObject position;
        QPoint mpos = base->pos();
        position.insert(QSL("x"),mpos.x());
        position.insert(QSL("y"),mpos.y());
        item.insert(QSL("position"),position);

        item.insert(QSL("data"),base->storeToJson());

        components.append(item);
        cCnt++;
    }
    root.insert(QSL("componentCount"),cCnt);
    root.insert(QSL("components"),components);

    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Indented);
}

void ZRenderArea::deleteComponents()
{
    for (const auto &a : children())
    {
        auto base = qobject_cast<ZCPBase*>(a);
        if (base)
            base->deleteComponent();
    }
}

ZCPBase* ZRenderArea::createCpInstance(const QString& className, const QPoint& pos, const QString& objectName)
{
    ZCPBase* res = nullptr;
    QString name = className;
    if (name.startsWith(QSL("QCP")))
        name.replace(0,1,QChar('Z'));
    if (name==QSL("ZCPInp")) res = new ZCPInp(this,this);
    else if (name==QSL("ZCPHW")) res = new ZCPHW(this,this);
    else if (name==QSL("ZCPNull")) res = new ZCPNull(this,this);
    else if (name==QSL("ZCPFile")) res = new ZCPFile(this,this);
    else if (name==QSL("ZCPRate")) res = new ZCPRate(this,this);
    else if (name==QSL("ZCPRoute")) res = new ZCPRoute(this,this);
    else if (name==QSL("ZCPDMix")) res = new ZCPDMix(this,this);
    else if (name==QSL("ZCPMeter")) res = new ZCPMeter(this,this);
    else if (name==QSL("ZCPConv")) res = new ZCPConv(this,this);
    else if (name==QSL("ZCPLADSPA")) res = new ZCPLADSPA(this,this);

    if (res==nullptr) {
        qCritical() << "Unable to create component " << className;
        return res;
    }

    if (!pos.isNull())
        res->move(pos);

    if (!objectName.isEmpty())
        res->setObjectName(objectName);

    return res;
}
