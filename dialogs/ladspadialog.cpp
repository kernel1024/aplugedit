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
*                                                                         *
***************************************************************************
*                                                                         *
*   Code in this module is partially from sources of LADSPA SDK tools     *
*   (http://www.ladspa.org/), written by Richard W.E. Furse.              *
*                                                                         *
***************************************************************************/

#include <QtWidgets>
#include <QtCore>
//#include <sys/types.h>
//#include <unistd.h>
#include <cmath>
#include <cfloat>
#include <ladspa.h>
#include "includes/ladspadialog.h"
#include "includes/cpbase.h"

ZLADSPADialog::ZLADSPADialog(QWidget *parent, int aSampleRate)
    : QDialog(parent)
{
    setupUi(this);

    auto alScroller=new QScrollArea(alTabSettings);
    alScroller->setWidgetResizable(true);

    alControls = new ZResizableFrame(alScroller);
    alControls->setObjectName(QSL("alControls"));
    alControls->setFrameShape(QFrame::StyledPanel);
    alControls->setFrameShadow(QFrame::Plain);
    if (auto vl=qobject_cast<QGridLayout*>(alTabSettings->layout()))
        vl->addWidget(alScroller,0,0,1,1);
    alControls->setMaximumSize(QSize(5*alTabSettings->width()/6,3000));
    alScroller->setWidget(alControls);

    vboxCLayout = new QVBoxLayout(alControls);
    vboxCLayout->setSpacing(6);
    vboxCLayout->setMargin(9);
    vboxCLayout->setObjectName(QSL("vboxCLayout"));

    alSampleRate=aSampleRate;

    connect(alPlugin,qOverload<int>(&QComboBox::currentIndexChanged),this,&ZLADSPADialog::changeLADSPA);
}

ZLADSPADialog::~ZLADSPADialog() = default;

ZResizableFrame::ZResizableFrame(QWidget *parent)
    : QFrame(parent)
{
    alScroller=qobject_cast<QScrollArea*>(parent);
}

QSize ZResizableFrame::minimumSizeHint() const
{
    QRect r(0,0,width(),0);
    for (int i=0;i<children().count();i++)
    {
        QWidget* w=qobject_cast<QWidget*>(children().at(i));
        if (!w) continue;
        if ((qobject_cast<QCheckBox*>(w)) || (qobject_cast<QDoubleSpinBox*>(w)))
        {
            r.adjust(0,0,0,7*w->sizeHint().height()/5);
        }
    }
    if (r.height()==0)
        return QSize(r.width(),r.height());

    return size().expandedTo(QSize(r.width(),r.height()));
}

QSize ZResizableFrame::sizeHint() const
{
    return minimumSizeHint();
}

void ZLADSPADialog::changeLADSPA(int index)
{
    selectedPlugin=index;
    analyzePlugin(index);
}

void ZLADSPADialog::setParams(const QString &plugLabel, const QString &plugID,
                              const ZLADSPAControlItems &controlItems)
{
    isShowed=false;
    m_preservedPlugLabel=plugLabel;
    m_preservedPlugID=plugID;
    m_preservedControlItems=controlItems;
}

void ZLADSPADialog::showEvent(QShowEvent * event)
{
    Q_UNUSED(event)

    if (isShowed) return;
    isShowed=true;
    scanPlugins();
    controlsRequested=true;
    if (lsPluginLabel.count()==0) {
        changeLADSPA(-1);
        return;
    }
    int idx = -1;
    if (!lsPluginLabel.isEmpty() && !m_preservedPlugID.isEmpty())
        idx = lsPluginLabel.indexOf(m_preservedPlugLabel);
    if ((idx==-1) || (lsPluginID.indexOf(m_preservedPlugID)!=idx)) {
        alPlugin->setCurrentIndex(0);
        changeLADSPA(0);
        return;
    }
    alPlugin->setCurrentIndex(idx);
    for (int i=0;i<m_preservedControlItems.count();i++) {
        if (i>=alCItems.count()) continue;
        if (alCItems.at(i).portName!=m_preservedControlItems.at(i).portName) continue;
        alCItems[i].aatType = m_preservedControlItems.at(i).aatType;
        alCItems[i].aasToggle = m_preservedControlItems.at(i).aasToggle;
        alCItems[i].aasValue = m_preservedControlItems.at(i).aasValue;
        alCItems[i].aasFreq = m_preservedControlItems.at(i).aasFreq;
        alCItems[i].aasInt = m_preservedControlItems.at(i).aasInt;
        if (alCItems.at(i).aatType==ZLADSPA::aacToggle) {
            if (auto w=qobject_cast<QCheckBox*>(alCItems.at(i).aawControl)) {
                if (alCItems.at(i).aasToggle) {
                    w->setCheckState(Qt::Checked);
                } else {
                    w->setCheckState(Qt::Unchecked);
                }
            }
        } else if (auto w=qobject_cast<QDoubleSpinBox*>(alCItems.at(i).aawControl)) {
            if (alCItems.at(i).aatType==ZLADSPA::aacInteger) {
                w->setValue(alCItems.at(i).aasInt);
            } else if (alCItems.at(i).aatType==ZLADSPA::aacFreq) {
                w->setValue(alCItems.at(i).aasFreq);
            } else {
                w->setValue(alCItems.at(i).aasValue);
            }
        }
    }
}

void ZLADSPADialog::getParams(QString &plugLabel, QString &plugID, QString &plugName, QString &plugFile,
                              ZLADSPAControlItems &aCItems)
{
    plugLabel.clear();
    plugID.clear();
    plugName.clear();
    plugFile.clear();
    aCItems.clear();
    if (selectedPlugin==-1) return;

    plugLabel=lsPluginLabel.at(selectedPlugin);
    plugID=lsPluginID.at(selectedPlugin);
    plugName=lsPluginName.at(selectedPlugin);
    plugFile=lsPluginFile.at(selectedPlugin);
    for (const auto &item : qAsConst(alCItems)) {
        aCItems.append(item);
        aCItems.last().disconnectFromControls();
    }
}

void ZLADSPADialog::valueChanged(double value)
{
    Q_UNUSED(value)
    readInfoFromControls();
}

void ZLADSPADialog::stateChanged(int value)
{
    Q_UNUSED(value)
    readInfoFromControls();
}

void ZLADSPADialog::readInfoFromControls()
{
    for (int i=0;i<alCItems.count();i++) {
        if (alCItems.at(i).aawControl==nullptr) continue;
        if (alCItems.at(i).aatType==ZLADSPA::aacToggle) {
            if (auto w=qobject_cast<QCheckBox*>(alCItems.at(i).aawControl))
                alCItems[i].aasToggle=(w->checkState()==Qt::Checked);
        } else if (auto w=qobject_cast<QDoubleSpinBox*>(alCItems.at(i).aawControl)) {
            if (alCItems.at(i).aatType==ZLADSPA::aacInteger) {
                alCItems[i].aasInt=qRound(w->value());
            } else if (alCItems.at(i).aatType==ZLADSPA::aacFreq) {
                alCItems[i].aasFreq=qRound(w->value());
            } else {
                alCItems[i].aasValue=w->value();
            }
        } else {
            alCItems[i].aasToggle=false;
            alCItems[i].aasValue=0.0;
            alCItems[i].aasFreq=0;
            alCItems[i].aasInt=0;
        }
    }
}

void ZLADSPADialog::scanPlugins()
{
    alPlugin->clear();
    lsPluginFile.clear();
    lsPluginName.clear();
    lsPluginID.clear();
    lsPluginLabel.clear();
    alPluginInfo->clear();

    clearCItems();
    alControls->resize(sizeHint());

    static QString ladspa_path;
    if (ladspa_path.isEmpty()) {
        ladspa_path=qEnvironmentVariable("LADSPA_PATH");
        if (ladspa_path.isEmpty()) {
            ladspa_path = QSL("/usr/lib/ladspa:/usr/local/lib/ladspa");
            QMessageBox::warning(this,tr("LADSPA warning"),
                                 tr("Warning: You do not have a LADSPA_PATH environment variable set.\n"
                                    "Defaulting to /usr/lib/ladspa, /usr/local/lib/ladspa."));
        }
    }

    const QStringList ladspa_dirs=ladspa_path.split(':',QString::SkipEmptyParts);
    for (const auto &dir : ladspa_dirs) {
        QDir ladspa_dir(dir);
        ladspa_dir.setFilter(QDir::Files);
        for (int j=0;j<static_cast<int>(ladspa_dir.count());j++) {
            QLibrary plugin(ladspa_dir.filePath(ladspa_dir[j]));
            if (plugin.load()) {
                auto fDescriptorFunction = reinterpret_cast<LADSPA_Descriptor_Function>(plugin.resolve("ladspa_descriptor"));
                if (fDescriptorFunction) {
                    const LADSPA_Descriptor * psDescriptor;
                    for (unsigned long lIndex=0;(psDescriptor=fDescriptorFunction(lIndex))!=nullptr;lIndex++) {
                        QString pluginName = QString::fromUtf8(psDescriptor->Name);
                        QString pluginID = QSL("%1").arg(psDescriptor->UniqueID);
                        QString pluginLabel = QString::fromUtf8(psDescriptor->Label);
                        lsPluginFile << ladspa_dir.filePath(ladspa_dir[j]);
                        lsPluginName << pluginName;
                        lsPluginID << pluginID;
                        lsPluginLabel << pluginLabel;
                        alPlugin->addItem(QSL("%1 (%2/%3)").arg(pluginName,pluginID,pluginLabel));
                    }
                }
                plugin.unload();
            }
        }
    }
    selectedPlugin=-1;
}

// TODO: add LADSPA pin enumeration and config (for multichannel plugins)

void ZLADSPADialog::analyzePlugin(int index)
{
    const float nearZero = 0.00001F;
    alPluginInfo->clear();
    if (!controlsRequested) return;
    QString pinfo = QSL("<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\"white-space: pre-wrap;\">");

    clearCItems();
    alControls->resize(sizeHint());

    if (index==-1) return;
    QLibrary plugin(lsPluginFile[index]);
    if (plugin.load()) {
        auto pfDescriptorFunction = reinterpret_cast<LADSPA_Descriptor_Function>(plugin.resolve("ladspa_descriptor"));
        if (pfDescriptorFunction) {
            const LADSPA_Descriptor * psDescriptor;
            unsigned long lPluginIndex;
            unsigned long lPortIndex;
            LADSPA_PortRangeHintDescriptor iHintDescriptor;
            LADSPA_Data fBound;
            LADSPA_Data fDefault;
            QString sPluginLabel = lsPluginLabel[index];
            bool foundPlugin=false;

            for (lPluginIndex = 0;; lPluginIndex++) {
                psDescriptor = pfDescriptorFunction(lPluginIndex);
                if (!psDescriptor)
                    break;
                if (!sPluginLabel.isEmpty() && (sPluginLabel!=QString::fromUtf8(psDescriptor->Label)))
                    continue;
                foundPlugin=true;

                pinfo+=tr("Plugin Name: <b>\"%1\"</b><br/>").arg(QString::fromUtf8(psDescriptor->Name));
                pinfo+=tr("Plugin Label: <b>\"%1\"</b><br/>").arg(QString::fromUtf8(psDescriptor->Label));
                pinfo+=tr("Plugin Unique ID: <b>%1</b><br/>").arg(psDescriptor->UniqueID);
                pinfo+=tr("Maker: <b>\"%1\"</b><br/>").arg(QString::fromUtf8(psDescriptor->Maker));
                pinfo+=tr("Copyright: <b>\"%1\"</b><br/>").arg(QString::fromUtf8(psDescriptor->Copyright));
                pinfo+=tr("Plugin library: <b>\"%1\"</b><br/><br/>").arg(lsPluginFile[index]);

                if (LADSPA_IS_REALTIME(psDescriptor->Properties)) {
                    pinfo+=tr("Must Run Real-Time: <b>Yes</b><br/>");
                } else {
                    pinfo+=tr("Must Run Real-Time: <b>No</b><br/>");
                }
                if (psDescriptor->activate != nullptr) {
                    pinfo+=tr("Has activate() Function: <b>Yes</b><br/>");
                } else {
                    pinfo+=tr("Has activate() Function: <b>No</b><br/>");
                }
                if (psDescriptor->deactivate != nullptr) {
                    pinfo+=tr("Has deactivate() Function: <b>Yes</b><br/>");
                } else {
                    pinfo+=tr("Has deactivate() Function: <b>No</b><br/>");
                }
                if (psDescriptor->run_adding != nullptr) {
                    pinfo+=tr("Has run_adding() Function: <b>Yes</b><br/>");
                } else {
                    pinfo+=tr("Has run_adding() Function: <b>No</b><br/>");
                }

                if (psDescriptor->instantiate == nullptr)
                    pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN HAS NO INSTANTIATE FUNCTION.</font></b><br/>");
                if (psDescriptor->connect_port == nullptr)
                    pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN HAS NO CONNECT_PORT FUNCTION.</font></b><br/>");
                if (psDescriptor->run == nullptr)
                    pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN HAS NO RUN FUNCTION.</font></b><br/>");
                if (psDescriptor->run_adding != nullptr
                        && psDescriptor->set_run_adding_gain == nullptr)
                    pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN HAS RUN_ADDING FUNCTION BUT NOT SET_RUN_ADDING_GAIN.</font></b><br/>");
                if (psDescriptor->run_adding == nullptr
                        && psDescriptor->set_run_adding_gain != nullptr)
                    pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN HAS SET_RUN_ADDING_GAIN FUNCTION BUT NOT RUN_ADDING.</font></b><br/>");
                if (psDescriptor->cleanup == nullptr)
                    pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN HAS NO CLEANUP FUNCTION.</font></b><br/>");

                if (LADSPA_IS_HARD_RT_CAPABLE(psDescriptor->Properties)) {
                    pinfo+=tr("Environment: <b>Normal or Hard Real-Time</b><br/>");
                } else {
                    pinfo+=tr("Environment: <b>Normal</b><br/>");
                }

                if (LADSPA_IS_INPLACE_BROKEN(psDescriptor->Properties))
                    pinfo+=tr("<i>This plugin cannot use in-place processing. It will not work with all hosts.</i><br/>");
                pinfo+=QSL("<br/>");

                if (psDescriptor->PortCount == 0)
                    pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN HAS NO PORTS.</font></b><br/>");

                for (lPortIndex = 0;lPortIndex < psDescriptor->PortCount;lPortIndex++) {
                    if (!(LADSPA_IS_PORT_CONTROL(psDescriptor->PortDescriptors[lPortIndex]) &&
                          LADSPA_IS_PORT_INPUT(psDescriptor->PortDescriptors[lPortIndex])))
                        continue;

                    iHintDescriptor
                            = psDescriptor->PortRangeHints[lPortIndex].HintDescriptor;

                    if (LADSPA_IS_HINT_TOGGLED(iHintDescriptor)) {
                        if ((iHintDescriptor
                             | LADSPA_HINT_DEFAULT_0
                             | LADSPA_HINT_DEFAULT_1)
                                != (LADSPA_HINT_TOGGLED
                                    | LADSPA_HINT_DEFAULT_0
                                    | LADSPA_HINT_DEFAULT_1)) {
                            pinfo+=tr("<b><font color=\"#8B0000\">ERROR: TOGGLED INCOMPATIBLE WITH OTHER HINT</font></b><br/>");
                        } else {
                            auto acheckBox = new QCheckBox(tr("%1").arg(QString::fromUtf8(psDescriptor->PortNames[lPortIndex])),
                                                           alControls);
                            bool toggledState=false;
                            if (LADSPA_IS_HINT_DEFAULT_1(iHintDescriptor)) toggledState=true;
                            alCItems << ZLADSPAControlItem(tr("%1").arg(QString::fromUtf8(psDescriptor->PortNames[lPortIndex])),
                                                           ZLADSPA::aacToggle,toggledState,0.0,acheckBox,nullptr,nullptr);
                            acheckBox->setObjectName(tr("checkBox#%1").arg(alCItems.size()-1));
                            alControls->layout()->addWidget(acheckBox);
                            connect(acheckBox,SIGNAL(stateChanged(int)),this,SLOT(stateChanged(int)));
                        }
                    } else {
                        auto ahboxLayout = new QHBoxLayout();
                        ahboxLayout->setSpacing(6);
                        ahboxLayout->setMargin(0);
                        auto alabel=new QLabel(alControls);
                        if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
                            alabel->setText(tr("%1 (in dB)").arg(QString::fromUtf8(psDescriptor->PortNames[lPortIndex])));
                        } else if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor)) {
                            alabel->setText(tr("%1 (in Hz)").arg(QString::fromUtf8(psDescriptor->PortNames[lPortIndex])));
                        } else {
                            alabel->setText(tr("%1").arg(QString::fromUtf8(psDescriptor->PortNames[lPortIndex])));
                        }
                        auto aspinBox=new QDoubleSpinBox(alControls);
                        QString pname=tr("%1").arg(QString::fromUtf8(psDescriptor->PortNames[lPortIndex]));

                        ZLADSPA::Control ciType;
                        if (LADSPA_IS_HINT_INTEGER(iHintDescriptor)) {
                            ciType=ZLADSPA::aacInteger;
                        } else if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
                            ciType=ZLADSPA::aacLogarithmic;
                        } else if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor)) {
                            ciType=ZLADSPA::aacFreq;
                        } else {
                            ciType=ZLADSPA::aacLinear;
                        }
                        alCItems << ZLADSPAControlItem(pname,ciType,false,0.0,aspinBox,ahboxLayout,alabel);
                        ahboxLayout->setObjectName(tr("hboxlayout#%1").arg(alCItems.size()-1));
                        alabel->setObjectName(tr("alabel#%1").arg(alCItems.size()-1));
                        aspinBox->setObjectName(tr("aspinBox#%1").arg(alCItems.size()-1));

                        alabel->setBuddy(aspinBox);
                        ahboxLayout->addWidget(alabel);
                        ahboxLayout->addWidget(aspinBox);
                        aspinBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed));
                        vboxCLayout->addLayout(ahboxLayout);
                        connect(aspinBox,SIGNAL(valueChanged(double)),this,SLOT(valueChanged(double)));

                        aspinBox->setValue(0.0);
                        if (LADSPA_IS_HINT_INTEGER(iHintDescriptor)) {
                            aspinBox->setDecimals(0);
                            aspinBox->setMinimum(-1000);
                            aspinBox->setMaximum(1000);
                            aspinBox->setSingleStep(1);
                            if (LADSPA_IS_HINT_BOUNDED_BELOW(iHintDescriptor)) {
                                aspinBox->setMinimum(static_cast<double>(
                                                         truncf(psDescriptor->PortRangeHints[lPortIndex].LowerBound)));
                            }
                            if (LADSPA_IS_HINT_BOUNDED_ABOVE(iHintDescriptor)) {
                                aspinBox->setMaximum(static_cast<double>(
                                                         truncf(psDescriptor->PortRangeHints[lPortIndex].UpperBound)));
                            }
                        } else {
                            aspinBox->setMaximum(3.3e+37);
                            if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor)) {
                                aspinBox->setDecimals(0);
                                aspinBox->setSingleStep(1);
                                aspinBox->setMinimum(0);
                            } else {
                                aspinBox->setDecimals(2);
                                aspinBox->setSingleStep(0.01);
                                aspinBox->setMinimum(-3.3e+37);
                            }
                            if (LADSPA_IS_HINT_BOUNDED_BELOW(iHintDescriptor)) {
                                fBound = psDescriptor->PortRangeHints[lPortIndex].LowerBound;
                                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fBound)>nearZero) {
                                    aspinBox->setMinimum(static_cast<double>(fBound*alSampleRate));
                                } else {
                                    aspinBox->setMinimum(static_cast<double>(fBound));
                                }
                            }
                            if (LADSPA_IS_HINT_BOUNDED_ABOVE(iHintDescriptor)) {
                                fBound = psDescriptor->PortRangeHints[lPortIndex].UpperBound;
                                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fBound)>nearZero) {
                                    aspinBox->setMaximum(static_cast<double>(fBound*alSampleRate));
                                } else {
                                    aspinBox->setMaximum(static_cast<double>(fBound));
                                }
                            }
                        }
                        switch (iHintDescriptor & LADSPA_HINT_DEFAULT_MASK) {
                            case LADSPA_HINT_DEFAULT_NONE:
                                break;
                            case LADSPA_HINT_DEFAULT_MINIMUM:
                                fDefault = psDescriptor->PortRangeHints[lPortIndex].LowerBound;
                                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(fDefault*alSampleRate));
                                } else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(truncf(fDefault)));
                                } else {
                                    aspinBox->setValue(static_cast<double>(fDefault));
                                }
                                break;
                            case LADSPA_HINT_DEFAULT_LOW:
                                if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
                                    fDefault = expf(logf(psDescriptor->PortRangeHints[lPortIndex].LowerBound) * 0.75F
                                                    + logf(psDescriptor->PortRangeHints[lPortIndex].UpperBound) * 0.25F);
                                } else {
                                    fDefault = (psDescriptor->PortRangeHints[lPortIndex].LowerBound * 0.75F
                                                + psDescriptor->PortRangeHints[lPortIndex].UpperBound * 0.25F);
                                }
                                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(fDefault*alSampleRate));
                                } else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(truncf(fDefault)));
                                } else {
                                    aspinBox->setValue(static_cast<double>(fDefault));
                                }
                                break;
                            case LADSPA_HINT_DEFAULT_MIDDLE:
                                if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
                                    fDefault = sqrtf(psDescriptor->PortRangeHints[lPortIndex].LowerBound
                                                     * psDescriptor->PortRangeHints[lPortIndex].UpperBound);
                                } else {
                                    fDefault = 0.5F * (psDescriptor->PortRangeHints[lPortIndex].LowerBound
                                                       + psDescriptor->PortRangeHints[lPortIndex].UpperBound);
                                }
                                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(fDefault*alSampleRate));
                                } else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(truncf(fDefault)));
                                } else {
                                    aspinBox->setValue(static_cast<double>(fDefault));
                                }
                                break;
                            case LADSPA_HINT_DEFAULT_HIGH:
                                if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
                                    fDefault = expf(logf(psDescriptor->PortRangeHints[lPortIndex].LowerBound) * 0.25F
                                                    + logf(psDescriptor->PortRangeHints[lPortIndex].UpperBound) * 0.75F);
                                } else {
                                    fDefault = (psDescriptor->PortRangeHints[lPortIndex].LowerBound * 0.25F
                                                + psDescriptor->PortRangeHints[lPortIndex].UpperBound * 0.75F);
                                }
                                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(fDefault*alSampleRate));
                                } else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(truncf(fDefault)));
                                } else {
                                    aspinBox->setValue(static_cast<double>(fDefault));
                                }
                                break;
                            case LADSPA_HINT_DEFAULT_MAXIMUM:
                                fDefault = psDescriptor->PortRangeHints[lPortIndex].UpperBound;
                                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(fDefault*alSampleRate));
                                } else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(truncf(fDefault)));
                                } else {
                                    aspinBox->setValue(static_cast<double>(fDefault));
                                }
                                break;
                            case LADSPA_HINT_DEFAULT_0:
                                fDefault=0.0F;
                                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(fDefault*alSampleRate));
                                } else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(truncf(fDefault)));
                                } else {
                                    aspinBox->setValue(static_cast<double>(fDefault));
                                }
                                break;
                            case LADSPA_HINT_DEFAULT_1:
                                fDefault=1.0F;
                                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(fDefault*alSampleRate));
                                } else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(truncf(fDefault)));
                                } else {
                                    aspinBox->setValue(static_cast<double>(fDefault));
                                }
                                break;
                            case LADSPA_HINT_DEFAULT_100:
                                fDefault=100.0F;
                                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(fDefault*alSampleRate));
                                } else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(truncf(fDefault)));
                                } else {
                                    aspinBox->setValue(static_cast<double>(fDefault));
                                }
                                break;
                            case LADSPA_HINT_DEFAULT_440:
                                fDefault=440.0F;
                                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(fDefault*alSampleRate));
                                } else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && abs(fDefault)>nearZero) {
                                    aspinBox->setValue(static_cast<double>(truncf(fDefault)));
                                } else {
                                    aspinBox->setValue(static_cast<double>(fDefault));
                                }
                                break;
                        } // switch (iHintDescriptor & LADSPA_HINT_DEFAULT_MASK)

                        if (LADSPA_IS_HINT_INTEGER(iHintDescriptor)) {
                            alCItems.last().aasInt=qRound(aspinBox->value());
                        } else if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor)) {
                            alCItems.last().aasFreq=qRound(aspinBox->value());
                        } else {
                            alCItems.last().aasValue=aspinBox->value();
                        }
                    } // else ... (LADSPA_IS_HINT_TOGGLED(iHintDescriptor))

                } // for (port enumeration)
                break;
            } // for (plugin search)
            if (!foundPlugin)
                pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN NOT FOUND IN THIS LIBRARY.</font></b><br/>");
        } else {// if (fDescriptorFunction correct)
            pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN INCORRECT (LADSPA_DESCRIPTOR SYMBOL NOT FOUND).</font></b><br/>");
        }
        plugin.unload();
    } else {// if (plugin.load())
        pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN LIBRARY NOT FOUND.</font></b><br/>");
    }
    pinfo+=tr("</body></html>");
    alPluginInfo->setHtml(pinfo);
    alControls->resize(sizeHint());
    alControls->update();

    // update element placement in vboxCLayout
    static bool updresize=false;
    if (updresize) {
        resize(width()+2,height());
    } else {
        resize(width()-2,height());
    }
    updresize=!updresize;
}

void ZLADSPADialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    clearCItems();
}

void ZLADSPADialog::clearCItems()
{
    for (auto & item : alCItems)
        item.destroyControls();

    alCItems.clear();
}

/* --------- ZLADSPAControlItem ------------*/

ZLADSPAControlItem::ZLADSPAControlItem(const QString &AportName, ZLADSPA::Control AaatType,
                                       bool AaasToggle, double AaasValue, QWidget* AaawControl,
                                       QLayout* AaawLayout, QLabel* AaawLabel)
{
    aatType=AaatType;
    aasToggle=AaasToggle;
    aasValue=AaasValue;
    aasFreq=qRound(aasValue);
    aasInt=qRound(aasValue);
    portName=AportName;
    aawControl=AaawControl;
    aawLayout=AaawLayout;
    aawLabel=AaawLabel;
}

QJsonValue ZLADSPAControlItem::storeToJson() const
{
    QJsonObject data;

    auto typeEnum = QMetaEnum::fromType<ZLADSPA::Control>();
    data.insert(QSL("type"),typeEnum.valueToKey(aatType));

    data.insert(QSL("toggle"),static_cast<int>(aasToggle));
    data.insert(QSL("value"),aasValue);
    data.insert(QSL("freq"),aasFreq);
    data.insert(QSL("int"),aasInt);
    data.insert(QSL("portName"),portName);

    return data;
}

