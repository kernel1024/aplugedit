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

#include "includes/alsastructures.h"


CCardItem::CCardItem(const QString &aCardID, const QString &aCardName, int aCardNum)
{
    cardName = aCardName;
    cardNum = aCardNum;
    cardID = aCardID;
}

CDeviceItem::CDeviceItem(int aDevNum, int aSubdevices, const QString& aName)
{
    devNum = aDevNum;
    subdevices = aSubdevices;
    devName = aName;
}

CPCMItem::CPCMItem(const QString &aName)
{
    name = aName;
}

CPCMItem::CPCMItem(const QString &aName, const QStringList &aDescription)
{
    name = aName;
    description = aDescription;
}

bool CPCMItem::operator==(const CPCMItem &s) const
{
    return (name == s.name);
}

bool CPCMItem::operator!=(const CPCMItem &s) const
{
    return !operator==(s);
}

CCTLItem::CCTLItem(const QString &aName, const QStringList &aDescription, const QString &aDisplayName, snd_ctl_t *aCtl)
{
    name = aName;
    description = aDescription;
    displayName = aDisplayName;
    ctl = aCtl;
}

bool CCTLItem::operator==(const CCTLItem &s) const
{
    return (ctl == s.ctl);
}

bool CCTLItem::operator!=(const CCTLItem &s) const
{
    return !operator==(s);
}

CMixerItem::CMixerItem(unsigned int aNumid, const QString& aName, const QVector<int> &aValues)
{
    type = itBoolean;
    numid = aNumid;
    name = aName;
    values.reserve(aValues.count());
    for (const auto &v : aValues)
        values.append(v);
}

CMixerItem::CMixerItem(unsigned int aNumid, const QString& aName, const QVector<long> &aValues,
                       long min, long max, long step)
{
    type = itInteger;
    numid = aNumid;
    name = aName;
    valueMin = min;
    valueMax = max;
    valueStep = step;
    values.reserve(aValues.count());
    for (const auto &v : aValues)
        values.append(v);
}

CMixerItem::CMixerItem(unsigned int aNumid, const QString& aName, const QVector<long long> &aValues,
                       long long min, long long max, long long step)
{
    type = itInteger64;
    numid = aNumid;
    name = aName;
    valueMin = min;
    valueMax = max;
    valueStep = step;
    values = aValues;
}

CMixerItem::CMixerItem(unsigned int aNumid, const QString &aName, const QVector<unsigned int> &aValues, const QStringList &aLabels)
{
    type = itEnumerated;
    numid = aNumid;
    name = aName;
    labels = aLabels;
    values.reserve(aValues.count());
    for (const auto &v : aValues)
        values.append(v);
}

bool CMixerItem::isEmpty() const
{
    return (numid == 0);
}

bool CMixerItem::operator==(const CMixerItem &s) const
{
    return (numid == s.numid);
}

bool CMixerItem::operator!=(const CMixerItem &s) const
{
    return !operator==(s);
}
