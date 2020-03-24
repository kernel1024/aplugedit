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

#include <functional>
#include "includes/alsabackend.h"
#include "includes/generic.h"
#include "includes/renderarea.h"
#include "includes/mainwindow.h"
#include "includes/cpinp.h"
#include "includes/cphw.h"
#include "includes/cpnull.h"
#include "includes/cpfile.h"
#include "includes/cprate.h"
#include "includes/cproute.h"
#include "includes/cpshare.h"
#include "includes/cpmeter.h"
#include "includes/cpconv.h"
#include "includes/cpladspa.h"
#include "includes/cpplug.h"
#include "includes/cpupmix.h"
#include "includes/cpvdownmix.h"
#include "includes/cpmulti.h"
#include "includes/cpblacklist.h"

ZRenderArea::ZRenderArea(QScrollArea *aScroller)
    : QFrame(aScroller)
{
    m_scroller=aScroller;

    m_recycle = new QLabel(this);
    m_recycle->setGeometry(QRect(10, 10, 48, 48));
    m_recycle->clear();
    m_recycle->setPixmap(QPixmap(QSL(":/images/trashcan_empty.png")));
    m_recycle->setObjectName(QSL("trashcan"));
}

QSize ZRenderArea::minimumSizeHint() const
{
    int x=3*m_scroller->width()/2;
    int y=3*m_scroller->height()/2;
    QRect r(0,0,0,0);
    const auto wlist = findComponents<QWidget*>();
    for (const auto& w : wlist)
        r=r.united(w->geometry());
    QSize cmSize=QSize(r.width(),r.height());
    return QSize(x,y).expandedTo(cmSize);
}

QSize ZRenderArea::sizeHint() const
{
    return minimumSizeHint();
}

void ZRenderArea::initConnBuilder(int type, int pinNum, ZCPInput* input, ZCPOutput* output,
                                  ZCPBase *initialFilter)
{
    if (m_connBuilding) {
        m_connBuilding=false;
        return;
    }

    m_startPinType=type;
    m_startPinNum=pinNum;
    m_startInput=nullptr;
    m_startOutput=nullptr;
    m_startFilter=initialFilter;

    // init connection form input
    if (m_startPinType==ZCPBase::PinType::ptInput) {
        m_startInput=input;
        if (m_startInput) {
            // do not disconnect anything, just start building mode
            m_connCursor=m_startInput->ownerFilter->pos()+m_startInput->relCoord;
        }
    } else {
        // init connection from output
        m_startOutput=output;
        if (m_startOutput) {
            // disconnect old connections from this output
            if ((m_startOutput->toPin!=-1) && (m_startOutput->toFilter)) {
                m_startOutput->toFilter->fInputs.at(m_startOutput->toPin)
                        ->links.removeAll(CInpLink(pinNum,m_startFilter));
            }
            m_startOutput->toFilter=nullptr;
            m_startOutput->toPin=-1;
            m_connCursor=m_startOutput->ownerFilter->pos()+m_startOutput->relCoord;
        }
    }
    m_connBuilding=true;
}

void ZRenderArea::repaintConn()
{
    repaint();
}

void ZRenderArea::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event)
    QPainter p(this);
    p.save();

    paintConnections(&p);

    if (m_connBuilding) {
        p.setPen(QPen(Qt::darkCyan));
        QPoint c1;
        if (m_startPinType==ZCPBase::PinType::ptInput) {
            c1=m_startInput->ownerFilter->pos()+m_startInput->relCoord;
        } else {
            c1=m_startOutput->ownerFilter->pos()+m_startOutput->relCoord;
        }
        QPoint c2=m_connCursor;
        p.drawLine(c1,c2);
    }

    p.restore();
}

