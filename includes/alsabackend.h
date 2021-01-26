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
***************************************************************************
*                                                                         *
*   Parts of this code from ALSA project utilities:                       *
*   amixer, aplay, alsactl.                                               *
*                                                                         *
*   Copyright (c) by Takashi Iwai <tiwai@suse.de>                         *
*   Copyright (c) by Abramo Bagnara <abramo@alsa-project.org>             *
*   Copyright (c) by Jaroslav Kysela <perex@perex.cz>                     *
*   Based on vplay program by Michael Beck                                *
*                                                                         *
***************************************************************************/

#ifndef ALSABACKEND_H
#define ALSABACKEND_H

#include <QObject>
#include "alsastructures.h"

class ZAlsaBackendPrivate;
class ZSamplePlayer;

class ZAlsaBackend : public QObject
{
    Q_OBJECT
private:
    Q_DISABLE_COPY(ZAlsaBackend)
    Q_DECLARE_PRIVATE_D(dptr,ZAlsaBackend)
    QScopedPointer<ZAlsaBackendPrivate> dptr;

public:
    explicit ZAlsaBackend(QObject *parent = nullptr);
    ~ZAlsaBackend() override;
    static ZAlsaBackend *instance();

    void initialize();
    void reloadGlobalConfig();
    void setupErrorLogger();

    QVector<CCardItem> cards() const;
    QVector<CPCMItem> pcmList() const;
    bool getCardNumber(const QString &name, QString &cardId, unsigned int* devNum, unsigned int *subdevNum);

    QVector<CMixerItem> getMixerControls(const QString &ctlName) const;
    void setMixerControl(const QString &ctlName, const CMixerItem& item);
    void deleteMixerControl(const QString &ctlName, const CMixerItem& item);
    QStringList getMixerCtls(bool forceReload = false);
    QString getMixerName(const QString &ctlName) const;

    QStringList getAlsaWarnings();
    bool isWarnings() const;
    QStringList getDebugMessages();

Q_SIGNALS:
    void alsaWarningMsg(const QString& message); // cross-thread signal!
    void debugOutputUpdated(); // cross-thread signal!
    void alsaMixerReconfigured(const QString& ctlName);
    void alsaMixerValueChanged(const QString& ctlName);
};

#define gAlsa (ZAlsaBackend::instance())

#endif // ALSABACKEND_H
