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

#include <algorithm>
#include <QApplication>
#include "includes/generic.h"
#include "includes/alsabackend.h"
#include "includes/alsabackend_p.h"

extern "C" {
#include <alsa/asoundlib.h>
}

#define WARN(MSG) d->addAlsaWarning(QStringLiteral("%1: %2 (%3:%4)") \
    .arg(static_cast<const char *>(Q_FUNC_INFO)) \
    .arg(MSG) \
    .arg(static_cast<const char *>(__FILE__)) \
    .arg(__LINE__))

ZAlsaBackend::ZAlsaBackend(QObject *parent)
    : QObject(parent)
    , dptr(new ZAlsaBackendPrivate(this))
{
}

ZAlsaBackend::~ZAlsaBackend() = default;

ZAlsaBackend *ZAlsaBackend::instance()
{
    static QPointer<ZAlsaBackend> inst;
    static QAtomicInteger<bool> initializedOnce(false);

    if (inst.isNull()) {
        if (initializedOnce.testAndSetAcquire(false,true)) {
            inst = new ZAlsaBackend(QApplication::instance());
            return inst.data();
        }

        qCritical() << "Accessing to gAlsa after destruction!!!";
        return nullptr;
    }

    return inst.data();
}

void ZAlsaBackend::initialize()
{
    Q_D(ZAlsaBackend);
    d->enumerateCards();
}

void ZAlsaBackend::reloadGlobalConfig()
{
    snd_config_update_free_global();
    snd_config_update();
}

void ZAlsaBackend::setupErrorLogger()
{
    snd_lib_error_set_handler(&ZAlsaBackendPrivate::snd_lib_error_handler);
}

QVector<CCardItem> ZAlsaBackend::cards() const
{
    Q_D(const ZAlsaBackend);
    return d->m_cards;
}

QVector<CPCMItem> ZAlsaBackend::pcmList() const
{
    QVector<CPCMItem> res;

    const char *filter = "Output";

    void **hints;
    if (snd_device_name_hint(-1, "pcm", &hints) < 0)
        return res;

    void **n = hints;
    while (*n != nullptr) {
        QScopedPointer<char,QScopedPointerPodDeleter> name(snd_device_name_get_hint(*n, "NAME"));
        QScopedPointer<char,QScopedPointerPodDeleter> descr(snd_device_name_get_hint(*n, "DESC"));
        QScopedPointer<char,QScopedPointerPodDeleter> io(snd_device_name_get_hint(*n, "IOID"));
        if (!(io != nullptr && strcmp(io.data(), filter) != 0)) {
            QStringList descList;
            if (descr != nullptr)
                descList = QString::fromUtf8(descr.data()).split('\n');
            res.append(CPCMItem(QString::fromUtf8(name.data()),descList));
        }
        n++;
    }
    snd_device_name_free_hint(hints);

    return res;
}

bool ZAlsaBackend::getCardNumber(const QString& name, QString &cardId, unsigned int *devNum, unsigned int *subdevNum)
{
    Q_D(ZAlsaBackend);

    int err;
    QByteArray deviceName = name.toUtf8();
    snd_ctl_t *ctl;
    snd_ctl_card_info_t *info;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_t *pcminfo;
    snd_pcm_info_alloca(&pcminfo);

    err = snd_ctl_open(&ctl, deviceName.constData(), SND_CTL_NONBLOCK);
    if (err < 0) {
        WARN(QSL("snd_ctl_open: %1").arg(QString::fromUtf8(snd_strerror(err))));
        return false;
    }

    err = snd_ctl_card_info(ctl,info);
    if (err < 0) {
        WARN(QSL("snd_ctl_card_info: %1").arg(QString::fromUtf8(snd_strerror(err))));
        snd_ctl_close(ctl);
        return false;
    }

    cardId = QString::fromUtf8(snd_ctl_card_info_get_id(info));

    err = snd_ctl_pcm_info(ctl,pcminfo);
    if (err < 0) {
        WARN(QSL("snd_ctl_pcm_info: %1").arg(QString::fromUtf8(snd_strerror(err))));
        snd_ctl_close(ctl);
        return false;
    }

    *devNum = snd_pcm_info_get_device(pcminfo);
    *subdevNum = snd_pcm_info_get_subdevice(pcminfo);

    snd_ctl_close(ctl);

    return true;
}