void ZRenderArea::drawArrowLine(QPainter *p, const QPoint &p1, const QPoint &p2,
                                bool invertDirection, bool arrowAtEnd)
{
    const int arrowSize = 10;

    QPointF pEnd = p2 - p1;
    QPointF np;

    p->save();
    QBrush b(p->pen().color(),Qt::BrushStyle::SolidPattern);
    p->setBrush(b);

    p->translate(p1);

    p->drawLine(np,pEnd);

    if (pEnd.manhattanLength() > (arrowSize*2)) {
        if (arrowAtEnd) {
            p->translate(pEnd);
        } else {
            p->translate(pEnd.x()/2.0,pEnd.y()/2.0);
        }

        auto angle = qRadiansToDegrees(qAtan2(pEnd.y(),pEnd.x())) + 180.0;
        if (invertDirection)
            angle += 180.0;
        p->rotate(angle);
        if (!arrowAtEnd)
            p->translate(QPoint(-1*arrowSize/2,0));

        QPolygon head({ QPoint(0,0) });
        head << QPoint(arrowSize,arrowSize/2);
        head << QPoint(2*arrowSize/3,0);
        head << QPoint(arrowSize,-1*arrowSize/2);
        p->drawPolygon(head);
    }

    p->restore();
}

void ZRenderArea::paintConnections(QPainter* p)
{
    const int pinSpace = 20;
    const int seqMarginStep = 4; // incremental margin for pin

    QVector<ZCPBase *> cmps;
    QVector<QRect> cmpsRect;

    QPen wirePen = QPen(Qt::red);
    QRect framingRect;

    m_connCount=0;

    // collect components geometry

    const auto cplist = findComponents<ZCPBase*>();
    cmps.reserve(cplist.count());
    cmpsRect.reserve(cplist.count());
    for (const auto& cp : cplist) {
        cmps << cp;
        cmpsRect << cp->geometry();
        framingRect = framingRect.united(cp->geometry());
    }

    int walkaroundCmps = 0;
    for (const auto &base : qAsConst(cmps)) {
        int outIdx = 0;
        for (const auto &outPin : qAsConst(base->fOutputs)) {
            if (outPin->toPin==-1) continue;
            if (outPin->toFilter==nullptr) continue;
            ZCPInput* inpPin=outPin->toFilter->fInputs.at(outPin->toPin);

            QPoint c1 = base->pos()+outPin->relCoord;
            QPoint c2 = outPin->toFilter->pos()+inpPin->relCoord;
            QRect linkRect = QRect(c1,c2);
            linkRect.adjust(zcpPinSize/2,0,-zcpPinSize/2-2,0);
            c1 = linkRect.topLeft();
            c2 = linkRect.bottomRight();
            linkRect = linkRect.normalized();

            bool selfIntersect = (linkRect.intersects(base->geometry()) ||
                                linkRect.intersects(outPin->toFilter->geometry()));
            // Check intersection between line framing rect and any components
            bool noIntersects = true;
            for (const auto &rect : qAsConst(cmpsRect)) {
                if (linkRect.intersects(rect))
                    noIntersects = false;
            }
            // If no intersections occurs - simply draw 3-part line
            if (noIntersects) {
                int seqMargin = seqMarginStep*outIdx;
                QPoint c3=QPoint(((c1.x()+c2.x())/2)+seqMargin,c1.y());
                QPoint c4=QPoint(c3.x(),c2.y());
                p->setPen(wirePen);
                p->drawLine(c1,c3);
                drawArrowLine(p,c3,c4);
                drawArrowLine(p,c4,c2,false,true);
            } else {
                int si = -1;
                if (selfIntersect)
                    si = 1;
                int margin = seqMarginStep*walkaroundCmps;
                // walkaround points
                QPoint p1; // near pins
                QPoint p2;
                QPoint m1; // near schematic border
                QPoint m2;
                p->setPen(wirePen);
                int dx = c2.x()-c1.x();
                int dy = c2.y()-c1.y();
                if ((dy>0 && dx<0) || (dy<0 && dx>0)) {
                    p1 = QPoint(linkRect.right()+si*(pinSpace+margin),linkRect.top());
                    p2 = QPoint(linkRect.left()-si*(pinSpace+margin),linkRect.bottom());
                    c1 = linkRect.topRight();
                    c2 = linkRect.bottomLeft();
                    m1 = QPoint(linkRect.right()+si*(pinSpace+margin),framingRect.bottom()+pinSpace+margin);
                    m2 = QPoint(linkRect.left()-si*(pinSpace+margin),framingRect.bottom()+pinSpace+margin);
                    p->drawLine(c1,p1);
                    drawArrowLine(p,p1,m1,(si<0));
                    drawArrowLine(p,m1,m2,(si<0));
                    drawArrowLine(p,m2,p2,(si<0));
                    p->drawLine(p2,c2);
                } else {
                    p1 = QPoint(linkRect.left()-si*(pinSpace+margin),linkRect.top());
                    p2 = QPoint(linkRect.right()+si*(pinSpace+margin),linkRect.bottom());
                    c1 = linkRect.topLeft();
                    c2 = linkRect.bottomRight();
                    m1 = QPoint(linkRect.left()-si*(pinSpace+margin),framingRect.bottom()+pinSpace+margin);
                    m2 = QPoint(linkRect.right()+si*(pinSpace+margin),framingRect.bottom()+pinSpace+margin);
                    p->setPen(wirePen);
                    p->drawLine(c1,p1);
                    drawArrowLine(p,p1,m1,(si>0));
                    drawArrowLine(p,m1,m2,(si>0));
                    drawArrowLine(p,m2,p2,(si>0));
                    p->drawLine(p2,c2);
                }
                walkaroundCmps++;
            }
            outIdx++;
            m_connCount++;
        }
    }
}

