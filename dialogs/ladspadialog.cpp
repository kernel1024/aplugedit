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

#include <cmath>
#include <cfloat>
#include <algorithm>
#include <ladspa.h>
#include "includes/generic.h"
#include "includes/ladspadialog.h"

ZLADSPADialog::ZLADSPADialog(QWidget *parent, int channels, int sampleRate)
    : QDialog(parent)
{
    setupUi(this);

    auto alScroller=new QScrollArea(tabSettings);
    alScroller->setWidgetResizable(true);

    m_controls = new ZResizableFrame(alScroller);
    m_controls->setObjectName(QSL("alControls"));
    m_controls->setFrameShape(QFrame::StyledPanel);
    m_controls->setFrameShadow(QFrame::Plain);
    if (auto vl=qobject_cast<QGridLayout*>(tabSettings->layout()))
        vl->addWidget(alScroller,0,0,1,1);
    m_controls->setMaximumSize(QSize(5*tabSettings->width()/6,3000));
    alScroller->setWidget(m_controls);

    m_vboxLayout = new QVBoxLayout(m_controls);
    m_vboxLayout->setSpacing(6);
    m_vboxLayout->setMargin(9);
    m_vboxLayout->setObjectName(QSL("vboxCLayout"));

    m_sampleRate = sampleRate;
    m_channels = channels;

    m_inputsModel = new ZLADSPABindingsModel(this);
    m_outputsModel = new ZLADSPABindingsModel(this);
    tableInputs->setModel(m_inputsModel);
    tableOutputs->setModel(m_outputsModel);
    tableInputs->setItemDelegateForColumn(1, new ZLADSPAPortEditDelegate(this));
    tableOutputs->setItemDelegateForColumn(1, new ZLADSPAPortEditDelegate(this));

    checkPolicy->setCheckState(Qt::PartiallyChecked);

    connect(comboPlugin,qOverload<int>(&QComboBox::currentIndexChanged),this,&ZLADSPADialog::comboPluginIndexChanged);
    connect(buttonAddInput,&QPushButton::clicked,this,&ZLADSPADialog::addInputBinding);
    connect(buttonDeleteInput,&QPushButton::clicked,this,&ZLADSPADialog::deleteInputBinding);
    connect(buttonAddOutput,&QPushButton::clicked,this,&ZLADSPADialog::addOutputBinding);
    connect(buttonDeleteOutput,&QPushButton::clicked,this,&ZLADSPADialog::deleteOutputBinding);
    connect(checkPolicy,&QCheckBox::clicked,this,&ZLADSPADialog::policyChanged);
}

ZLADSPADialog::~ZLADSPADialog() = default;

void ZLADSPADialog::comboPluginIndexChanged(int index)
{
    if (index<0) {
        m_selectedPluginID = 0L;
    } else {
        m_selectedPluginID = comboPlugin->itemData(index).toLongLong();
    }
    analyzePlugin();
}

void ZLADSPADialog::setPlugItem(const CLADSPAPlugItem &item)
{
    m_isShowed=false;
    m_preservedPlugID=item.plugID;
    m_preservedControlItems=item.plugControls;

    if (!item.usePolicy) {
        checkPolicy->setCheckState(Qt::PartiallyChecked);
    } else if (item.policy==ZLADSPA::Policy::plDuplicate) {
        checkPolicy->setCheckState(Qt::Checked);
    } else {
        checkPolicy->setCheckState(Qt::Unchecked);
    }

    m_inputsModel->setBindings(item.inputBindings);
    m_outputsModel->setBindings(item.outputBindings);
}