QVector<CMixerItem> ZAlsaBackend::getMixerControls(int cardNum)
{
    Q_D(ZAlsaBackend);

    int err;

    QVector<CMixerItem> res;

    snd_ctl_t* ctl = d->getMixerCtl(cardNum);
    if (ctl==nullptr) return res;

    snd_ctl_elem_list_t *clist = nullptr;
    snd_ctl_elem_list_alloca(&clist);
    if ((err = snd_ctl_elem_list(ctl, clist)) < 0) {
        WARN(QSL("snd_ctl_elem_list: %1").arg(QString::fromUtf8(snd_strerror(err))));
        return res;
    }

    if ((err = snd_ctl_elem_list_alloc_space(clist, snd_ctl_elem_list_get_count(clist))) < 0) {
        WARN(QSL("snd_ctl_elem_list_alloc_space: %1").arg(QString::fromUtf8(snd_strerror(err))));
        return res;
    }
    if ((err = snd_ctl_elem_list(ctl, clist)) < 0) {
        WARN(QSL("snd_ctl_elem_list: %1").arg(QString::fromUtf8(snd_strerror(err))));
        snd_ctl_elem_list_free_space(clist);
        return res;
    }

    unsigned int controls = snd_ctl_elem_list_get_used(clist);
    for (unsigned int cidx = 0; cidx < controls; cidx++) {
        snd_ctl_elem_id_t* cid;
        snd_ctl_elem_id_malloc(&cid);
        snd_ctl_elem_info_t* cinfo;
        snd_ctl_elem_info_malloc(&cinfo);
        snd_ctl_elem_value_t* cvalue;
        snd_ctl_elem_value_malloc(&cvalue);

        snd_ctl_elem_list_get_id(clist, cidx, cid);

        snd_ctl_elem_info_set_id(cinfo,cid);
        if ((err = snd_ctl_elem_info(ctl,cinfo)) < 0) {
            WARN(QSL("snd_ctl_elem_info: %1").arg(QString::fromUtf8(snd_strerror(err))));
        } else {
            snd_ctl_elem_value_set_id(cvalue,cid);
            if ((err = snd_ctl_elem_read(ctl,cvalue)) < 0) {
                WARN(QSL("snd_ctl_elem_read: %1").arg(QString::fromUtf8(snd_strerror(err))));
            } else {

                // Only accept mixer controls
                if (snd_ctl_elem_info_get_interface(cinfo) == SND_CTL_ELEM_IFACE_MIXER) {
                    unsigned int count = snd_ctl_elem_info_get_count(cinfo);
                    unsigned int numid = snd_ctl_elem_info_get_numid(cinfo);
                    QString name = QString::fromUtf8(snd_ctl_elem_info_get_name(cinfo));
                    auto ctype = snd_ctl_elem_info_get_type(cinfo);

                    // Acceptable control types and basic parameters
                    CMixerItem mxItem;
                    switch (ctype) {
                    case SND_CTL_ELEM_TYPE_BOOLEAN: {
                        QVector<int> values;
                        for (unsigned int k = 0; k < count; k++)
                            values.append(snd_ctl_elem_value_get_boolean(cvalue,k));
                        res.append(CMixerItem(numid,name,values));
                        break;
                    }
                    case SND_CTL_ELEM_TYPE_INTEGER: {
                        QVector<long> values;
                        for (unsigned int k = 0; k < count; k++)
                            values.append(snd_ctl_elem_value_get_integer(cvalue,k));
                        mxItem = CMixerItem(numid,name,values,
                                            snd_ctl_elem_info_get_min(cinfo),
                                            snd_ctl_elem_info_get_max(cinfo),
                                            snd_ctl_elem_info_get_step(cinfo));
                        break;
                    }
                    case SND_CTL_ELEM_TYPE_INTEGER64: {
                        QVector<long long> values;
                        for (unsigned int k = 0; k < count; k++)
                            values.append(snd_ctl_elem_value_get_integer64(cvalue,k));
                        mxItem = CMixerItem(numid,name,values,
                                            snd_ctl_elem_info_get_min64(cinfo),
                                            snd_ctl_elem_info_get_max64(cinfo),
                                            snd_ctl_elem_info_get_step64(cinfo));
                        break;
                    }
                    case SND_CTL_ELEM_TYPE_ENUMERATED: {
                        unsigned int cnt = snd_ctl_elem_info_get_items(cinfo);
                        QStringList labels;
                        for (unsigned int k = 0; k < cnt; k++) {
                            snd_ctl_elem_info_set_item(cinfo,k);
                            snd_ctl_elem_info(ctl,cinfo);
                            labels.append(QString::fromUtf8(snd_ctl_elem_info_get_item_name(cinfo)));
                        }
                        QVector<unsigned int> values;
                        for (unsigned int k = 0; k < count; k++)
                            values.append(snd_ctl_elem_value_get_enumerated(cvalue,k));
                        mxItem = CMixerItem(numid,name,values,labels);
                        break;
                    }
                    default: break;
                    }

                    // Postprocessing
                    if (!mxItem.isEmpty()) {
                        mxItem.isUser = (snd_ctl_elem_info_is_user(cinfo) != 0);

                        res.append(mxItem);
                    }
                } // snd_ctl_elem_info_get_interface(cinfo) == SND_CTL_ELEM_IFACE_MIXER
            } // snd_ctl_elem_read
        } // snd_ctl_elem_info
        if (cid) free(cid);
        if (cinfo) free(cinfo);
        if (cvalue) free(cvalue);
    } // loop control list
    snd_ctl_elem_list_free_space(clist);

    // Sorting and binding
    std::sort(res.begin(),res.end(),ZAlsaBackendPrivate::lessThanMixerItem);
    for (int i=0; i<res.count(); i++) {
        if ((res.at(i).type == CMixerItem::itInteger) || (res.at(i).type == CMixerItem::itInteger64)) {
            int score = 0;
            const auto relList = d->findRelatedMixerItems(res.at(i),res,&score);
            res[i].related = relList;
            res[i].relatedNameLength = score;
            for (const auto &idx : relList)
                res[idx].isRelated = true;
        }
    }

    return res;
}

