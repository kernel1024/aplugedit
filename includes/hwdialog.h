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

#ifndef HWDLG_H
#define HWDLG_H
#include <QtGui>
#include <QtCore>
#include "ui_hwdlg.h"

class CCardItem;

class ZHWDialog : public QDialog, public Ui::ZHWDialog
{
    Q_OBJECT
private:
    QList<CCardItem> hwCnt;

public:
    ZHWDialog(QWidget *parent = nullptr);
    ~ZHWDialog() override;

    void setParams(int mCard, int mDevice, int mSubdevice, int mMmap_emulation,
                   int mSync_ptr_ioctl, int mNonblock, int mChannels, int mRate,
                   const QString& mFormat);

    void getParams(int &mCard, int &mDevice, int &mSubdevice, int &mMmap_emulation,
                   int &mSync_ptr_ioctl, int &mNonblock, int &mChannels, int &mRate,
                   QString& mFormat);

public Q_SLOTS:
    void cardSelected(int index);
    void devSelected(int index);
};

class CDeviceItem
{
public:
    int devNum { -1 };
    int subdevices { -1 };
    CDeviceItem() = default;
    ~CDeviceItem() = default;
    CDeviceItem(const CDeviceItem& other);
    CDeviceItem(int aDevNum, int aSubdevices);
    CDeviceItem &operator=(const CDeviceItem& other) = default;
};

class CCardItem
{
public:
    QString cardName;
    int cardNum { -1 };
    QList<CDeviceItem> devices;
    CCardItem() = default;
    ~CCardItem() = default;
    CCardItem(const CCardItem& other);
    CCardItem(const QString& aCardName, int aCardNum);
    CCardItem &operator=(const CCardItem& other) = default;
};

#endif