void ZRenderArea::refreshConnBuilder(const QPoint & atPos)
{
    if (!m_connBuilding) return;
    if (m_startPinType==ZCPBase::PinType::ptInput) {
        m_connCursor=QPoint(m_startInput->ownerFilter->pos()+atPos);
    } else {
        m_connCursor=QPoint(m_startOutput->ownerFilter->pos()+atPos);
    }
    repaintConn();
}

void ZRenderArea::doneConnBuilder(bool none, int type, int pinNum, ZCPInput* input, ZCPOutput* output, ZCPBase *finishFilter)
{
    // if we making trace from input to this output...
    if ((m_startPinType==ZCPBase::PinType::ptInput) && m_startInput) {
        // and we have new output now...
        if (output) {
            // then we remove old connection to this output to connect our new trace to it
            if ((output->toPin!=-1) && output->toFilter) {
                output->toFilter->fInputs.at(output->toPin)
                        ->links.removeAll(CInpLink(pinNum,finishFilter));
            }
            output->toFilter=nullptr;
            output->toPin=-1;
        }
    }
    // if we making trace from output to this input...
    else if (m_startOutput) {
        // if our output (from that we making connection) is connected - then disconnect it now
        if ((m_startOutput->toPin!=-1) && m_startOutput->toFilter) {
            m_startOutput->toFilter->fInputs.at(m_startOutput->toPin)
                    ->links.removeAll(CInpLink(m_startPinNum,m_startFilter));
        }
        m_startOutput->toFilter=nullptr;
        m_startOutput->toPin=-1;
    }
    // if this is simple deletion or incorrect route (in-in, out-out), then delete it
    if ((none) || (type==m_startPinType)) {
        m_connBuilding=false;
        repaintConn();
        return;
    }
    // if this output can't possible connect to specified input (np: DMix connecting not to HW), then delete it
    ZCPBase *toFilter;
    ZCPBase *fromFilter;
    if (m_startPinType==ZCPBase::PinType::ptInput) {
        toFilter=m_startInput->ownerFilter;
        fromFilter=output->ownerFilter;
    } else {
        toFilter=input->ownerFilter;
        fromFilter=m_startOutput->ownerFilter;
    }
    if ((!fromFilter->canConnectOut(toFilter)) || (!toFilter->canConnectIn(fromFilter))) {
        m_connBuilding=false;
        repaintConn();
        return;
    }
    if (m_startPinType==ZCPBase::PinType::ptInput) {
        m_startInput->links.append(CInpLink(pinNum,output->ownerFilter));
        output->toFilter=m_startInput->ownerFilter;
        output->toPin=m_startPinNum;
    } else {
        m_startOutput->toFilter=input->ownerFilter;
        m_startOutput->toPin=pinNum;
        input->links.append(CInpLink(m_startPinNum,m_startOutput->ownerFilter));
    }
    m_connBuilding=false;
    repaintConn();
}

