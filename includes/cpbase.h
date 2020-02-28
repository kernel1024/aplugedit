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

const int zcpPinSize = 8;

class ZCPInput;
class ZCPOutput;
class ZRenderArea;

class ZCPBase : public QWidget
{
    Q_OBJECT
    friend class ZRenderArea;
public:
    enum PinType {
        ptInput = 1,
        ptOutput = 2
    };
    Q_ENUM(PinType)

    ZCPBase(QWidget *parent, ZRenderArea *aOwner);
    
    virtual void readFromStreamLegacy(QDataStream & stream);
    virtual void readFromJson(const QJsonValue& json);
    virtual QJsonValue storeToJson() const;

    virtual bool canConnectOut(ZCPBase *toFilter);
    virtual bool canConnectIn(ZCPBase *toFilter);

    QSize sizeHint() const override;

    ZRenderArea *ownerArea() const;
    bool postLoadBind();
    void doGenerate(QTextStream & stream);
    void redrawPins(QPainter & painter);

    void registerInput(ZCPInput* inp);
    void registerOutput(ZCPOutput* out);
    virtual ZCPOutput* getMainOutput() const;

private:
    QList<ZCPInput*> fInputs;
    QList<ZCPOutput*> fOutputs;
    bool m_isDragging { false };
    QColor m_pinColor { Qt::blue };
    QPoint m_relCorner;
    ZRenderArea *m_owner;

    void mouseInPin(const QPoint& mx, int &aPinNum, ZCPBase::PinType &aPinType, ZCPBase *&aFilter);
    void checkRecycle();

    Q_DISABLE_COPY(ZCPBase)

protected:
    void mouseMoveEvent(QMouseEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    void paintEvent(QPaintEvent *event) override;
    virtual void realignPins()=0;
    virtual void doInfoGenerate(QTextStream & stream) const = 0;
    virtual void showSettingsDlg();
    virtual void addCtxMenuItems(QMenu* menu);

Q_SIGNALS:
    void componentChanged(ZCPBase * obj);

public Q_SLOTS:
    void deleteComponent();

};

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

class ZCPInput : public QObject
{
    Q_OBJECT
public:
    qint32 fromPin { -1 };
    ZCPBase * fromFilter { nullptr };
    ZCPBase * ownerFilter { nullptr };
    QPoint relCoord;
    QString pinName;
    QString ffLogic;

    ZCPInput(QObject * parent, ZCPBase * aOwner);
    
    void readFromStreamLegacy( QDataStream & stream );
    void readFromJson(const QJsonValue& json);
    QJsonValue storeToJson() const;
    bool postLoadBind();
};

#define QSL QStringLiteral

#endif
