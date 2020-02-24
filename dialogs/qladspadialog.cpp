/***************************************************************************
*   Copyright (C) 2006 by Kernel                                          *
*   kernelonline@bk.ru                                                    *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
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
*   Code in this module is partially from sources of LADSPA SDK tools     *
*   (http://www.ladspa.org/), written by Richard W.E. Furse.              *
*                                                                         *
***************************************************************************/

#include <QtWidgets>
#include <QtCore>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include "includes/ladspa.h"
#include "includes/qladspadialog.h"
#include "includes/cpbase.h"

static bool updresize=false;

QLADSPADialog::QLADSPADialog(QWidget *parent, int aSampleRate)
  : QDialog(parent)
{
  setupUi(this);
  
  QScrollArea* alScroller=new QScrollArea(alTabSettings);
  alScroller->setWidgetResizable(true);

  alControls = new QResizableFrame(alScroller);
  alControls->setObjectName(QString::fromUtf8("alControls"));
  alControls->setFrameShape(QFrame::StyledPanel);
  alControls->setFrameShadow(QFrame::Plain);
  if (QGridLayout* vl=qobject_cast<QGridLayout*>(alTabSettings->layout()))
    vl->addWidget(alScroller,0,0,1,1);
  alControls->setMaximumSize(QSize(5*alTabSettings->width()/6,3000));
  alScroller->setWidget(alControls);
  
  vboxCLayout = new QVBoxLayout(alControls);
  vboxCLayout->setSpacing(6);
  vboxCLayout->setMargin(9);
  vboxCLayout->setObjectName(QString::fromUtf8("vboxCLayout"));

  selectedPlugin=0;
  alSampleRate=aSampleRate;
  isShowed=false;
  psPlugLabel="";
  psPlugID="";
  psCItems=0;
  controlsRequested=false;
  alCItems.clear();
  
  connect(alPlugin,SIGNAL(currentIndexChanged(int)),this,SLOT(changeLADSPA(int)));
}

QLADSPADialog::~QLADSPADialog()
{
}

QResizableFrame::QResizableFrame(QWidget *parent)
 : QFrame(parent)
{
  alScroller=qobject_cast<QScrollArea*>(parent);
}

QSize QResizableFrame::minimumSizeHint() const
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
  else
    return size().expandedTo(QSize(r.width(),r.height()));
}

QSize QResizableFrame::sizeHint() const
{
  return minimumSizeHint();
}

void QLADSPADialog::changeLADSPA(int index)
{
  selectedPlugin=index;
  analysePlugin(index);
}

void QLADSPADialog::setParams(QString &plugLabel, QString &plugID, QalCItems* aCItems)
{
  isShowed=false;
  psPlugLabel=plugLabel;
  psPlugID=plugID;
  psCItems=aCItems;
}

void QLADSPADialog::showEvent(QShowEvent * )
{
  if (isShowed) return;
  isShowed=true;
  scanPlugins();
  controlsRequested=true;
  if (lsPluginLabel.count()==0)
  {
    changeLADSPA(-1);
    return;
  }
  int idx=lsPluginLabel.indexOf(QRegExp("^"+psPlugLabel));
  if ((idx==-1) || (lsPluginID.indexOf(QRegExp("^"+psPlugID))!=idx))
  {
    alPlugin->setCurrentIndex(0);
    changeLADSPA(0);
    return;
  }
  alPlugin->setCurrentIndex(idx);
  for (int i=0;i<psCItems->count();i++)
  {
    if (alCItems[i]->portName!=(*psCItems)[i]->portName) continue;
    alCItems[i]->aatType=(*psCItems)[i]->aatType;
    alCItems[i]->aasToggle=(*psCItems)[i]->aasToggle;
    alCItems[i]->aasValue=(*psCItems)[i]->aasValue;
    alCItems[i]->aasFreq=(*psCItems)[i]->aasFreq;
    alCItems[i]->aasInt=(*psCItems)[i]->aasInt;
    if (alCItems[i]->aatType==aacToggle)
    {
      if (QCheckBox* w=qobject_cast<QCheckBox*>(alCItems[i]->aawControl))
        if (alCItems[i]->aasToggle)
          w->setCheckState(Qt::Checked);
        else
          w->setCheckState(Qt::Unchecked);
    } else if (QDoubleSpinBox* w=qobject_cast<QDoubleSpinBox*>(alCItems[i]->aawControl)) 
    {
      if (alCItems[i]->aatType==aacInteger)
        w->setValue(alCItems[i]->aasInt);
      else if (alCItems[i]->aatType==aacFreq)
        w->setValue(alCItems[i]->aasFreq);
      else
        w->setValue(alCItems[i]->aasValue);
    }
  }
}

