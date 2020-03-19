#include "includes/cpblacklist.h"
#include "includes/generic.h"

ZCPBlacklist::ZCPBlacklist(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent, aOwner)
{
}

ZCPBlacklist::~ZCPBlacklist() = default;

QSize ZCPBlacklist::minimumSizeHint() const
{
    return QSize(140,50);
}

void ZCPBlacklist::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    paintBase(p);

    setBaseFont(p,ftTitle);
    p.drawText(rect(),Qt::AlignCenter,QSL("Blacklist"));

    setBaseFont(p,ftDesc);
    p.drawText(QRect(0,2*height()/3,width(),height()/3),Qt::AlignCenter,
               tr("%1 PCMs").arg(m_PCMs.count()));

    p.restore();
}

void ZCPBlacklist::readFromJson(const QJsonValue &json)
{

}

QJsonValue ZCPBlacklist::storeToJson() const
{

}

void ZCPBlacklist::doInfoGenerate(QTextStream &stream, QStringList &warnings) const
{

}

void ZCPBlacklist::showSettingsDlg()
{
    // TODO: get PCM list and show dialog.

}