void ZLADSPADialog::showEvent(QShowEvent * event)
{
    Q_UNUSED(event)

    if (m_isShowed) return;
    m_isShowed = true;

    scanPlugins();

    if (m_pluginLabel.isEmpty()) {
        comboPluginIndexChanged(-1);
        return;
    }

    int listIdx = comboPlugin->findData(m_preservedPlugID);
    if (listIdx < 0) {
        comboPlugin->setCurrentIndex(0);
        return;
    }

    comboPlugin->setCurrentIndex(listIdx);
    for (int i=0;i<m_preservedControlItems.count();i++) {
        if (i>=m_controlItems.count()) continue;
        if (m_controlItems.at(i).portName!=m_preservedControlItems.at(i).portName) continue;
        m_controlItems[i].aatType = m_preservedControlItems.at(i).aatType;
        m_controlItems[i].aasToggle = m_preservedControlItems.at(i).aasToggle;
        m_controlItems[i].aasValue = m_preservedControlItems.at(i).aasValue;
        m_controlItems[i].aasFreq = m_preservedControlItems.at(i).aasFreq;
        m_controlItems[i].aasInt = m_preservedControlItems.at(i).aasInt;
        if (m_controlItems.at(i).aatType==ZLADSPA::aacToggle) {
            if (auto w=qobject_cast<QCheckBox*>(m_controlItems.at(i).aawControl)) {
                if (m_controlItems.at(i).aasToggle) {
                    w->setCheckState(Qt::Checked);
                } else {
                    w->setCheckState(Qt::Unchecked);
                }
            }
        } else if (auto w=qobject_cast<QDoubleSpinBox*>(m_controlItems.at(i).aawControl)) {
            if (m_controlItems.at(i).aatType==ZLADSPA::aacInteger) {
                w->setValue(m_controlItems.at(i).aasInt);
            } else if (m_controlItems.at(i).aatType==ZLADSPA::aacFreq) {
                w->setValue(m_controlItems.at(i).aasFreq);
            } else {
                w->setValue(m_controlItems.at(i).aasValue);
            }
        }
    }
}

CLADSPAPlugItem ZLADSPADialog::getPlugItem() const
{
    if (m_selectedPluginID == 0L) return CLADSPAPlugItem();

    QVector<ZLADSPAControlItem> controls;
    controls.reserve(m_controlItems.count());
    for (const auto &item : qAsConst(m_controlItems)) {
        controls.append(item);
        controls.last().disconnectFromControls();
    }

    ZLADSPA::Policy policy = ZLADSPA::Policy::plNone;
    bool usePolicy = false;
    if (checkPolicy->checkState() == Qt::Checked) {
        policy = ZLADSPA::Policy::plDuplicate;
        usePolicy = true;
    } else if (checkPolicy->checkState() == Qt::Unchecked) {
        usePolicy = true;
    }

    CLADSPAPlugItem item(m_pluginLabel.value(m_selectedPluginID),
                         m_selectedPluginID,
                         m_pluginName.value(m_selectedPluginID),
                         m_pluginFile.value(m_selectedPluginID),
                         controls,usePolicy,policy,m_inputsModel->getBindings(),
                         m_outputsModel->getBindings());

    return item;
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

void ZLADSPADialog::addInputBinding()
{
    m_inputsModel->appendRow();
}

void ZLADSPADialog::deleteInputBinding()
{
    QModelIndex idx = tableInputs->currentIndex();
    m_inputsModel->deleteRow(idx);
}

void ZLADSPADialog::addOutputBinding()
{
    m_outputsModel->appendRow();
}

void ZLADSPADialog::deleteOutputBinding()
{
    QModelIndex idx = tableOutputs->currentIndex();
    m_outputsModel->deleteRow(idx);
}

void ZLADSPADialog::policyChanged(bool status)
{
    Q_UNUSED(status)

    buttonAddInput->setEnabled(checkPolicy->checkState()!=Qt::Checked);
    buttonDeleteInput->setEnabled(checkPolicy->checkState()!=Qt::Checked);
    buttonAddOutput->setEnabled(checkPolicy->checkState()!=Qt::Checked);
    buttonDeleteOutput->setEnabled(checkPolicy->checkState()!=Qt::Checked);

    if (checkPolicy->checkState()==Qt::Checked) {
        // Only one binding allowed for duplicate policy
        int count = m_inputsModel->rowCount();
        if (count < 1) {
            m_inputsModel->appendRow();
        } else if (count > 1) {
            m_inputsModel->removeRows(1, count - 1);
        }

        count = m_outputsModel->rowCount();
        if (count < 1) {
            m_outputsModel->appendRow();
        } else if (count > 1) {
            m_outputsModel->removeRows(1, count - 1);
        }
    }
}

int ZLADSPADialog::getChannels() const
{
    return m_channels;
}

void ZLADSPADialog::readInfoFromControls()
{
    for (int i=0;i<m_controlItems.count();i++) {
        if (m_controlItems.at(i).aawControl==nullptr) continue;
        if (m_controlItems.at(i).aatType==ZLADSPA::aacToggle) {
            if (auto w=qobject_cast<QCheckBox*>(m_controlItems.at(i).aawControl))
                m_controlItems[i].aasToggle=(w->checkState()==Qt::Checked);
        } else if (auto w=qobject_cast<QDoubleSpinBox*>(m_controlItems.at(i).aawControl)) {
            if (m_controlItems.at(i).aatType==ZLADSPA::aacInteger) {
                m_controlItems[i].aasInt=ZGenericFuncs::truncDouble(w->value());
            } else if (m_controlItems.at(i).aatType==ZLADSPA::aacFreq) {
                m_controlItems[i].aasFreq=ZGenericFuncs::truncDouble(w->value());
            } else {
                m_controlItems[i].aasValue=w->value();
            }
        } else {
            m_controlItems[i].aasToggle=false;
            m_controlItems[i].aasValue=0.0;
            m_controlItems[i].aasFreq=0;
            m_controlItems[i].aasInt=0;
        }
    }
}

void ZLADSPADialog::scanPlugins()
{
    m_selectedPluginID = 0L;
    comboPlugin->clear();
    m_pluginFile.clear();
    m_pluginName.clear();
    m_pluginLabel.clear();
    editPluginInfo->clear();

    clearCItems();
    m_controls->resize(sizeHint());

    using CComboPair = QPair<QString,qint64>;
    QVector<CComboPair> comboItems;

    const QStringList ladspa_dirs=ZGenericFuncs::getLADSPAPath().split(':',QString::SkipEmptyParts);
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
                        QString pluginLabel = QString::fromUtf8(psDescriptor->Label);
                        auto plugID = static_cast<qint64>(psDescriptor->UniqueID);
                        m_pluginFile[plugID] = ladspa_dir.filePath(ladspa_dir[j]);
                        m_pluginName[plugID] = pluginName;
                        m_pluginLabel[plugID] = pluginLabel;
                        comboItems.append(qMakePair(QSL("%1 (%2/%3)")
                                                    .arg(pluginName)
                                                    .arg(plugID)
                                                    .arg(pluginLabel),plugID));
                    }
                }
                plugin.unload();
            }
        }
    }
    std::sort(comboItems.begin(),comboItems.end(),[](const CComboPair &a, const CComboPair &b){
        return (a.first < b.first);
    });
    comboPlugin->blockSignals(true);
    for (const auto &i : qAsConst(comboItems))
        comboPlugin->addItem(i.first,i.second);
    comboPlugin->blockSignals(false);
}

