#ifndef ZCPBLACKLIST_H
#define ZCPBLACKLIST_H

#include <QtCore>
#include "cpbase.h"

class ZCPBlacklist : public ZCPBase
{
    Q_OBJECT
private:
    QStringList m_PCMs;

public:
    ZCPBlacklist(QWidget *parent, ZRenderArea *aOwner);
    ~ZCPBlacklist() override;

    void readFromJson(const QJsonValue& json) override;
    QJsonValue storeToJson() const override;

    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent * event) override;
    void doInfoGenerate(QTextStream & stream, QStringList & warnings) const override;
    void showSettingsDlg() override;

};

#endif // ZCPBLACKLIST_H