void ZAlsaBackend::setMixerControl(int cardNum, const CMixerItem &item)
{
    Q_D(ZAlsaBackend);

    int err;

    if (item.isEmpty()) return;

    snd_ctl_t* ctl = d->getMixerCtl(cardNum);
    if (ctl == nullptr) return;

    snd_ctl_elem_info_t* cinfo;
    snd_ctl_elem_info_alloca(&cinfo);
    snd_ctl_elem_value_t* cvalue;
    snd_ctl_elem_value_alloca(&cvalue);

    snd_ctl_elem_info_set_numid(cinfo,item.numid);
    if ((err = snd_ctl_elem_info(ctl,cinfo)) < 0) {
        WARN(QSL("snd_ctl_elem_info: %1").arg(QString::fromUtf8(snd_strerror(err))));

    } else if (static_cast<int>(snd_ctl_elem_info_get_count(cinfo)) == item.values.count()) {
        unsigned int type = snd_ctl_elem_info_get_type(cinfo);
        snd_ctl_elem_value_set_numid(cvalue,item.numid);

        if ((err = snd_ctl_elem_read(ctl,cvalue)) < 0) {
            WARN(QSL("snd_ctl_elem_read: %1").arg(QString::fromUtf8(snd_strerror(err))));

        } else {
            bool needWrite = false;
            if ((item.type == CMixerItem::itBoolean) && (type == SND_CTL_ELEM_TYPE_BOOLEAN)) {
                for (int i=0; i<item.values.count(); i++) {
                    snd_ctl_elem_value_set_boolean(cvalue,static_cast<unsigned int>(i),static_cast<long>(item.values.at(i)));
                }
                needWrite = true;
            } else if ((item.type == CMixerItem::itInteger) && (type == SND_CTL_ELEM_TYPE_INTEGER)) {
                for (int i=0; i<item.values.count(); i++) {
                    snd_ctl_elem_value_set_integer(cvalue,static_cast<unsigned int>(i),static_cast<long>(item.values.at(i)));
                }
                needWrite = true;
            } else if ((item.type == CMixerItem::itInteger64) && (type == SND_CTL_ELEM_TYPE_INTEGER64)) {
                for (int i=0; i<item.values.count(); i++) {
                    snd_ctl_elem_value_set_integer64(cvalue,static_cast<unsigned int>(i),static_cast<long long>(item.values.at(i)));
                }
                needWrite = true;
            } else if ((item.type == CMixerItem::itEnumerated) && (type == SND_CTL_ELEM_TYPE_ENUMERATED)) {
                for (int i=0; i<item.values.count(); i++) {
                    snd_ctl_elem_value_set_enumerated(cvalue,static_cast<unsigned int>(i),static_cast<unsigned int>(item.values.at(i)));
                }
                needWrite = true;
            }

            if (needWrite) {
                if ((err = snd_ctl_elem_write(ctl,cvalue)) < 0)
                    WARN(QSL("snd_ctl_elem_write: %1").arg(QString::fromUtf8(snd_strerror(err))));
            }
        }
    }
}

