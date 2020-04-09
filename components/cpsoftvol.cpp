#include "includes/cpsoftvol.h"
#include "includes/generic.h"
#include "includes/cphw.h"
#include "includes/softvoldialog.h"

ZCPSoftvol::ZCPSoftvol(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this, QSL("in"));
    registerInput(fInp);

    fOut=new ZCPOutput(this, QSL("out"));
    registerOutput(fOut);

    fCtlOut=new ZCPOutput(this, QSL("ctl"),CStructures::PinClass::pcCTL);
    registerOutput(fCtlOut);
}

ZCPSoftvol::~ZCPSoftvol() = default;

QSize ZCPSoftvol::minimumSizeHint() const
{
    return QSize(180,65);
}

void ZCPSoftvol::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/3);
    fCtlOut->relCoord=QPoint(width()-zcpPinSize/2,2*height()/3);
}

void ZCPSoftvol::doInfoGenerate(QTextStream &stream, QStringList &warnings) const
{
    stream << QSL("pcm.%1 {").arg(objectName()) << endl;
    stream << QSL("  type softvol") << endl;
    if (fOut->toFilter) {
        stream << QSL("  slave.pcm \"%1\"").arg(fOut->toFilter->objectName()) << endl;
    } else {
        warnings.append(tr("Softvol plugin: slave PCM not connected."));
    }

    auto hwCtl = qobject_cast<ZCPHW *>(fCtlOut->toFilter);
    if (m_name.isEmpty()) {
        warnings.append(tr("Softvol plugin: volume control not configured - empty control name."));
    } else if (hwCtl==nullptr) {
        warnings.append(tr("Softvol plugin: control must be connected to HW sink plugin."));
    } else {
        stream << QSL("  control {") << endl;
        stream << QSL("    name \"%1\"").arg(m_name) << endl;
        hwCtl->doCtlGenerate(stream,warnings,true);
        if (m_channels>0)
            stream << QSL("    count %1").arg(m_channels) << endl;
        stream << QSL("  }") << endl;
    }

    if (m_min_dB<0.0)
        stream << QSL("  min_dB %1").arg(m_min_dB,0,'f',1) << endl;
    if ((m_max_dB>m_min_dB) && (m_max_dB<90.0))
        stream << QSL("  max_dB %1").arg(m_max_dB,0,'f',1) << endl;
    if ((m_resolution>1) && (m_resolution<=1024))
        stream << QSL("  resolution %1").arg(m_resolution) << endl;

    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << endl;
    stream << endl;

}

void ZCPSoftvol::showSettingsDlg()
{
    ZSoftvolDialog dlg(window());
    dlg.setParams(m_name,m_min_dB,m_max_dB,m_resolution,m_channels);
    if (dlg.exec()==QDialog::Rejected) return;
    Q_EMIT componentChanged(this);
    dlg.getParams(m_name,m_min_dB,m_max_dB,m_resolution,m_channels);
    update();
}

void ZCPSoftvol::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    paintBase(p);

    setBaseFont(p,ftTitle);
    p.drawText(rect(),Qt::AlignCenter,QSL("Softvol"));

    setBaseFont(p,ftDesc);
    p.drawText(QRect(0,2*height()/3,width(),height()/3),Qt::AlignCenter,m_name);

    p.restore();
}

void ZCPSoftvol::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));
    m_name = json.toObject().value(QSL("name")).toString();
    m_channels = json.toObject().value(QSL("channels")).toInt(2);
    m_resolution = json.toObject().value(QSL("resolution")).toInt(256);
    m_min_dB = json.toObject().value(QSL("min_dB")).toDouble(-51.0);
    m_max_dB = json.toObject().value(QSL("max_dB")).toDouble(0.0);
}

QJsonValue ZCPSoftvol::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());
    data.insert(QSL("name"),m_name);
    data.insert(QSL("channels"),m_channels);
    data.insert(QSL("resolution"),m_resolution);
    data.insert(QSL("min_dB"),m_min_dB);
    data.insert(QSL("max_dB"),m_max_dB);
    return data;
}
