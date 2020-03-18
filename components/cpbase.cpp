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

#include <cstring>
#include "includes/generic.h"
#include "includes/renderarea.h"
#include "includes/cpbase.h"
#include "ui_hintdlg.h"

ZCPBase::ZCPBase(QWidget *parent, ZRenderArea *aOwner)
    : QWidget(parent)
{
    m_owner=aOwner;
}

bool ZCPBase::canConnectOut(ZCPBase *toFilter)
{
    Q_UNUSED(toFilter)
    return true;
}

bool ZCPBase::canConnectIn(ZCPBase *toFilter)
{
    Q_UNUSED(toFilter)
    return true;
}

void ZCPBase::deleteOutput(int idx)
{
    for (int i=0;i<fOutputs.count();i++) {
        if ((fOutputs.at(i)->toPin!=-1) && (fOutputs.at(i)->toFilter)) {
            if (i==idx) { // disconnect specified output from input
                fOutputs.at(i)->toFilter->fInputs.at(fOutputs.at(i)->toPin)
                        ->links.removeAll(CInpLink(i,this));
                fOutputs.at(i)->toFilter=nullptr;
                fOutputs.at(i)->toPin=-1;
            } else if (i>idx) { // shift pin number
                int lnk = fOutputs.at(i)->toFilter->fInputs.at(fOutputs.at(i)->toPin)
                          ->links.indexOf(CInpLink(i,this));
                if (lnk>=0)
                    fOutputs.at(i)->toFilter->fInputs.at(fOutputs.at(i)->toPin)->links[lnk].fromPin--;
            }
        }
    }
    auto out = fOutputs.takeAt(idx);
    m_owner->repaintConn();
    out->deleteLater();
}

void ZCPBase::deleteComponent()
{
    for (const auto &inp : qAsConst(fInputs)) {
        while (!(inp->links.isEmpty())) {
            const auto link = inp->links.constLast();
            if ((link.fromPin!=-1) && (link.fromFilter!=nullptr)) {
                link.fromFilter->fOutputs[link.fromPin]->toFilter=nullptr;
                link.fromFilter->fOutputs[link.fromPin]->toPin=-1;
            }
            inp->links.removeLast();
        }
    }
    for (int i=0;i<fOutputs.count();i++) {
        if ((fOutputs.at(i)->toPin!=-1) && (fOutputs.at(i)->toFilter!=nullptr)) {
            fOutputs.at(i)->toFilter->fInputs.at(fOutputs.at(i)->toPin)
                    ->links.removeAll(CInpLink(i,this));
        }
        fOutputs[i]->toFilter=nullptr;
        fOutputs[i]->toPin=-1;
    }
    m_owner->repaintConn();
    deleteLater();
}

void ZCPBase::mouseInPin(const QPoint & mx, int &aPinNum, PinType &aPinType, ZCPBase* &aFilter)
{
    // Note: x and y must be in global screen coordinate system.
    QRect r;
    QPoint p;
    aPinNum=-1;
    aPinType=PinType::ptInput;
    aFilter=nullptr;
    for (const auto &oc : m_owner->children())
    {
        auto *c=qobject_cast<ZCPBase*>(oc);
        if (c==nullptr) continue;
        int j = 0;
        for (const auto &a : qAsConst(c->fInputs))
        {
            p=QPoint(m_owner->mapToGlobal(QPoint(a->ownerFilter->x()+a->relCoord.x()+zcpPinSize/2,
                                                 a->ownerFilter->y()+a->relCoord.y()+zcpPinSize/2)));
            r=QRect(p.x()-zcpPinSize,p.y()-zcpPinSize,2*zcpPinSize,2*zcpPinSize);
            if (r.contains(mx.x(),mx.y()))
            {
                aPinNum=j;
                aPinType=PinType::ptInput;
                aFilter=c;
                return;
            }
            j++;
        }

        j = 0;
        for (const auto &a : qAsConst(c->fOutputs))
        {
            p=QPoint(m_owner->mapToGlobal(QPoint(a->ownerFilter->x()+a->relCoord.x()+zcpPinSize/2,
                                                 a->ownerFilter->y()+a->relCoord.y()+zcpPinSize/2)));
            r=QRect(p.x()-zcpPinSize,p.y()-zcpPinSize,2*zcpPinSize,2*zcpPinSize);
            if (r.contains(mx.x(),mx.y()))
            {
                aPinNum=j;
                aPinType=PinType::ptOutput;
                aFilter=c;
                return;
            }
            j++;
        }
    }
}

