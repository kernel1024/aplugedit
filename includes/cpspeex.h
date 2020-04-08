#ifndef ZCPSPEEX_H
#define ZCPSPEEX_H

#include <QtCore>
#include <QtWidgets>
#include "cpbase.h"

namespace CDefaults {
    const int speexFrames = 64;
    const int speexAGCLevel = 8000;
    const int speexFilterLength = 256;
}

class ZCPSpeex : public ZCPBase
{
    Q_OBJECT
private:
    Qt::CheckState m_denoise { Qt::CheckState::Checked };
    Qt::CheckState m_agc { Qt::CheckState::PartiallyChecked };
    Qt::CheckState m_dereverb { Qt::CheckState::PartiallyChecked };
    Qt::CheckState m_echo { Qt::CheckState::PartiallyChecked };
    int m_frames { 0 };
    int m_agcLevel { 0 };
    int m_filterLength { 0 };
    // int m_dereverbLevel, int m_dereverbDecay - yet not used in speex now

    ZCPInput* fInp { nullptr };
    ZCPOutput* fOut { nullptr };
    ZCPOutput* fCtlOut { nullptr };

public:
    ZCPSpeex(QWidget *parent, ZRenderArea *aOwner);
    ~ZCPSpeex() override;

    void readFromJson(const QJsonValue& json) override;
    QJsonValue storeToJson() const override;

    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent * event) override;
    void realignPins() override;
    void doInfoGenerate(QTextStream & stream, QStringList & warnings) const override;
    void showSettingsDlg() override;
    bool needSettingsDlg() override { return true; }
};

#endif // ZCPSPEEX_H