void ZLADSPAControlItem::destroyControls()
{
    if (aawControl)
        aawControl->deleteLater();
    if (aawLayout)
        aawLayout->deleteLater();
    if (aawLabel)
        aawLabel->deleteLater();
    disconnectFromControls();
}

void ZLADSPAControlItem::disconnectFromControls()
{
    aawControl=nullptr;
    aawLayout=nullptr;
    aawLabel=nullptr;
}

ZLADSPAControlItem::ZLADSPAControlItem(QDataStream &s)
{
    s.readRawData(reinterpret_cast<char*>(&(aatType)),sizeof(aatType));
    s >> aasToggle >> aasValue >> aasFreq >> aasInt >> portName;
}

ZLADSPAControlItem::ZLADSPAControlItem(const QJsonValue &json)
{
    auto typeEnum = QMetaEnum::fromType<ZLADSPA::Control>();
    aatType = static_cast<ZLADSPA::Control>(typeEnum.keyToValue(json.toObject().value(QSL("type"))
                                                                .toString().toLatin1().constData()));

    aasToggle = static_cast<bool>(json.toObject().value(QSL("toggle")).toInt(0));
    aasValue = json.toObject().value(QSL("value")).toDouble(0.0);
    aasFreq = json.toObject().value(QSL("freq")).toInt(0);
    aasInt = json.toObject().value(QSL("int")).toInt(0);
    portName = json.toObject().value(QSL("portName")).toString();
}
