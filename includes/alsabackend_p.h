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

#ifndef ALSABACKENDPRIVATE_H
#define ALSABACKENDPRIVATE_H

#include <QObject>
#include <QTimer>
#include "alsastructures.h"

extern "C" {
#include <alsa/asoundlib.h>
}

class ZAlsaBackend;

class ZAlsaBackendPrivate : public QObject
{
    Q_OBJECT
private:
    Q_DISABLE_COPY(ZAlsaBackendPrivate)
    Q_DECLARE_PUBLIC(ZAlsaBackend)
    ZAlsaBackend* q_ptr { nullptr };

public:
    explicit ZAlsaBackendPrivate(ZAlsaBackend *parent);
    ~ZAlsaBackendPrivate() override;

    QVector<CCardItem> m_cards;
    QVector<snd_ctl_t *> m_mixerCtl;
    QStringList m_alsaWarnings;
    QTimer m_mixerPollTimer;

    snd_ctl_t* getMixerCtl(int cardNum);
    void addAlsaWarning(const QString& msg);
    void enumerateCards();
    static void snd_lib_error_handler(const char *file, int line, const char *function, int err, const char *fmt,...);
    static bool lessThanMixerItem(const CMixerItem &a, const CMixerItem &b);
    QVector<int> findRelatedMixerItems(const CMixerItem &base, const QVector<CMixerItem> &items, int *topScore);

private Q_SLOTS:
    void pollMixerEvents();

};

#endif // ALSABACKENDPRIVATE_H
