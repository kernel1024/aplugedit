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

#ifndef CPHW_H
#define CPHW_H 1

#include <QtCore>
#include <QtGui>
#include "cpbase.h"

class ZCPHW : public ZCPBase
{
    Q_OBJECT
private:
    ZCPInput* fInp { nullptr };
    int m_card { 0 };      // link to card
    int m_device { -1 };    // device number (default 0)
    int m_subdevice { -1 }; // subdevice number (default -1: first available)
    int m_mmap_emulation { -1 }; // enable mmap emulation for ro/wo devices
    int m_sync_ptr_ioctl { -1 }; // use SYNC_PTR ioctl rather than the direct mmap access for control structures
    int m_nonblock { -1 };   // force non-blocking open mode
    int m_channels { -1 }; // restrict only to the given channels
    int m_rate { -1 }; // restrict only to the given rate
    QString m_format; // restrict only to the given format

public:
    ZCPHW(QWidget *parent, ZRenderArea *aOwner);
    ~ZCPHW() override;

    void readFromStreamLegacy(QDataStream & stream) override;
    void readFromJson(const QJsonValue& json) override;
    QJsonValue storeToJson() const override;

    QSize minimumSizeHint() const override;

    int getRate() const;
    int getChannels() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void realignPins() override;
    void doInfoGenerate(QTextStream & stream) const override;
    void showSettingsDlg() override;
};
#endif
