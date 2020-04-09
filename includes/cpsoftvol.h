#ifndef ZCPSOFTVOL_H
#define ZCPSOFTVOL_H

#include <QtCore>
#include "cpbase.h"

class ZCPSoftvol : public ZCPBase
{
    Q_OBJECT
private:
    ZCPInput* fInp { nullptr };
    ZCPOutput* fOut { nullptr };
    ZCPOutput* fCtlOut { nullptr };

    QString m_name;
    int m_channels { 2 };
    double m_min_dB { -51.0 };
    double m_max_dB { 0.0 };
    int m_resolution { 256 };

public:
    ZCPSoftvol(QWidget *parent, ZRenderArea *aOwner);
    ~ZCPSoftvol() override;

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

#endif // ZCPSOFTVOL_H
