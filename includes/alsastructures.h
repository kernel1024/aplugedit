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

#ifndef ALSASTRUCT_H
#define ALSASTRUCT_H

#include <QString>
#include <QVector>
#include <QStringList>

class CDeviceItem
{
public:
    int devNum { -1 };
    int subdevices { -1 };
    QString devName;
    CDeviceItem() = default;
    ~CDeviceItem() = default;
    CDeviceItem(const CDeviceItem& other) = default;
    CDeviceItem(int aDevNum, int aSubdevices, const QString &aName);
    CDeviceItem &operator=(const CDeviceItem& other) = default;
};

class CCardItem
{
public:
    int cardNum { -1 };
    QString cardName;
    QString cardID;
    QVector<CDeviceItem> devices;
    CCardItem() = default;
    ~CCardItem() = default;
    CCardItem(const CCardItem& other) = default;
    CCardItem(const QString& aCardID, const QString& aCardName, int aCardNum);
    CCardItem &operator=(const CCardItem& other) = default;
};

class CPCMItem
{
public:
    QString name;
    QStringList description;
    CPCMItem() = default;
    ~CPCMItem() = default;
    CPCMItem(const CPCMItem& other) = default;
    explicit CPCMItem(const QString& aName);
    CPCMItem(const QString& aName, const QStringList& aDescription);
    CPCMItem &operator=(const CPCMItem& other) = default;
    bool operator==(const CPCMItem &s) const;
    bool operator!=(const CPCMItem &s) const;
};

class CMixerItem
{
public:
    enum ItemType {
        itBoolean,
        itInteger,
        itInteger64,
        itEnumerated
    };

    bool isUser { false };
    bool isRelated { false };
    int relatedNameLength { 0 };

    unsigned int numid { 0 };
    ItemType type { itInteger };
    long long valueMin { 0L };
    long long valueMax { 0L };
    long long valueStep { 0L };

    QString name;
    QVector<long long> values;
    QStringList labels;

    QVector<int> related;

    CMixerItem() = default;
    ~CMixerItem() = default;
    CMixerItem(const CMixerItem& other) = default;
    CMixerItem &operator=(const CMixerItem& other) = default;

    CMixerItem(unsigned int aNumid, const QString& aName, const QVector<int> &aValues);
    CMixerItem(unsigned int aNumid, const QString& aName, const QVector<long> &aValues,
               long min, long max, long step);
    CMixerItem(unsigned int aNumid, const QString& aName, const QVector<long long> &aValues,
               long long min, long long max, long long step);
    CMixerItem(unsigned int aNumid, const QString& aName, const QVector<unsigned int> &aValues,
               const QStringList &aLabels);

    bool isEmpty() const;
    bool operator==(const CMixerItem &s) const;
    bool operator!=(const CMixerItem &s) const;
};

#endif // ALSASTRUCT_H
