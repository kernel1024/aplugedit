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

#ifndef CPBASE_H
#define CPBASE_H 1

#include <QtCore>
#include <QtWidgets>
#include "renderarea.h"

const int zcpPinSize = 8;

class ZCPBase;
class ZRenderArea;
class ZCPMulti;

class ZCPOutput : public QObject
{
    Q_OBJECT
public:
    qint32 toPin { -1 };
    ZCPBase *toFilter { nullptr };
    ZCPBase *ownerFilter { nullptr };
    QPoint relCoord;
    QString pinName;
    QString ffLogic;

    ZCPOutput(QObject * parent, ZCPBase * aOwner);

    void readFromStreamLegacy( QDataStream & stream );
    void readFromJson(const QJsonValue& json);
    QJsonValue storeToJson() const;
    bool postLoadBind();
};

class CInpLink
{
public:
    qint32 fromPin { -1 };
    ZCPBase * fromFilter { nullptr };
    QString ffLogic;
    CInpLink() = default;
    ~CInpLink() = default;
    CInpLink(const CInpLink& other) = default;
    CInpLink(qint32 aFromPin, ZCPBase *aFromFilter, const QString &affLogic = QString());
    bool operator==(const CInpLink &s) const;
    bool operator!=(const CInpLink &s) const;
};

class ZCPInput : public QObject
{
    Q_OBJECT
public:
    ZCPBase * ownerFilter { nullptr };
    QPoint relCoord;
    QString pinName;
    QVector<CInpLink> links;

    ZCPInput(QObject * parent, ZCPBase * aOwner);
    
    void readFromStreamLegacy( QDataStream & stream );
    void readFromJson(const QJsonValue& json);
    QJsonValue storeToJson() const;
    bool postLoadBind();
};

class ZCPBase : public QWidget
{
    Q_OBJECT
    friend class ZRenderArea;
    friend class ZCPMulti;
public:
    enum PinType {
        ptInput = 1,
        ptOutput = 2
    };
    Q_ENUM(PinType)

    enum FontType {
        ftTitle = 1,
        ftDesc = 2,
        ftHint = 3
    };
    Q_ENUM(FontType)

    ZCPBase(QWidget *parent, ZRenderArea *aOwner);

    virtual void readFromStreamLegacy(QDataStream & stream);
    virtual void readFromJson(const QJsonValue& json);
    virtual QJsonValue storeToJson() const;

    virtual bool canConnectOut(ZCPBase *toFilter);
    virtual bool canConnectIn(ZCPBase *toFilter);

    QSize sizeHint() const override;

    ZRenderArea *ownerArea() const;

    void registerInput(ZCPInput* inp);
    void registerOutput(ZCPOutput* out);

    QString getHint() const;

protected:
    int paintBase(QPainter &p, bool isGrowable = false);
    void setBaseFont(QPainter& p, FontType type) const;
    void mouseMoveEvent(QMouseEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    virtual void doInfoGenerate(QTextStream & stream, QStringList & warnings) const;
    virtual void realignPins() { }
    virtual void showSettingsDlg() { }
    virtual bool needSettingsDlg() { return false; }
    virtual bool isHintSupported() { return true; }
    virtual void addCtxMenuItems(QMenu* menu) { Q_UNUSED(menu) }

    ZCPBase* searchPluginBackward(const char *targetClass, const ZCPBase *node = nullptr,
                                  QSharedPointer<QStringList> searchStack = { }) const;
    ZCPBase* searchPluginForward(const char *targetClass, const ZCPBase *node = nullptr,
                                 QSharedPointer<QStringList> searchStack = { }) const;

Q_SIGNALS:
    void componentChanged(ZCPBase * obj);

public Q_SLOTS:
    void deleteComponent();

private:
    bool m_isDragging { false };
    bool m_hintShow { true };
    ZRenderArea *m_owner;
    QString m_hint;
    QList<ZCPInput*> fInputs;
    QList<ZCPOutput*> fOutputs;
    QColor m_pinColor { Qt::blue };
    QPoint m_relCorner;

    void deleteOutput(int idx);

    void mouseInPin(const QPoint& mx, int &aPinNum, ZCPBase::PinType &aPinType, ZCPBase *&aFilter);
    void checkRecycle();
    void showCtxMenu(const QPoint& pos);

    void redrawPins(QPainter & painter);
    bool postLoadBind();
    void showHintDlg();

    Q_DISABLE_COPY(ZCPBase)

};

#endif
