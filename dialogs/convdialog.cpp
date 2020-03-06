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

#include "includes/convdialog.h"

ZConvDialog::ZConvDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    alFormat->addItem(tr("S8 Signed 8 bit"));
    alFormat->addItem(tr("U8 Unsigned 8 bit"));
    alFormat->addItem(tr("S16_LE Signed 16 bit Little Endian"));
    alFormat->addItem(tr("S16_BE Signed 16 bit Big Endian"));
    alFormat->addItem(tr("U16_LE Unsigned 16 bit Little Endian"));
    alFormat->addItem(tr("U16_BE Unsigned 16 bit Big Endian"));
    alFormat->addItem(tr("S24_LE Signed 24 bit Little Endian"));
    alFormat->addItem(tr("S24_BE Signed 24 bit Big Endian"));
    alFormat->addItem(tr("U24_LE Unsigned 24 bit Little Endian"));
    alFormat->addItem(tr("U24_BE Unsigned 24 bit Big Endian"));
    alFormat->addItem(tr("S32_LE Signed 32 bit Little Endian"));
    alFormat->addItem(tr("S32_BE Signed 32 bit Big Endian"));
    alFormat->addItem(tr("U32_LE Unsigned 32 bit Little Endian"));
    alFormat->addItem(tr("U32_BE Unsigned 32 bit Big Endian"));
    alFormat->addItem(tr("FLOAT_LE Float 32 bit Little Endian, Range -1.0 to 1.0"));
    alFormat->addItem(tr("FLOAT_BE Float 32 bit Big Endian, Range -1.0 to 1.0"));
    alFormat->addItem(tr("FLOAT64_LE Float 64 bit Little Endian, Range -1.0 to 1.0"));
    alFormat->addItem(tr("FLOAT64_BE Float 64 bit Big Endian, Range -1.0 to 1.0"));
    alFormat->addItem(tr("IEC_958_SUBFRAME_LE IEC-958 Little Endian"));
    alFormat->addItem(tr("IEC_958_SUBFRAME_BE IEC-958 Big Endian"));
    alFormat->addItem(tr("MU_LAW u-Law format"));
    alFormat->addItem(tr("A_LAW a-Law format"));
    alFormat->addItem(tr("IMA_ADPCM Ima-ADPCM format"));
    alFormat->addItem(tr("MPEG compressed format"));
    alFormat->addItem(tr("GSM compressed format"));
    alFormat->addItem(tr("S24_3LE Signed 24bit Little Endian in 3bytes format"));
    alFormat->addItem(tr("S24_3BE Signed 24bit Big Endian in 3bytes format"));
    alFormat->addItem(tr("U24_3LE Unsigned 24bit Little Endian in 3bytes format"));
    alFormat->addItem(tr("U24_3BE Unsigned 24bit Big Endian in 3bytes format"));
    alFormat->addItem(tr("S20_3LE Signed 20bit Little Endian in 3bytes format"));
    alFormat->addItem(tr("S20_3BE Signed 20bit Big Endian in 3bytes format"));
    alFormat->addItem(tr("U20_3LE Unsigned 20bit Little Endian in 3bytes format"));
    alFormat->addItem(tr("U20_3BE Unsigned 20bit Big Endian in 3bytes format"));
    alFormat->addItem(tr("S18_3LE Signed 18bit Little Endian in 3bytes format"));
    alFormat->addItem(tr("S18_3BE Signed 18bit Big Endian in 3bytes format"));
    alFormat->addItem(tr("U18_3LE Unsigned 18bit Little Endian in 3bytes format"));
    alFormat->addItem(tr("U18_3BE Unsigned 18bit Big Endian in 3bytes format"));
}

ZConvDialog::~ZConvDialog() = default;