void ZCPBase::redrawPins(QPainter & painter)
{
    realignPins();

    QPen op=painter.pen();
    QBrush ob=painter.brush();
    QFont of=painter.font();
    QPen penPin=QPen(m_pinColor);
    QBrush brshPin=QBrush(m_pinColor,Qt::SolidPattern);
    painter.setPen(penPin);
    painter.setBrush(brshPin);
    QFont n=of;
    n.setBold(false);
    painter.setFont(n);
    for (const auto & a : qAsConst(fInputs))
    {
        painter.fillRect(QRect(   a->relCoord.x()-zcpPinSize/2,
                                  a->relCoord.y()-zcpPinSize/2,
                                  zcpPinSize,
                                  zcpPinSize),brshPin);
        painter.drawText(QPoint(  a->relCoord.x()+zcpPinSize/2+1,
                                  a->relCoord.y()+painter.fontMetrics().height()/4),
                         a->pinName);

    }
    for (const auto & a : qAsConst(fOutputs))
    {
        painter.fillRect(QRect(   a->relCoord.x()-zcpPinSize/2,
                                  a->relCoord.y()-zcpPinSize/2,
                                  zcpPinSize,
                                  zcpPinSize),brshPin);
        painter.drawText(QPoint(  a->relCoord.x()-zcpPinSize/2-1 - painter.fontMetrics().boundingRect(a->pinName).width(),
                                  a->relCoord.y()+painter.fontMetrics().height()/4), a->pinName);
    }
    painter.setFont(of);
    painter.setBrush(ob);
    painter.setPen(op);
}

ZRenderArea *ZCPBase::ownerArea() const
{
    return m_owner;
}

void ZCPBase::registerInput(ZCPInput *inp)
{
    fInputs.append(inp);
}

void ZCPBase::registerOutput(ZCPOutput *out)
{
    fOutputs.append(out);
}

bool ZCPBase::postLoadBind()
{
    for (const auto &a : qAsConst(fInputs)) {
        if (!(a->postLoadBind()))
            return false;
    }
    for (const auto &a : qAsConst(fOutputs)) {
        if (!(a->postLoadBind()))
            return false;
    }
    return true;
}

void ZCPBase::checkRecycle()
{
    if (frameGeometry().intersects(m_owner->m_recycle->frameGeometry()))
        deleteComponent();
}

void ZCPBase::showHintDlg()
{
    QDialog dlg(topLevelWidget());
    Ui::ZHintDialog ui;
    ui.setupUi(&dlg);

    ui.editHint->setText(m_hint);
    ui.checkShowHint->setChecked(m_hintShow);

    if (dlg.exec()==QDialog::Rejected) return;

    m_hint = ui.editHint->text();
    m_hintShow = ui.checkShowHint->isChecked();

    Q_EMIT componentChanged(this);
    update();
}

void ZCPBase::showCtxMenu(const QPoint &pos)
{
    QMenu menu;
    addCtxMenuItems(&menu);
    if (!menu.isEmpty())
        menu.addSeparator();

    QAction* ac = menu.addAction(tr("Hint..."));
    connect(ac,&QAction::triggered,this,&ZCPBase::showHintDlg);

    ac = menu.addAction(tr("Properties..."));
    connect(ac,&QAction::triggered,this,[this](){
        showSettingsDlg();
    });

    menu.exec(pos);
}

void ZCPBase::mouseMoveEvent(QMouseEvent * event)
{
    if (m_isDragging)
    {
        move(QPoint(x()+event->x()-m_relCorner.x(),
                    y()+event->y()-m_relCorner.y()));
        m_owner->repaintConn();
        m_owner->resize(m_owner->minimumSizeHint());
        update();
        Q_EMIT componentChanged(this);
    } else {
        m_owner->refreshConnBuilder(event->pos());
    }
}

int ZCPBase::paintBase(QPainter &p, bool isGrowable)
{
    QPen pn=QPen(Qt::black);
    pn.setWidth(2);

    QPen op=p.pen();
    QBrush ob=p.brush();
    QFont of=p.font();

    p.setPen(pn);
    p.setBrush(QBrush(Qt::white,Qt::SolidPattern));

    p.drawRect(rect());

    redrawPins(p);

    int hintHeight = 0;

    if (!m_hint.isEmpty()) {
        QFont n=of;
        n.setBold(false);
        n.setPointSize(n.pointSize()-3);
        p.setPen(QPen(Qt::blue));
        p.setFont(n);
        hintHeight = height()/3;
        if (isGrowable)
            hintHeight = p.fontMetrics().height()+5;
        QRect hrect(0,0,width(),hintHeight);

        p.drawText(hrect,Qt::AlignCenter,m_hint);
    }

    p.setPen(op);
    p.setBrush(ob);
    p.setFont(of);

    return hintHeight;
}