void QLADSPADialog::getParams(QString &plugLabel, QString &plugID, QString &plugName, QString &plugFile, QalCItems* aCItems)
{
  plugLabel="";
  plugID="";
  plugName="";
  plugFile="";
  aCItems->clear();
  if (selectedPlugin==-1) return;
  plugLabel=lsPluginLabel[selectedPlugin];
  plugID=lsPluginID[selectedPlugin];
  plugName=lsPluginName[selectedPlugin];
  plugFile=lsPluginFile[selectedPlugin];
  for (int i=0;i<alCItems.count();i++)
  {
    QControlItem* ci=new QControlItem(alCItems[i]);
    ci->setParent(0);
    aCItems->append(ci);
  }
}

void QLADSPADialog::valueChanged(double )
{
  readInfoFromControls();
}

void QLADSPADialog::stateChanged(int )
{
  readInfoFromControls();
}

void QLADSPADialog::readInfoFromControls()
{
  for (int i=0;i<alCItems.count();i++)
  {
    if (alCItems[i]->aawControl==0) continue;
    if (alCItems[i]->aatType==aacToggle)
    {
      if (QCheckBox* w=qobject_cast<QCheckBox*>(alCItems[i]->aawControl))
        alCItems[i]->aasToggle=(w->checkState()==Qt::Checked);
    } else if (QDoubleSpinBox* w=qobject_cast<QDoubleSpinBox*>(alCItems[i]->aawControl))
    {
      if (alCItems[i]->aatType==aacInteger)
        alCItems[i]->aasInt=(int)truncf(w->value());
      else if (alCItems[i]->aatType==aacFreq)
        alCItems[i]->aasFreq=(int)truncf(w->value());
      else
        alCItems[i]->aasValue=w->value();
    } else {
      alCItems[i]->aasToggle=false;
      alCItems[i]->aasValue=0.0;
      alCItems[i]->aasFreq=0;
      alCItems[i]->aasInt=0;
    }
  }
}

void QLADSPADialog::scanPlugins()
{
  alPlugin->clear();
  lsPluginFile.clear();
  lsPluginName.clear();
  lsPluginID.clear();
  lsPluginLabel.clear();
  alPluginInfo->clear();
  
  clearCItems();
  alControls->resize(sizeHint());
  
  QStringList env = QProcess::systemEnvironment();
  int env_idx = env.indexOf(QRegExp("^LADSPA_PATH=.*"));
  if (env_idx==-1)
  {
    selectedPlugin=-1;
    QMessageBox::warning(0,tr("LADSPA error"),tr("Warning: You do not have a LADSPA_PATH environment variable set."));
    return;
  }
  QString ladspa_path=env[env_idx];
  ladspa_path.remove("LADSPA_PATH=");
  QStringList ladspa_dirs=ladspa_path.split(':',QString::SkipEmptyParts);
  for (int i=0;i<ladspa_dirs.count();i++)
  {
    QDir ladspa_dir(ladspa_dirs[i]);
    ladspa_dir.setFilter(QDir::Files);
    for (uint j=0;j<ladspa_dir.count();j++)
    {
      QLibrary plugin(ladspa_dir.filePath(ladspa_dir[j]));
      if (plugin.load())
      {
        LADSPA_Descriptor_Function fDescriptorFunction
	= (LADSPA_Descriptor_Function)plugin.resolve("ladspa_descriptor");
	if (fDescriptorFunction)
	{
          const LADSPA_Descriptor * psDescriptor;
          for (long lIndex=0;(psDescriptor=fDescriptorFunction(lIndex))!=NULL;lIndex++)
          {
            lsPluginFile << ladspa_dir.filePath(ladspa_dir[j]);
            lsPluginName << psDescriptor->Name;
            lsPluginID << tr("%1").arg(psDescriptor->UniqueID);
            lsPluginLabel << psDescriptor->Label;
            alPlugin->addItem(tr("%1 (%2/%3)").arg( psDescriptor->Name ).arg( psDescriptor->UniqueID ).arg(psDescriptor->Label));
          }
	}
        plugin.unload();
      }
    }
  }
  selectedPlugin=-1;
}

