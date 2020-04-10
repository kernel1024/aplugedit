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
*   Parts of this code from ALSA project aplay.c utility.                 *
*                                                                         *
*   Copyright (c) by Jaroslav Kysela <perex@perex.cz>                     *
*   Based on vplay program by Michael Beck                                *
*                                                                         *
***************************************************************************/

#include <algorithm>
#include <QApplication>
#include "includes/generic.h"
#include "includes/alsabackend.h"

extern "C" {
#include <alsa/asoundlib.h>
}

#define WARN(MSG) m_alsaWarnings.append(QStringLiteral("%1: %2 (%3:%4)") \
    .arg(static_cast<const char *>(Q_FUNC_INFO)) \
    .arg(MSG) \
    .arg(static_cast<const char *>(__FILE__)) \
    .arg(__LINE__))

ZAlsaBackend::ZAlsaBackend(QObject *parent)
    : QObject(parent)
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
    enumerateCards();
}

void ZAlsaBackend::reloadGlobalConfig()
{
    snd_config_update_free_global();
    snd_config_update();
}

void ZAlsaBackend::snd_lib_error_handler(const char *file, int line, const char *function, int err, const char *fmt,...)
{
    va_list arg;
    va_start(arg, fmt);
    QString msg = QString::vasprintf(fmt,arg);
    if (err != 0)
        qWarning() << QSL(": %1").arg(QString::fromUtf8(snd_strerror(err)));
    va_end(arg);

    msg = QSL("ALSA: %1: %2 (%3:%4)")
            .arg(QString::fromUtf8(function),msg,QString::fromUtf8(file))
            .arg(line);

    auto alsa = gAlsa;
    if (alsa)
        Q_EMIT alsa->alsaErrorMsg(msg);
}

void ZAlsaBackend::setupErrorLogger()
{
    snd_lib_error_set_handler(&snd_lib_error_handler);
}

void ZAlsaBackend::enumerateCards()
{
    m_cards.clear();

    static snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;

    int card = -1;
    int err;
    int dev;
    snd_ctl_t *handle = nullptr;

    snd_ctl_card_info_t *info = nullptr;
    snd_pcm_info_t *pcminfo = nullptr;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);

    if (snd_card_next(&card) < 0 || card < 0) {
        WARN(QSL("no soundcards found!"));
        return;
    }
    // **** List of Hardware Devices ****
    while (card >= 0) {
        QString name = QSL("hw:%1").arg(card);

        if ((err = snd_ctl_open(&handle, name.toLatin1().constData(), 0)) < 0) {
            WARN(QSL("snd_ctl_open: %1").arg(QString::fromUtf8(snd_strerror(err))));
        } else {
            if ((err = snd_ctl_card_info(handle, info)) < 0) {
                WARN(QSL("snd_ctl_card_info: %1").arg(QString::fromUtf8(snd_strerror(err))));
            } else {

                m_cards << CCardItem(QString::fromUtf8(snd_ctl_card_info_get_id(info)),
                                     QString::fromUtf8(snd_ctl_card_info_get_name(info)),
                                     card);
                dev = -1;
                while (true) {
                    unsigned int count;
                    if (snd_ctl_pcm_next_device(handle, &dev)<0)
                        WARN(QSL("snd_ctl_pcm_next_device: %1").arg(QString::fromUtf8(snd_strerror(err))));
                    if (dev < 0)
                        break;
                    snd_pcm_info_set_device(pcminfo, static_cast<unsigned int>(dev));
                    snd_pcm_info_set_subdevice(pcminfo, 0);
                    snd_pcm_info_set_stream(pcminfo, stream);
                    if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                        if (err != -ENOENT)
                            WARN(QSL("snd_ctl_pcm_info: %1").arg(QString::fromUtf8(snd_strerror(err))));
                        continue;
                    }
                    count = snd_pcm_info_get_subdevices_count(pcminfo);
                    m_cards.last().devices << CDeviceItem(dev,static_cast<int>(count),
                                                          QString::fromUtf8(snd_pcm_info_get_name(pcminfo)));
                }
            }
            if (handle)
                snd_ctl_close(handle);
        }
        if (snd_card_next(&card) < 0)
            break;
    }
}