void ZCPBase::doInfoGenerate(QTextStream &stream, QStringList &warnings) const
{
    Q_UNUSED(warnings)

    if (!m_hint.isEmpty()) {
        QString opt = QSL("off");
        if (m_hintShow)
            opt = QSL("on");

        stream << QSL("  hint {") << endl;
        stream << QSL("    show %1").arg(opt) << endl;
        stream << QSL("    description \"%1\"").arg(m_hint) << endl;
        stream << QSL("  }") << endl;
    }
}

QString ZCPBase::getHint() const
{
    return m_hint;
}

void ZCPBase::mousePressEvent(QMouseEvent * event)
{
    raise();
    QPoint mx=mapToGlobal(event->pos());

    if (event->button()==Qt::RightButton)
    {
        showCtxMenu(mx);
        return;
    }

    ZCPBase* dFlt;
    int pNum;
    PinType pType;
    mouseInPin(mx,pNum,pType,dFlt);
    Q_EMIT componentChanged(this);
    if (pNum<0) {
        m_relCorner=event->pos();
        m_isDragging=true;
        return;
    }
    m_isDragging=false;
    if (pType==PinType::ptInput) {
        m_owner->initConnBuilder(PinType::ptInput,pNum,dFlt->fInputs.at(pNum),nullptr,dFlt);
    } else {
        m_owner->initConnBuilder(PinType::ptOutput,pNum,nullptr,dFlt->fOutputs.at(pNum),dFlt);
    }
}

void ZCPBase::mouseReleaseEvent(QMouseEvent * event)
{
    QPoint mx=mapToGlobal(event->pos());
    ZCPBase* dFlt;
    int pNum;
    PinType pType;
    mouseInPin(mx,pNum,pType,dFlt);
    if (pNum==-1) {
        bool f=m_isDragging;
        m_isDragging=false;
        if (!f) {
            m_owner->doneConnBuilder(true,PinType::ptInput,-1,nullptr,nullptr,nullptr);
        } else {
            checkRecycle();
        }
        m_isDragging=f;
        return;
    }
    if (pType==PinType::ptInput) {
        m_owner->doneConnBuilder(false,PinType::ptInput,pNum,dFlt->fInputs.at(pNum),nullptr,dFlt);
    } else {
        m_owner->doneConnBuilder(false,PinType::ptOutput,pNum,nullptr,dFlt->fOutputs.at(pNum),dFlt);
    }
    Q_EMIT componentChanged(this);
}

void ZCPBase::readFromStreamLegacy( QDataStream & stream )
{
    m_hint.clear();
    m_hintShow = true;
    stream >> m_pinColor;
    for (int i=0;i<fInputs.count();i++)
        fInputs.at(i)->readFromStreamLegacy(stream);
    for (int i=0;i<fOutputs.count();i++)
        fOutputs.at(i)->readFromStreamLegacy(stream);
}

void ZCPBase::readFromJson(const QJsonValue &json)
{
    QColor color(json.toObject().value(QSL("pinColor")).toString());
    if (color.isValid()) {
        m_pinColor = color;
    } else {
        qWarning() << "Incorrect color in JSON";
    }

    m_hint = json.toObject().value(QSL("hint")).toString();
    m_hintShow = json.toObject().value(QSL("hintShow")).toBool(true);

    const QJsonArray inputs = json.toObject().value(QSL("inputs")).toArray();
    for (int i=0;i<fInputs.count();i++)
        fInputs.at(i)->readFromJson(inputs.at(i));

    const QJsonArray outputs = json.toObject().value(QSL("outputs")).toArray();
    for (int i=0;i<fOutputs.count();i++)
        fOutputs.at(i)->readFromJson(outputs.at(i));
}

QJsonValue ZCPBase::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("pinColor"),m_pinColor.name());
    data.insert(QSL("hint"),m_hint);
    data.insert(QSL("hintShow"),m_hintShow);

    QJsonArray inputs;
    for (int i=0;i<fInputs.count();i++)
        inputs.append(fInputs.at(i)->storeToJson());
    data.insert(QSL("inputs"),inputs);

    QJsonArray outputs;
    for (int i=0;i<fOutputs.count();i++)
        outputs.append(fOutputs.at(i)->storeToJson());
    data.insert(QSL("outputs"),outputs);

    return data;
}

QSize ZCPBase::sizeHint() const
{
    return minimumSizeHint();
}

void ZCPBase::showSettingsDlg()
{
}

void ZCPBase::addCtxMenuItems(QMenu *menu)
{
    Q_UNUSED(menu)
}

