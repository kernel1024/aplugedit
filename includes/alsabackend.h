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

#ifndef ALSABACKEND_H
#define ALSABACKEND_H

#include <QtCore>

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
    QList<CDeviceItem> devices;
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

class ZAlsaBackend : public QObject
{
    Q_OBJECT
public:
    explicit ZAlsaBackend(QObject *parent = nullptr);
    ~ZAlsaBackend() override;
    static ZAlsaBackend *instance();
    void initialize();
    void reloadGlobalConfig();

    QVector<CCardItem> cards() const;
    QVector<CPCMItem> pcmList() const;
    bool getCardNumber(const QString &name, QString &cardId, unsigned int* devNum, unsigned int *subdevNum) const;

private:
    QVector<CCardItem> m_cards;

    Q_DISABLE_COPY(ZAlsaBackend)

    void enumerateCards();
};

#define gAlsa (ZAlsaBackend::instance())

#endif // ALSABACKEND_H
