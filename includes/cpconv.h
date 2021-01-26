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

#ifndef CPCONV_H
#define CPCONV_H 1

#include "cpbase.h"

class ZCPConv : public ZCPBase
{
    Q_OBJECT
public:
    enum ConverterType {
        alcLinear,
        alcFloat,
        alcIEC958,
        alcMuLaw,
        alcALaw,
        alcImaADPCM
    };
    Q_ENUM(ConverterType)

    ZCPConv(QWidget *parent, ZRenderArea *aOwner);
    ~ZCPConv() override;

    void readFromJson(const QJsonValue& json) override;
    QJsonValue storeToJson() const override;

    void setConverterType(ZCPConv::ConverterType type);
    ZCPConv::ConverterType getConverterType() const;

    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent * event) override;
    void realignPins() override;
    void doInfoGenerate(QTextStream & stream, QStringList & warnings) const override;
    void showSettingsDlg() override;
    bool needSettingsDlg() override { return true; }

private:
    QString m_format;
    ConverterType m_converter { alcLinear };
    ZCPInput* fInp { nullptr };
    ZCPOutput* fOut { nullptr };

};
#endif
