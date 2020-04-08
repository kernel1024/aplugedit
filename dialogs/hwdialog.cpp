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

#include "includes/generic.h"
#include "includes/hwdialog.h"
#include "includes/alsabackend.h"

ZHWDialog::ZHWDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    alCard->addItem(tr("-not specified-"));
    alCard->setCurrentIndex(0);

    m_cards = gAlsa->cards();

    for (const auto &hw : qAsConst(m_cards))
        alCard->addItem(QSL("%1 [%2]").arg(hw.cardID,hw.cardName),hw.cardID);

    connect(alCard,qOverload<int>(&QComboBox::currentIndexChanged),this,&ZHWDialog::cardSelected);
    connect(alDevice,qOverload<int>(&QComboBox::currentIndexChanged),this,&ZHWDialog::devSelected);

    alCard->setCurrentIndex(0);
    cardSelected(0);

    alFormat->addItem(tr("-not specified-"));
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

    alChannels->addItem(tr("-not specified-"));
    alChannels->addItem(tr("1 - mono"));
    alChannels->addItem(tr("2 - stereo"));
    alChannels->addItem(tr("4 - quadro"));
    alChannels->addItem(tr("6 - 5.1 speakers"));
    alChannels->addItem(tr("8 - 7.1 speakers"));

    alRate->addItem(tr("-not specified-"));
    alRate->addItem(tr("8000 Hz"));
    alRate->addItem(tr("11025 Hz"));
    alRate->addItem(tr("22050 Hz"));
    alRate->addItem(tr("44,1 kHz"));
    alRate->addItem(tr("48 kHz"));
    alRate->addItem(tr("96 kHz"));
    alRate->addItem(tr("192 kHz"));

    alMMap->setCheckState(Qt::PartiallyChecked);
    alSyncPtr->setCheckState(Qt::PartiallyChecked);
    alNonblock->setCheckState(Qt::PartiallyChecked);
}

ZHWDialog::~ZHWDialog() = default;

void ZHWDialog::setParams(int mCard, const QString &mCardName, bool mPreferSymbolicName,
                          int mDevice, int mSubdevice, int mMmap_emulation,
                          int mSync_ptr_ioctl, int mNonblock, int mChannels, int mRate,
                          const QString &mFormat)
{
    int cardidx=-1;
    int devidx=-1;
    int subidx=-1;

    for (int i=0;i<m_cards.count();i++) {
        if ((mPreferSymbolicName && !mCardName.isEmpty() && mCardName==m_cards.at(i).cardID) ||
                (m_cards.at(i).cardNum==mCard))
        {
            cardidx=i;
            break;
        }
    }
    if (cardidx!=-1)
    {
        alCard->setCurrentIndex(cardidx+1);
        for (int i=0;i<m_cards.at(cardidx).devices.count();i++) {
            if (m_cards.at(cardidx).devices.at(i).devNum==mDevice)
            {
                devidx=i;
                break;
            }
        }
        if (devidx!=-1)
        {
            alDevice->setCurrentIndex(devidx+1);
            if ((mSubdevice<m_cards.at(cardidx).devices.at(devidx).subdevices) && (mSubdevice>=0))
            {
                subidx=mSubdevice;
                alSubdevice->setCurrentIndex(subidx+1);
            }
        }
    }

    int fmtidx=alFormat->findText(mFormat,Qt::MatchStartsWith | Qt::MatchCaseSensitive);
    if ((fmtidx>=0) && (fmtidx<alFormat->count()))
        alFormat->setCurrentIndex(fmtidx);

    switch (mChannels)
    {
        case 1: alChannels->setCurrentIndex(1); break;
        case 2: alChannels->setCurrentIndex(2); break;
        case 4: alChannels->setCurrentIndex(3); break;
        case 6: alChannels->setCurrentIndex(4); break;
        case 8: alChannels->setCurrentIndex(5); break;
        default: alChannels->setCurrentIndex(0);
    }

    switch (mRate)
    {
        case 8000: alRate->setCurrentIndex(1); break;
        case 11025: alRate->setCurrentIndex(2); break;
        case 22050: alRate->setCurrentIndex(3); break;
        case 44100: alRate->setCurrentIndex(4); break;
        case 48000: alRate->setCurrentIndex(5); break;
        case 96000: alRate->setCurrentIndex(6); break;
        case 192000: alRate->setCurrentIndex(7); break;
        default: alRate->setCurrentIndex(0);
    }

    alMMap->setTristate(true);
    alSyncPtr->setTristate(true);
    alNonblock->setTristate(true);
    alPreferSymbolicName->setChecked(mPreferSymbolicName);

    switch (mMmap_emulation)
    {
        case -1: alMMap->setCheckState(Qt::PartiallyChecked); break;
        case 0:  alMMap->setCheckState(Qt::Unchecked); break;
        case 1:  alMMap->setCheckState(Qt::Checked); break;
        default: alMMap->setCheckState(Qt::Unchecked);
    }
    switch (mSync_ptr_ioctl)
    {
        case -1: alSyncPtr->setCheckState(Qt::PartiallyChecked); break;
        case 0:  alSyncPtr->setCheckState(Qt::Unchecked); break;
        case 1:  alSyncPtr->setCheckState(Qt::Checked); break;
        default: alSyncPtr->setCheckState(Qt::Unchecked);
    }
    switch (mNonblock)
    {
        case -1: alNonblock->setCheckState(Qt::PartiallyChecked); break;
        case 0:  alNonblock->setCheckState(Qt::Unchecked); break;
        case 1:  alNonblock->setCheckState(Qt::Checked); break;
        default: alNonblock->setCheckState(Qt::Unchecked);
    }
}

