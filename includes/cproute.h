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

#ifndef CPROUTE_H
#define CPROUTE_H 1

#include <QtCore>
#include <QtGui>
#include "cpbase.h"

class CRouteItem
{
public:
    int from { -1 };
    double coeff { 0.0 };
    CRouteItem() = default;
    ~CRouteItem() = default;
    CRouteItem(const CRouteItem& other);
    CRouteItem(int aFrom, double aCoeff);
    CRouteItem &operator=(const CRouteItem& other) = default;
};

class ZCPRoute : public ZCPBase
{
    Q_OBJECT
private:
    int m_channelsIn;
    QVector<CRouteItem> m_routeTable;
    ZCPInput* fInp { nullptr };
    ZCPOutput* fOut { nullptr };

public:
    ZCPRoute(QWidget *parent, ZRenderArea *aOwner);
    ~ZCPRoute() override;

    void readFromStreamLegacy( QDataStream & stream ) override;
    void readFromJson(const QJsonValue& json) override;
    QJsonValue storeToJson() const override;

    QSize minimumSizeHint() const override;
    int getChannelsOut() const;

protected:
    void paintEvent ( QPaintEvent * event ) override;
    void doInfoGenerate(QTextStream & stream, QStringList & warnings) const override;
    void realignPins() override;
    void showSettingsDlg() override;
};

#endif