void ZAlsaBackend::deleteMixerControl(int cardNum, const CMixerItem &item)
{
    Q_D(ZAlsaBackend);

    int err;

    if (item.isEmpty() || !item.isUser) return;

    snd_ctl_t* ctl = d->getMixerCtl(cardNum);
    if (ctl == nullptr) return;

    snd_ctl_elem_id_t* cid;
    snd_ctl_elem_id_alloca(&cid);
    snd_ctl_elem_info_t* cinfo;
    snd_ctl_elem_info_alloca(&cinfo);
    snd_ctl_elem_value_t* cvalue;
    snd_ctl_elem_value_alloca(&cvalue);

    snd_ctl_elem_info_set_numid(cinfo,item.numid);
    if ((err = snd_ctl_elem_info(ctl,cinfo)) < 0) {
        WARN(QSL("snd_ctl_elem_info: %1").arg(QString::fromUtf8(snd_strerror(err))));

    } else if (static_cast<int>(snd_ctl_elem_info_get_count(cinfo)) == item.values.count()) {
        unsigned int type = snd_ctl_elem_info_get_type(cinfo);
        if (((type == SND_CTL_ELEM_TYPE_BOOLEAN) && (item.type == CMixerItem::itBoolean)) ||
                ((type == SND_CTL_ELEM_TYPE_INTEGER) && (item.type == CMixerItem::itInteger)) ||
                ((type == SND_CTL_ELEM_TYPE_INTEGER64) && (item.type == CMixerItem::itInteger64)) ||
                ((type == SND_CTL_ELEM_TYPE_ENUMERATED) && (item.type == CMixerItem::itEnumerated))) {

            snd_ctl_elem_id_set_numid(cid,item.numid);
            if ((err = snd_ctl_elem_remove(ctl,cid)) < 0)
                WARN(QSL("snd_ctl_elem_read: %1").arg(QString::fromUtf8(snd_strerror(err))));
        }
    }
}

QStringList ZAlsaBackend::getAlsaWarnings()
{
    Q_D(ZAlsaBackend);

    auto res = d->m_alsaWarnings;
    d->m_alsaWarnings.clear();
    return res;
}

bool ZAlsaBackend::isWarnings()
{
    Q_D(ZAlsaBackend);
    return !d->m_alsaWarnings.isEmpty();
}