void ZHWDialog::getParams(int &mCard, QString &mCardName, bool &mPreferSymbolicName,
                          int &mDevice, int &mSubdevice, int &mMmap_emulation,
                          int &mSync_ptr_ioctl, int &mNonblock, int &mChannels, int &mRate,
                          QString &mFormat)
{
    int cardIdx = alCard->currentIndex() - 1;
    if (cardIdx>=0) {
        mCard = m_cards.at(cardIdx).cardNum;
        mCardName = m_cards.at(cardIdx).cardID;
        if (alDevice->currentIndex()!=0) {
            mDevice = m_cards.at(cardIdx).devices.at(alDevice->currentIndex()-1).devNum;
            mSubdevice = alSubdevice->currentIndex()-1;
        } else {
            mDevice = -1;
        }
    } else {
        mCard = -1;
    }

    switch (alChannels->currentIndex())
    {
        case 1: mChannels=1; break;
        case 2: mChannels=2; break;
        case 3: mChannels=4; break;
        case 4: mChannels=6; break;
        case 5: mChannels=8; break;
        default: mChannels=-1;
    }

    switch (alRate->currentIndex())
    {
        case 1: mRate=8000; break;
        case 2: mRate=11025; break;
        case 3: mRate=22050; break;
        case 4: mRate=44100; break;
        case 5: mRate=48000; break;
        case 6: mRate=96000; break;
        case 7: mRate=192000; break;
        default: mRate=-1; break;
    }

    if (alFormat->currentIndex()>0) {
        mFormat=alFormat->currentText().section(QSL(" "),0,0);
    } else {
        mFormat=QSL("<NONE>");
    }

    switch (alMMap->checkState())
    {
        case Qt::Unchecked:        mMmap_emulation=0; break;
        case Qt::PartiallyChecked: mMmap_emulation=-1; break;
        case Qt::Checked:          mMmap_emulation=1; break;
    }
    switch (alSyncPtr->checkState())
    {
        case Qt::Unchecked:        mSync_ptr_ioctl=0; break;
        case Qt::PartiallyChecked: mSync_ptr_ioctl=-1; break;
        case Qt::Checked:          mSync_ptr_ioctl=1; break;
    }
    switch (alNonblock->checkState())
    {
        case Qt::Unchecked:        mNonblock=0; break;
        case Qt::PartiallyChecked: mNonblock=-1; break;
        case Qt::Checked:          mNonblock=1; break;
    }
    mPreferSymbolicName = alPreferSymbolicName->isChecked();
}

void ZHWDialog::cardSelected(int index)
{
    alDevice->clear();
    alDevice->addItem(tr("-not specified-"));
    if (index<=0)
    {
        alDevice->setCurrentIndex(0);
        alDevice->setEnabled(false);
        return;
    }
    if (m_cards.isEmpty()) return;

    for (int i=0;i<m_cards.at(index-1).devices.count();i++) {
        const auto dev = m_cards.at(index-1).devices.at(i);
        alDevice->addItem(QSL("%1: %2").arg(dev.devNum).arg(dev.devName));
    }
    alDevice->setCurrentIndex(0);

    alDevice->setEnabled(true);
}

void ZHWDialog::devSelected(int index)
{
    alSubdevice->clear();
    alSubdevice->addItem(tr("-not specified-"));
    if (index<=0)
    {
        alSubdevice->setCurrentIndex(0);
        alSubdevice->setEnabled(false);
        return;
    }
    if (m_cards.isEmpty()) return;
    int card = alCard->currentIndex();
    if (card<=0) return;

    for (int i=0;i<m_cards.at(card-1).devices.at(index-1).subdevices;i++)
        alSubdevice->addItem(QString::number(i));
    alSubdevice->setCurrentIndex(0);

    alSubdevice->setEnabled(true);
}