void QLADSPADialog::analysePlugin(int index)
{
  alPluginInfo->clear();
  if (!controlsRequested) return;
  QString pinfo=
"<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\"white-space: pre-wrap; font-family:Courier New, Helvetica; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none; margin-top:0.25em; margin-bottom:0.25em;\">";

  clearCItems();
  alControls->resize(sizeHint());
  
  if (index==-1) return;
  QLibrary plugin(lsPluginFile[index]);
  if (plugin.load())
  {
    LADSPA_Descriptor_Function pfDescriptorFunction
    = (LADSPA_Descriptor_Function)plugin.resolve("ladspa_descriptor");
    if (pfDescriptorFunction) {
      const LADSPA_Descriptor * psDescriptor;
      unsigned long lPluginIndex;
      unsigned long lPortIndex;
      LADSPA_PortRangeHintDescriptor iHintDescriptor;
      LADSPA_Data fBound;
      LADSPA_Data fDefault;
      const char * pcPluginLabel=lsPluginLabel[index].toUtf8();
      bool foundPlugin=false;

      for (lPluginIndex = 0;; lPluginIndex++) {
        psDescriptor = pfDescriptorFunction(lPluginIndex);
        if (!psDescriptor)
          break;
        if (pcPluginLabel != NULL)
          if (strcmp(pcPluginLabel, psDescriptor->Label) != 0)
            continue;
        foundPlugin=true;
        
        pinfo+=tr("Plugin Name: <b>\"%1\"</b><br/>").arg(psDescriptor->Name);
        pinfo+=tr("Plugin Label: <b>\"%1\"</b><br/>").arg(psDescriptor->Label);
        pinfo+=tr("Plugin Unique ID: <b>%1</b><br/>").arg(psDescriptor->UniqueID);
        pinfo+=tr("Maker: <b>\"%1\"</b><br/>").arg(psDescriptor->Maker);
        pinfo+=tr("Copyright: <b>\"%1\"</b><br/>").arg(psDescriptor->Copyright);
        pinfo+=tr("Plugin library: <b>\"%1\"</b><br/><br/>").arg(lsPluginFile[index]);
        
        if (LADSPA_IS_REALTIME(psDescriptor->Properties))
          pinfo+=tr("Must Run Real-Time: <b>Yes</b><br/>");
        else
          pinfo+=tr("Must Run Real-Time: <b>No</b><br/>");
        if (psDescriptor->activate != NULL)
          pinfo+=tr("Has activate() Function: <b>Yes</b><br/>");
        else
          pinfo+=tr("Has activate() Function: <b>No</b><br/>");    
        if (psDescriptor->deactivate != NULL)
          pinfo+=tr("Has deactivate() Function: <b>Yes</b><br/>");
        else
          pinfo+=tr("Has deactivate() Function: <b>No</b><br/>");
        if (psDescriptor->run_adding != NULL)
          pinfo+=tr("Has run_adding() Function: <b>Yes</b><br/>");
        else
          pinfo+=tr("Has run_adding() Function: <b>No</b><br/>");
        
        if (psDescriptor->instantiate == NULL)
          pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN HAS NO INSTANTIATE FUNCTION.</font></b><br/>");
        if (psDescriptor->connect_port == NULL)
          pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN HAS NO CONNECT_PORT FUNCTION.</font></b><br/>");
        if (psDescriptor->run == NULL)
          pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN HAS NO RUN FUNCTION.</font></b><br/>");
        if (psDescriptor->run_adding != NULL
            && psDescriptor->set_run_adding_gain == NULL)
          pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN HAS RUN_ADDING FUNCTION BUT NOT SET_RUN_ADDING_GAIN.</font></b><br/>");
        if (psDescriptor->run_adding == NULL
            && psDescriptor->set_run_adding_gain != NULL)
          pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN HAS SET_RUN_ADDING_GAIN FUNCTION BUT NOT RUN_ADDING.</font></b><br/>");
        if (psDescriptor->cleanup == NULL)
          pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN HAS NO CLEANUP FUNCTION.</font></b><br/>");
        
        if (LADSPA_IS_HARD_RT_CAPABLE(psDescriptor->Properties))
          pinfo+=tr("Environment: <b>Normal or Hard Real-Time</b><br/>");
        else
          pinfo+=tr("Environment: <b>Normal</b><br/>");
        
        if (LADSPA_IS_INPLACE_BROKEN(psDescriptor->Properties))
          pinfo+=tr("<i>This plugin cannot use in-place processing. It will not work with all hosts.</i><br/>");
        pinfo+="<br/>";
        
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
                    | LADSPA_HINT_DEFAULT_1))
              pinfo+=tr("<b><font color=\"#8B0000\">ERROR: TOGGLED INCOMPATIBLE WITH OTHER HINT</font></b><br/>");
            else {
              QCheckBox* acheckBox = new QCheckBox(tr("%1").arg(psDescriptor->PortNames[lPortIndex]),alControls);
              bool toggledState=false;
              if (LADSPA_IS_HINT_DEFAULT_1(iHintDescriptor)) toggledState=true;
              QControlItem* ci=new QControlItem(0, tr("%1").arg( psDescriptor->PortNames[lPortIndex]),aacToggle,toggledState,0,acheckBox,0,0);
              alCItems.append(ci);
              acheckBox->setObjectName(tr("checkBox#%1").arg(alCItems.size()-1));
              alControls->layout()->addWidget(acheckBox);
              connect(acheckBox,SIGNAL(stateChanged(int)),this,SLOT(stateChanged(int)));
            }
          } else {
            QHBoxLayout* ahboxLayout = new QHBoxLayout();
            ahboxLayout->setSpacing(6);
            ahboxLayout->setMargin(0);
            QLabel* alabel=new QLabel(alControls);
            if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor))
            {
              if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor))
                alabel->setText(tr("%1 (in Hz)").arg(psDescriptor->PortNames[lPortIndex]));
              else
                alabel->setText(tr("%1 (in dB)").arg(psDescriptor->PortNames[lPortIndex]));
            } else
                alabel->setText(tr("%1").arg(psDescriptor->PortNames[lPortIndex]));
            QDoubleSpinBox* aspinBox=new QDoubleSpinBox(alControls);
            QString pname=tr("%1").arg(psDescriptor->PortNames[lPortIndex]);
            
            enum TalControl ciType;
            if (LADSPA_IS_HINT_INTEGER(iHintDescriptor))
              ciType=aacInteger;
            else if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor))
            {
              if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor))
                ciType=aacFreq;
              else
                ciType=aacLogarithmic;
            } else
              ciType=aacLinear;
            QControlItem* ci=new QControlItem(0,pname,ciType,false,0,aspinBox,ahboxLayout,alabel);
            
            alCItems.append(ci);
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
            if (LADSPA_IS_HINT_INTEGER(iHintDescriptor))
            {
              aspinBox->setDecimals(0);
              aspinBox->setMinimum(-1000);
              aspinBox->setMaximum(1000);
              aspinBox->setSingleStep(1);
              if (LADSPA_IS_HINT_BOUNDED_BELOW(iHintDescriptor))
                aspinBox->setMinimum(truncf(psDescriptor->PortRangeHints[lPortIndex].LowerBound));
              if (LADSPA_IS_HINT_BOUNDED_ABOVE(iHintDescriptor))
                aspinBox->setMaximum(truncf(psDescriptor->PortRangeHints[lPortIndex].UpperBound));
            } else {
              aspinBox->setMaximum(3.3e+37);
              if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor))
              {
                aspinBox->setDecimals(0);
                aspinBox->setSingleStep(1);
                aspinBox->setMinimum(0);
              } else {
                aspinBox->setDecimals(2);
                aspinBox->setSingleStep(0.01);
                aspinBox->setMinimum(-3.3e+37);
              }
              if (LADSPA_IS_HINT_BOUNDED_BELOW(iHintDescriptor) || LADSPA_IS_HINT_BOUNDED_ABOVE(iHintDescriptor)) {
                if (LADSPA_IS_HINT_BOUNDED_BELOW(iHintDescriptor)) {
                  fBound = psDescriptor->PortRangeHints[lPortIndex].LowerBound;
                  if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && fBound != 0) 
                    aspinBox->setMinimum(fBound*alSampleRate);
                  else
                    aspinBox->setMinimum(fBound);
                }
                if (LADSPA_IS_HINT_BOUNDED_ABOVE(iHintDescriptor)) {
                  fBound = psDescriptor->PortRangeHints[lPortIndex].UpperBound;
                  if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && fBound != 0)
                    aspinBox->setMaximum(fBound*alSampleRate);
                  else
                    aspinBox->setMaximum(fBound);
                }
              }
            }
            switch (iHintDescriptor & LADSPA_HINT_DEFAULT_MASK) {
              case LADSPA_HINT_DEFAULT_NONE:
                break;
              case LADSPA_HINT_DEFAULT_MINIMUM:
                fDefault = psDescriptor->PortRangeHints[lPortIndex].LowerBound;
                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(fDefault*alSampleRate);
                else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(truncf(fDefault));
                else 
                  aspinBox->setValue(fDefault);
                break;
              case LADSPA_HINT_DEFAULT_LOW:
                if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
                  fDefault 
                    = exp(log(psDescriptor->PortRangeHints[lPortIndex].LowerBound) 
                          * 0.75
                          + log(psDescriptor->PortRangeHints[lPortIndex].UpperBound) 
                          * 0.25);
                }
                else {
                  fDefault 
                    = (psDescriptor->PortRangeHints[lPortIndex].LowerBound
                      * 0.75
                      + psDescriptor->PortRangeHints[lPortIndex].UpperBound
                      * 0.25);
                }
                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(fDefault*alSampleRate);
                else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(truncf(fDefault));
                else 
                  aspinBox->setValue(fDefault);
                break;
              case LADSPA_HINT_DEFAULT_MIDDLE:
                if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
                  fDefault 
                    = sqrt(psDescriptor->PortRangeHints[lPortIndex].LowerBound
                          * psDescriptor->PortRangeHints[lPortIndex].UpperBound);
                }
                else {
                  fDefault 
                    = 0.5 * (psDescriptor->PortRangeHints[lPortIndex].LowerBound
                            + psDescriptor->PortRangeHints[lPortIndex].UpperBound);
                }
                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(fDefault*alSampleRate);
                else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(truncf(fDefault));
                else 
                  aspinBox->setValue(fDefault);
                break;
              case LADSPA_HINT_DEFAULT_HIGH:
                if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
                  fDefault 
                    = exp(log(psDescriptor->PortRangeHints[lPortIndex].LowerBound) 
                          * 0.25
                          + log(psDescriptor->PortRangeHints[lPortIndex].UpperBound) 
                          * 0.75);
                }
                else {
                  fDefault 
                    = (psDescriptor->PortRangeHints[lPortIndex].LowerBound
                      * 0.25
                      + psDescriptor->PortRangeHints[lPortIndex].UpperBound
                      * 0.75);
                }
                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(fDefault*alSampleRate);
                else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(truncf(fDefault));
                else 
                  aspinBox->setValue(fDefault);
                break;
              case LADSPA_HINT_DEFAULT_MAXIMUM:
                fDefault = psDescriptor->PortRangeHints[lPortIndex].UpperBound;
                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(fDefault*alSampleRate);
                else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(truncf(fDefault));
                else 
                  aspinBox->setValue(fDefault);
                break;
              case LADSPA_HINT_DEFAULT_0:
                fDefault=0.0;
                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(fDefault*alSampleRate);
                else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(truncf(fDefault));
                else 
                  aspinBox->setValue(fDefault);
                break;
              case LADSPA_HINT_DEFAULT_1:
                fDefault=1.0;
                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(fDefault*alSampleRate);
                else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(truncf(fDefault));
                else 
                  aspinBox->setValue(fDefault);
                break;
              case LADSPA_HINT_DEFAULT_100:
                fDefault=100.0;
                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(fDefault*alSampleRate);
                else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(truncf(fDefault));
                else 
                  aspinBox->setValue(fDefault);
                break;
              case LADSPA_HINT_DEFAULT_440:
                fDefault=440.0;
                if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(fDefault*alSampleRate);
                else if (LADSPA_IS_HINT_INTEGER(iHintDescriptor) && fDefault != 0)
                  aspinBox->setValue(truncf(fDefault));
                else 
                  aspinBox->setValue(fDefault);
                break;
            } // switch (iHintDescriptor & LADSPA_HINT_DEFAULT_MASK)

            if (LADSPA_IS_HINT_INTEGER(iHintDescriptor))
              ci->aasInt=(int)truncf(aspinBox->value());
            else if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor))
              ci->aasFreq=(int)truncf(aspinBox->value());
            else
              ci->aasValue=aspinBox->value();
          } // else ... (LADSPA_IS_HINT_TOGGLED(iHintDescriptor))

        } // for (port enumeration)
        break;
      } // for (plugin search)
      if (!foundPlugin)
        pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN NOT FOUND IN THIS LIBRARY.</font></b><br/>");
    } else // if (fDescriptorFunction correct)
      pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN INCORRECT (LADSPA_DESCRIPTOR SYMBOL NOT FOUND).</font></b><br/>");
    plugin.unload();
  } else// if (plugin.load())
    pinfo+=tr("<b><font color=\"#8B0000\">ERROR: PLUGIN LIBRARY NOT FOUND.</font></b><br/>");
  pinfo+=tr("</body></html>");
  alPluginInfo->setHtml(pinfo);
  alControls->resize(sizeHint());
  alControls->update();
  
  // this is dirty hack :) because Qt doesn't update element placement in vboxCLayout
  if (updresize)
    resize(width()+2,height());
  else
    resize(width()-2,height());
  updresize=!updresize;
}