QVector<CCardItem> ZAlsaBackend::cards() const
{
    return m_cards;
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
    int err;

    QVector<CMixerItem> res;

    QString name = QSL("hw:%1").arg(cardNum);

    snd_ctl_t* ctl = nullptr;
    if ((err = snd_ctl_open(&ctl, name.toLatin1().constData(), 0)) < 0) {
        WARN(QSL("snd_ctl_open: %1").arg(QString::fromUtf8(snd_strerror(err))));
        return res;
    }

    snd_ctl_elem_list_t *clist = nullptr;
    snd_ctl_elem_list_alloca(&clist);
    if ((err = snd_ctl_elem_list(ctl, clist)) < 0) {
        WARN(QSL("snd_ctl_elem_list: %1").arg(QString::fromUtf8(snd_strerror(err))));
        snd_ctl_close(ctl);
        return res;
    }

    if ((err = snd_ctl_elem_list_alloc_space(clist, snd_ctl_elem_list_get_count(clist))) < 0) {
        WARN(QSL("snd_ctl_elem_list_alloc_space: %1").arg(QString::fromUtf8(snd_strerror(err))));
        snd_ctl_close(ctl);
        return res;
    }
    if ((err = snd_ctl_elem_list(ctl, clist)) < 0) {
        WARN(QSL("snd_ctl_elem_list: %1").arg(QString::fromUtf8(snd_strerror(err))));
        snd_ctl_close(ctl);
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
                        QVector<QString> labels;
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

    snd_ctl_close(ctl);

    // Sorting and binding
    std::sort(res.begin(),res.end(),lessThanMixerItem);
    for (int i=0; i<res.count(); i++) {
        if ((res.at(i).type == CMixerItem::itInteger) || (res.at(i).type == CMixerItem::itInteger64)) {
            int score = 0;
            const auto relList = findRelatedMixerItems(res.at(i),res,&score);
            res[i].related = relList;
            res[i].relatedNameLength = score;
            for (const auto &idx : relList)
                res[idx].isRelated = true;
        }
    }

    return res;
}

bool ZAlsaBackend::lessThanMixerItem(const CMixerItem &a, const CMixerItem &b)
{
    static const QStringList names({ QSL("Master"),
                                     QSL("Headphone"),
                                     QSL("Speaker"),
                                     QSL("Tone"),
                                     QSL("Bass"),
                                     QSL("Treble"),
                                     QSL("3D Control"),
                                     QSL("PCM"),
                                     QSL("Front"),
                                     QSL("Surround"),
                                     QSL("Center"),
                                     QSL("LFE"),
                                     QSL("Side"),
                                     QSL("Synth"),
                                     QSL("FM"),
                                     QSL("Wave"),
                                     QSL("Music"),
                                     QSL("DSP"),
                                     QSL("Line"),
                                     QSL("CD"),
                                     QSL("Mic"),
                                     QSL("Digital"),
                                     QSL("Video"),
                                     QSL("Zoom Video"),
                                     QSL("Phone"),
                                     QSL("I2S"),
                                     QSL("IEC958"),
                                     QSL("PC Speaker"),
                                     QSL("Beep"),
                                     QSL("Aux"),
                                     QSL("Mono"),
                                     QSL("Playback"),
                                     QSL("Capture"),
                                     QSL("Mix"),
                                   });

    static const QStringList subNames({ QSL("Mono"),
                                        QSL("Digital"),
                                        QSL("Switch"),
                                        QSL("Depth"),
                                        QSL("Wide"),
                                        QSL("Space"),
                                        QSL("Level"),
                                        QSL("Center"),
                                        QSL("Output"),
                                        QSL("Boost"),
                                        QSL("Tone"),
                                        QSL("Bass"),
                                        QSL("Treble"),
                                      });

    int weightA = names.count();
    int weightB = names.count();
    for (int i=0; i<names.count(); i++) {
        if ((weightA>i) && (a.name.startsWith(names.at(i))))
            weightA = i;
        if ((weightB>i) && (b.name.startsWith(names.at(i))))
            weightB = i;
    }
    if (weightA == weightB) {
        int weightA = subNames.count();
        int weightB = subNames.count();
        for (int i=0; i<subNames.count(); i++) {
            if ((weightA>i) && (a.name.endsWith(subNames.at(i))))
                weightA = i;
            if ((weightB>i) && (b.name.endsWith(subNames.at(i))))
                weightB = i;
        }
    }

    return weightA < weightB;
}

QVector<int> ZAlsaBackend::findRelatedMixerItems(const CMixerItem &base, const QVector<CMixerItem> &items, int* topScore)
{
    QVector<QPair<int,int> > scores;
    int maxScore = 0;
    for (int i=0; i<items.count(); i++) {
        if (base.numid == items.at(i).numid) continue;
        if (items.at(i).isRelated) continue;
        if (items.at(i).type != CMixerItem::itBoolean) continue;

        const QString a = base.name;
        const QString b = items.at(i).name;
        int maxLen = qMin(a.length(),b.length());
        int score = 0;
        while ((score<maxLen) && (a.at(score) == b.at(score)))
            score++;

        maxScore = qMax(maxScore,score);

        scores.append(qMakePair(i,score));
    }

    QVector<int> res;
    if (maxScore < base.name.length()/2) return res;
    if (topScore)
        *topScore = maxScore;

    for (const auto &pair : qAsConst(scores)) {
        if (pair.second == maxScore)
            res.append(pair.first);
    }

    return res;
}

void ZAlsaBackend::setMixerControl(int cardNum, const CMixerItem &item)
{
    int err;

    if (item.isEmpty()) return;

    QString name = QSL("hw:%1").arg(cardNum);

    snd_ctl_t* ctl = nullptr;
    if ((err = snd_ctl_open(&ctl, name.toLatin1().constData(), 0)) < 0) {
        WARN(QSL("snd_ctl_open: %1").arg(QString::fromUtf8(snd_strerror(err))));
        return;
    }

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

    snd_ctl_close(ctl);
}

void ZAlsaBackend::deleteMixerControl(int cardNum, const CMixerItem &item)
{
    int err;

    if (item.isEmpty() || !item.isUser) return;

    QString name = QSL("hw:%1").arg(cardNum);

    snd_ctl_t* ctl = nullptr;
    if ((err = snd_ctl_open(&ctl, name.toLatin1().constData(), 0)) < 0) {
        WARN(QSL("snd_ctl_open: %1").arg(QString::fromUtf8(snd_strerror(err))));
        return;
    }

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

    snd_ctl_close(ctl);
}

CCardItem::CCardItem(const QString &aCardID, const QString &aCardName, int aCardNum)
{
    cardName = aCardName;
    cardNum = aCardNum;
    cardID = aCardID;
}

CDeviceItem::CDeviceItem(int aDevNum, int aSubdevices, const QString& aName)
{
    devNum = aDevNum;
    subdevices = aSubdevices;
    devName = aName;
}

CPCMItem::CPCMItem(const QString &aName)
{
    name = aName;
}

CPCMItem::CPCMItem(const QString &aName, const QStringList &aDescription)
{
    name = aName;
    description = aDescription;
}

bool CPCMItem::operator==(const CPCMItem &s) const
{
    return (name == s.name);
}

bool CPCMItem::operator!=(const CPCMItem &s) const
{
    return !operator==(s);
}

CMixerItem::CMixerItem(unsigned int aNumid, const QString& aName, const QVector<int> aValues)
{
    type = itBoolean;
    numid = aNumid;
    name = aName;
    for (const auto &v : aValues)
        values.append(v);
}

CMixerItem::CMixerItem(unsigned int aNumid, const QString& aName, const QVector<long> aValues,
                       long min, long max, long step)
{
    type = itInteger;
    numid = aNumid;
    name = aName;
    valueMin = min;
    valueMax = max;
    valueStep = step;
    for (const auto &v : aValues)
        values.append(v);
}

CMixerItem::CMixerItem(unsigned int aNumid, const QString& aName, const QVector<long long> aValues,
                       long long min, long long max, long long step)
{
    type = itInteger64;
    numid = aNumid;
    name = aName;
    valueMin = min;
    valueMax = max;
    valueStep = step;
    values = aValues;
}

CMixerItem::CMixerItem(unsigned int aNumid, const QString &aName, const QVector<unsigned int> aValues, const QVector<QString> aLabels)
{
    type = itEnumerated;
    numid = aNumid;
    name = aName;
    labels = aLabels;
    for (const auto &v : aValues)
        values.append(v);
}

bool CMixerItem::isEmpty() const
{
    return (numid == 0);
}

bool CMixerItem::operator==(const CMixerItem &s) const
{
    return (numid == s.numid);
}

bool CMixerItem::operator!=(const CMixerItem &s) const
{
    return !operator==(s);
}