bool ZRenderArea::postLoadBinding()
{
    const auto cplist = findComponents<ZCPBase*>();
    for (const auto& cp : cplist) {
        if (!(cp->postLoadBind()))
            return false;
    }
    return true;
}

void ZRenderArea::doGenerate(QTextStream & stream, QStringList & warnings)
{
    stream << "# This file is automatically generated in ALSA Plugin Editor v2." << endl
           << "# " << QDateTime::currentDateTime().toString(Qt::ISODate) << endl << endl;

    const auto cplist = findComponents<ZCPBase*>();
    for (const auto& cp : cplist) {
        cp->doInfoGenerate(stream,warnings);
    }
    
    stream << "# Generation successfully completted." << endl;
}

QVector<CPCMItem> ZRenderArea::getAllPCMNames() const
{
    QVector<CPCMItem> res;
    const auto cplist = findComponents<ZCPInp*>();
    res.reserve(cplist.count());
    for (const auto& cp : cplist) {
        res.append(CPCMItem(cp->dspName(),QStringList({cp->getHint()})));
    }
    return res;
}

int ZRenderArea::componentCount() const
{
    const auto cplist = findComponents<ZCPBase*>();
    return cplist.count();
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
    const auto cplist = findComponents<ZCPBase*>();
    for (const auto& base : cplist) {
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

void ZRenderArea::deleteComponents(const std::function<void()>& callback)
{
    const auto cplist = findComponents<ZCPBase*>();
    if (cplist.isEmpty()) {
        callback();
        return;
    }

    m_deletingComponents.storeRelease(cplist.count());
    for (const auto& base : cplist) {
        connect(base,&ZCPBase::destroyed,this,[this,callback](QObject* ptr){
            Q_UNUSED(ptr)
            if ((--m_deletingComponents)<=0) {
                callback();
            }
        });
        base->deleteComponent();
    }
}

ZCPBase* ZRenderArea::createCpInstance(const QString& className, const QPoint& pos, const QString& objectName)
{
    if (m_deletingComponents.loadAcquire()>0) return nullptr;

    ZCPBase* res = nullptr;
    QString name = className;
    if (name.startsWith(QSL("QCP")))
        name.replace(0,1,QChar('Z'));
    if (name==QSL("ZCPDMix")) name = QSL("ZCPShare");
    if (name==QSL("ZCPInp")) res = new ZCPInp(this,this);
    else if (name==QSL("ZCPHW")) res = new ZCPHW(this,this);
    else if (name==QSL("ZCPNull")) res = new ZCPNull(this,this);
    else if (name==QSL("ZCPFile")) res = new ZCPFile(this,this);
    else if (name==QSL("ZCPRate")) res = new ZCPRate(this,this);
    else if (name==QSL("ZCPRoute")) res = new ZCPRoute(this,this);
    else if (name==QSL("ZCPShare")) res = new ZCPShare(this,this);
    else if (name==QSL("ZCPMeter")) res = new ZCPMeter(this,this);
    else if (name==QSL("ZCPConv")) res = new ZCPConv(this,this);
    else if (name==QSL("ZCPLADSPA")) res = new ZCPLADSPA(this,this);
    else if (name==QSL("ZCPPlug")) res = new ZCPPlug(this,this);
    else if (name==QSL("ZCPUpmix")) res = new ZCPUpmix(this,this);
    else if (name==QSL("ZCPVDownmix")) res = new ZCPVDownmix(this,this);
    else if (name==QSL("ZCPMulti")) res = new ZCPMulti(this,this);
    else if (name==QSL("ZCPBlacklist")) res = new ZCPBlacklist(this,this);

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