ZCPBase *ZCPBase::searchPluginBackward(const char *targetClass, ZCPBase *node) const
{
    // TODO: add recursion protector

    const ZCPBase* base = node;
    if (base == nullptr)
        base = this;

    for (const auto& inp: qAsConst(base->fInputs)) {
        for (const auto& link : qAsConst(inp->links)) {
            if (link.fromFilter) {
                if (std::strcmp(targetClass,link.fromFilter->metaObject()->className()) == 0)
                    return link.fromFilter; // found

                if (auto component = searchPluginBackward(targetClass,link.fromFilter))
                    return component;
            }
        }
    }
    return nullptr;
}

ZCPBase *ZCPBase::searchPluginForward(const char *targetClass, ZCPBase *node) const
{
    // TODO: add recursion protector

    const ZCPBase* base = node;
    if (base == nullptr)
        base = this;

    for (const auto& out: qAsConst(base->fOutputs)) {
        if (out->toFilter) {
            if (std::strcmp(targetClass,out->toFilter->metaObject()->className()) == 0)
                return out->toFilter; // found

            if (auto component = searchPluginForward(targetClass,out->toFilter))
                return component;
        }
    }
    return nullptr;
}

ZCPOutput::ZCPOutput(QObject * parent, ZCPBase * aOwner)
    : QObject(parent)
{
    pinName=QSL("<NA>");
    ownerFilter=aOwner;
}

void ZCPOutput::readFromStreamLegacy( QDataStream & stream )
{
    stream >> toPin;
    toFilter=nullptr;
    QString q;
    stream >> q;
    if (q!=QSL("<NONE>")) {
        ffLogic=q;
    } else {
        ffLogic.clear();
    }
}

void ZCPOutput::readFromJson(const QJsonValue &json)
{
    toFilter = nullptr;
    toPin = json.toObject().value(QSL("toPin")).toInt(-1);
    ffLogic = json.toObject().value(QSL("toFilter")).toString();
}

QJsonValue ZCPOutput::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("toPin"),toPin);

    QString filter;
    if (toFilter)
        filter= toFilter->objectName();
    data.insert(QSL("toFilter"),filter);

    return data;
}

bool ZCPOutput::postLoadBind()
{
    if (ffLogic.isEmpty()) return true;
    auto b = ownerFilter->ownerArea()->findChild<ZCPBase *>(ffLogic);
    if (b) {
        toFilter=b;
    } else {
        return false;
    }
    return true;
}

ZCPInput::ZCPInput(QObject * parent, ZCPBase * aOwner)
    : QObject(parent)
{
    pinName=QSL("<NA>");
    ownerFilter=aOwner;
}

void ZCPInput::readFromStreamLegacy( QDataStream & stream )
{
    qint32 fromPin;
    stream >> fromPin;

    QString ffLogic;
    stream >> ffLogic;
    if (ffLogic==QSL("<NONE>"))
        ffLogic.clear();

    links.append(CInpLink(fromPin,nullptr,ffLogic));
}

void ZCPInput::readFromJson(const QJsonValue &json)
{
    if (json.isArray()) {
        const QJsonArray jarray = json.toArray();
        for (const auto& item : jarray) {
            qint32 fromPin = item.toObject().value(QSL("fromPin")).toInt(-1);
            QString ffLogic = item.toObject().value(QSL("fromFilter")).toString();
            links.append(CInpLink(fromPin,nullptr,ffLogic));
        }
    } else {
        qint32 fromPin = json.toObject().value(QSL("fromPin")).toInt(-1);
        QString ffLogic = json.toObject().value(QSL("fromFilter")).toString();
        links.append(CInpLink(fromPin,nullptr,ffLogic));
    }
}

QJsonValue ZCPInput::storeToJson() const
{
    QJsonArray data;
    for (const auto& link : qAsConst(links)) {
        QJsonObject item;
        item.insert(QSL("fromPin"),link.fromPin);

        QString filter;
        if (link.fromFilter)
            filter = link.fromFilter->objectName();
        item.insert(QSL("fromFilter"),filter);

        data.append(item);
    }
    return data;
}

bool ZCPInput::postLoadBind()
{
    bool failed = false;
    for (auto & link : links) {
        if (!(link.ffLogic.isEmpty())) {
            auto b = ownerFilter->ownerArea()->findChild<ZCPBase *>(link.ffLogic);
            if (b) {
                link.fromFilter = b;
            } else {
                failed = true;
            }
        }
    }
    return (!failed);
}

CInpLink::CInpLink(qint32 aFromPin, ZCPBase *aFromFilter, const QString &affLogic)
{
    fromPin = aFromPin;
    fromFilter = aFromFilter;
    ffLogic = affLogic;
}

bool CInpLink::operator==(const CInpLink &s) const
{
    return ((fromPin == s.fromPin) &&
            (reinterpret_cast<qintptr>(fromFilter) == reinterpret_cast<qintptr>(s.fromFilter)));
}

bool CInpLink::operator!=(const CInpLink &s) const
{
    return !operator==(s);
}