void ZLADSPADialog::analyzePlugin()
{
    const float nearZero = 0.00001F;
    editPluginInfo->clear();
    m_selectedPluginValidInputs.clear();
    m_selectedPluginValidOutputs.clear();
    QString pinfo = QSL("<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\"white-space: pre-wrap;\">");

    clearCItems();
    m_controls->resize(sizeHint());

    if (m_selectedPluginID == 0L) return;

    QLibrary plugin(m_pluginFile.value(m_selectedPluginID));
    if (plugin.load()) {
        auto pfDescriptorFunction = reinterpret_cast<LADSPA_Descriptor_Function>(plugin.resolve("ladspa_descriptor"));
        if (pfDescriptorFunction) {
            const LADSPA_Descriptor * psDescriptor;
            unsigned long lPluginIndex;
            unsigned long lPortIndex;
            LADSPA_PortRangeHintDescriptor iHintDescriptor;
            LADSPA_Data fBound;
            LADSPA_Data fDefault;
            QString sPluginLabel = m_pluginLabel.value(m_selectedPluginID);
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
                pinfo+=tr("Plugin library: <b>\"%1\"</b><br/><br/>").arg(m_pluginFile.value(m_selectedPluginID));

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

                pinfo+=tr("Ports:<br/>");

                for (lPortIndex = 0;lPortIndex < psDescriptor->PortCount;lPortIndex++) {

                    QString name = QString::fromUtf8(psDescriptor->PortNames[lPortIndex]);

                    if (LADSPA_IS_PORT_AUDIO(psDescriptor->PortDescriptors[lPortIndex]) &&
                            LADSPA_IS_PORT_INPUT(psDescriptor->PortDescriptors[lPortIndex])) {
                        m_selectedPluginValidInputs.append(name);
                        pinfo+=tr("    <b>\"%1\"</b> input, audio<br/>").arg(name);
                    }

                    if (LADSPA_IS_PORT_AUDIO(psDescriptor->PortDescriptors[lPortIndex]) &&
                            LADSPA_IS_PORT_OUTPUT(psDescriptor->PortDescriptors[lPortIndex])) {
                        m_selectedPluginValidOutputs.append(name);
                        pinfo+=tr("    <b>\"%1\"</b> output, audio<br/>").arg(name);
                    }

                    if (LADSPA_IS_PORT_CONTROL(psDescriptor->PortDescriptors[lPortIndex]) &&
                            LADSPA_IS_PORT_INPUT(psDescriptor->PortDescriptors[lPortIndex])) {

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
                                auto acheckBox = new QCheckBox(QSL("%1").arg(name),m_controls);
                                bool toggledState=false;
                                if (LADSPA_IS_HINT_DEFAULT_1(iHintDescriptor)) toggledState=true;
                                m_controlItems << ZLADSPAControlItem(
                                                      QSL("%1").arg(QString::fromUtf8(psDescriptor->PortNames[lPortIndex])),
                                                      ZLADSPA::aacToggle,toggledState,0.0,acheckBox,nullptr,nullptr);
                                acheckBox->setObjectName(tr("checkBox#%1").arg(m_controlItems.size()-1));
                                m_controls->layout()->addWidget(acheckBox);
                                connect(acheckBox,SIGNAL(stateChanged(int)),this,SLOT(stateChanged(int)));
                                pinfo+=tr("    <b>\"%1\"</b> input, control, toggle, default: %2<br/>").arg(name).arg(toggledState);
                            }
                        } else {
                            auto ahboxLayout = new QHBoxLayout();
                            ahboxLayout->setSpacing(6);
                            ahboxLayout->setMargin(0);
                            auto alabel=new QLabel(m_controls);
                            if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
                                alabel->setText(tr("%1 (in dB)").arg(QString::fromUtf8(psDescriptor->PortNames[lPortIndex])));
                            } else if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor)) {
                                alabel->setText(tr("%1 (in Hz)").arg(QString::fromUtf8(psDescriptor->PortNames[lPortIndex])));
                            } else {
                                alabel->setText(QSL("%1").arg(QString::fromUtf8(psDescriptor->PortNames[lPortIndex])));
                            }
                            auto aspinBox=new QDoubleSpinBox(m_controls);
                            QString pname=QSL("%1").arg(QString::fromUtf8(psDescriptor->PortNames[lPortIndex]));

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
                            m_controlItems << ZLADSPAControlItem(pname,ciType,false,0.0,aspinBox,ahboxLayout,alabel);
                            ahboxLayout->setObjectName(tr("hboxlayout#%1").arg(m_controlItems.size()-1));
                            alabel->setObjectName(tr("alabel#%1").arg(m_controlItems.size()-1));
                            aspinBox->setObjectName(tr("aspinBox#%1").arg(m_controlItems.size()-1));

                            alabel->setBuddy(aspinBox);
                            ahboxLayout->addWidget(alabel);
                            ahboxLayout->addWidget(aspinBox);
                            aspinBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed));
                            m_vboxLayout->addLayout(ahboxLayout);
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
                                        aspinBox->setMinimum(static_cast<double>(fBound*m_sampleRate));
                                    } else {
                                        aspinBox->setMinimum(static_cast<double>(fBound));
                                    }
                                }
                                if (LADSPA_IS_HINT_BOUNDED_ABOVE(iHintDescriptor)) {
                                    fBound = psDescriptor->PortRangeHints[lPortIndex].UpperBound;
                                    if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fBound)>nearZero) {
                                        aspinBox->setMaximum(static_cast<double>(fBound*m_sampleRate));
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
                                        aspinBox->setValue(static_cast<double>(fDefault*m_sampleRate));
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
                                        aspinBox->setValue(static_cast<double>(fDefault*m_sampleRate));
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
                                        aspinBox->setValue(static_cast<double>(fDefault*m_sampleRate));
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
                                        aspinBox->setValue(static_cast<double>(fDefault*m_sampleRate));
                                    } else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && abs(fDefault)>nearZero) {
                                        aspinBox->setValue(static_cast<double>(truncf(fDefault)));
                                    } else {
                                        aspinBox->setValue(static_cast<double>(fDefault));
                                    }
                                    break;
                                case LADSPA_HINT_DEFAULT_MAXIMUM:
                                    fDefault = psDescriptor->PortRangeHints[lPortIndex].UpperBound;
                                    if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fDefault)>nearZero) {
                                        aspinBox->setValue(static_cast<double>(fDefault*m_sampleRate));
                                    } else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && abs(fDefault)>nearZero) {
                                        aspinBox->setValue(static_cast<double>(truncf(fDefault)));
                                    } else {
                                        aspinBox->setValue(static_cast<double>(fDefault));
                                    }
                                    break;
                                case LADSPA_HINT_DEFAULT_0:
                                    fDefault=0.0F;
                                    if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fDefault)>nearZero) {
                                        aspinBox->setValue(static_cast<double>(fDefault*m_sampleRate));
                                    } else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && abs(fDefault)>nearZero) {
                                        aspinBox->setValue(static_cast<double>(truncf(fDefault)));
                                    } else {
                                        aspinBox->setValue(static_cast<double>(fDefault));
                                    }
                                    break;
                                case LADSPA_HINT_DEFAULT_1:
                                    fDefault=1.0F;
                                    if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fDefault)>nearZero) {
                                        aspinBox->setValue(static_cast<double>(fDefault*m_sampleRate));
                                    } else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && abs(fDefault)>nearZero) {
                                        aspinBox->setValue(static_cast<double>(truncf(fDefault)));
                                    } else {
                                        aspinBox->setValue(static_cast<double>(fDefault));
                                    }
                                    break;
                                case LADSPA_HINT_DEFAULT_100:
                                    fDefault=100.0F;
                                    if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fDefault)>nearZero) {
                                        aspinBox->setValue(static_cast<double>(fDefault*m_sampleRate));
                                    } else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && abs(fDefault)>nearZero) {
                                        aspinBox->setValue(static_cast<double>(truncf(fDefault)));
                                    } else {
                                        aspinBox->setValue(static_cast<double>(fDefault));
                                    }
                                    break;
                                case LADSPA_HINT_DEFAULT_440:
                                    fDefault=440.0F;
                                    if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && abs(fDefault)>nearZero) {
                                        aspinBox->setValue(static_cast<double>(fDefault*m_sampleRate));
                                    } else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && abs(fDefault)>nearZero) {
                                        aspinBox->setValue(static_cast<double>(truncf(fDefault)));
                                    } else {
                                        aspinBox->setValue(static_cast<double>(fDefault));
                                    }
                                    break;
                            } // switch (iHintDescriptor & LADSPA_HINT_DEFAULT_MASK)

                            if (LADSPA_IS_HINT_INTEGER(iHintDescriptor)) {
                                m_controlItems.last().aasInt=ZGenericFuncs::truncDouble(aspinBox->value());
                            } else if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor)) {
                                m_controlItems.last().aasFreq=ZGenericFuncs::truncDouble(aspinBox->value());
                            } else {
                                m_controlItems.last().aasValue=aspinBox->value();
                            }

                            pinfo+=tr("    <b>\"%1\"</b> input, control, %2 to %3, default: %4<br/>")
                                    .arg(name).arg(aspinBox->minimum()).arg(aspinBox->maximum()).arg(aspinBox->value());


                        } // else ... (LADSPA_IS_HINT_TOGGLED(iHintDescriptor))

                    } // if (LADSPA_IS_PORT_CONTROL...
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
    editPluginInfo->setHtml(pinfo);
    m_controls->resize(sizeHint());
    m_controls->update();

    m_inputsModel->setValidPorts(m_selectedPluginValidInputs);
    m_outputsModel->setValidPorts(m_selectedPluginValidOutputs);

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
    for (auto & item : m_controlItems)
        item.destroyControls();

    m_controlItems.clear();
}
