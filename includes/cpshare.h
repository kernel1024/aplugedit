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

#ifndef CPDMIX_H
#define CPDMIX_H 1

#include "cpbase.h"

class ZCPShare : public ZCPBase
{
    Q_OBJECT
public:
    enum SharePlugin {
        spDMix = 0,
        spDSnoop = 1,
        spDShare = 2
    };
    Q_ENUM(SharePlugin)

    enum HWPtrAlignment {
        haEmpty = 0,
        haAuto = 1,
        haNo = 2,
        haRoundUp = 3,
        haRoundDown = 4,
        ha_Max = 5
    };
    Q_ENUM(HWPtrAlignment)

    ZCPShare(QWidget *parent, ZRenderArea *aOwner, ZCPShare::SharePlugin mode = ZCPShare::SharePlugin::spDMix);
    ~ZCPShare() override;

    void setPluginMode(ZCPShare::SharePlugin mode);

    void readFromJson(const QJsonValue& json) override;
    QJsonValue storeToJson() const override;

    QSize minimumSizeHint() const override;
    bool canConnectOut(ZCPBase * toFilter) override;

protected:
    void paintEvent (QPaintEvent * event) override;
    void realignPins() override;
    void doInfoGenerate(QTextStream & stream, QStringList & warnings) const override;
    void showSettingsDlg() override;
    bool needSettingsDlg() override { return true; }

private:
    ZCPInput* fInp { nullptr };
    ZCPOutput* fOut { nullptr };
    ZCPOutput* fCtlOut { nullptr };

    Qt::CheckState m_slowPtr { Qt::CheckState::PartiallyChecked }; // default - no change
    SharePlugin m_mode { spDMix };
    HWPtrAlignment m_hwPtrAlignment { haEmpty };
    QVector<int> m_bindings;
    QString m_IPCkey;
    QString m_IPCpermissions;

};
#endif