void QLADSPADialog::closeEvent(QCloseEvent *)
{
  clearCItems();
}

void QLADSPADialog::clearCItems()
{
  while (!alCItems.isEmpty())
  {
    QControlItem* w=alCItems.takeFirst();
    w->destroyControls();
    delete w;
  }
}

/* --------- QControlItem ------------*/

QControlItem::QControlItem(QWidget *parent, const QString &AportName, TalControl AaatType, bool AaasToggle, float AaasValue, QWidget* AaawControl, QLayout* AaawLayout, QLabel* AaawLabel)
 : QObject(parent)
{
  aatType=AaatType;
  aasToggle=AaasToggle;
  aasValue=AaasValue;
  aasFreq=(int)truncf(aasValue);
  aasInt=(int)truncf(aasInt);
  portName=AportName;
  aawControl=AaawControl;
  aawLayout=AaawLayout;
  aawLabel=AaawLabel;
}

QControlItem::QControlItem(QControlItem* &c)
  : QObject(c->parent())
{
  aatType=c->aatType;
  aasToggle=c->aasToggle;
  aasValue=c->aasValue;
  aasFreq=c->aasFreq;
  aasInt=c->aasInt;
  portName=c->portName;
  aawControl=c->aawControl;
  aawLayout=c->aawLayout;
  aawLabel=c->aawLabel;
}

QControlItem::QControlItem()
  : QObject(0)
{
  aatType=aacToggle;
  aasToggle=false;
  aasValue=0.0;
  aasFreq=0;
  aasInt=0;
  portName="";
  aawControl=0;
  aawLayout=0;
  aawLabel=0;
}

void QControlItem::destroyControls()
{
  if (aawControl)
  {
    aawControl->disconnect(this);
    aawControl->deleteLater();
  }
  aawControl=0;
  if (aawLayout)
    aawLayout->deleteLater();
  aawLayout=0;
  if (aawLabel)
    aawLabel->deleteLater();
  aawLabel=0;
}

void QControlItem::storeToStream(QDataStream &s)
{
  s.writeRawData((char*)&(aatType),sizeof(aatType));
  s << aasToggle << aasValue << aasFreq << aasInt << portName;
}

QControlItem::QControlItem(QDataStream &s)
{
  s.readRawData((char*)&(aatType),sizeof(aatType));
  s >> aasToggle >> aasValue >> aasFreq >> aasInt >> portName;
  aawControl=0;
  aawLayout=0;
  aawLabel=0;
}
