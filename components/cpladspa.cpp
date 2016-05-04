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
***************************************************************************/

#include "includes/cpbase.h"
#include "includes/cpladspa.h"
#include "includes/cphw.h"
#include "includes/cprate.h"

QCPLADSPA::QCPLADSPA(QWidget *parent, QRenderArea *aOwner)
  : QCPBase(parent,aOwner)
{
  fInp=new QCPInput(this,this);
  fInp->pinName="in";
  fInputs.append(fInp);
  fOut=new QCPOutput(this,this);
  fOut->pinName="out";
  fOutputs.append(fOut);
  
  alPlugName="<please select filter!!!>";
  alPlugID="";
  alPlugLabel="";
  alPlugFile="";
  alCItems.clear();
}

QCPLADSPA::~QCPLADSPA()
{
  delete fInp;
  delete fOut;
}

QSize QCPLADSPA::minimumSizeHint() const
{
  return QSize(200,50);
}

QSize QCPLADSPA::sizeHint() const
{
  return minimumSizeHint();
}

void QCPLADSPA::realignPins(QPainter &)
{
  fInp->relCoord=QPoint(QCP_PINSIZE/2,height()/2);
  fOut->relCoord=QPoint(width()-QCP_PINSIZE/2,height()/2);
}

void QCPLADSPA::doInfoGenerate(QTextStream & stream)
{
  stream << "pcm." << objectName() << " {" << endl;
  stream << "  type ladspa" << endl;
  if (fOut->toFilter!=0)
  {
    stream << "  slave {" << endl;
    stream << "    pcm \"" << fOut->toFilter->objectName() << "\"" << endl;
    stream << "  }" << endl;
    if (alPlugLabel!="")
    {
      QFileInfo fi(alPlugFile);
      stream << "  path \"" << fi.path() << "\"" << endl;
      stream << "  plugins [" << endl;
      stream << "    {" << endl;
      stream << "      label " << alPlugLabel << endl;
      stream << "      input {" << endl;
      stream << "        controls [ ";
      for (int i=0;i<alCItems.count();i++)
      {
        switch (alCItems[i]->aatType)
        {
          case aacToggle:
            if (alCItems[i]->aasToggle)
              stream << "1";
            else
              stream << "0";
            break;
          case aacFreq: stream << alCItems[i]->aasFreq; break;
          case aacInteger: stream << alCItems[i]->aasInt; break;
          default: stream << alCItems[i]->aasValue;
        }
        stream << " ";
      }
      stream << "]" << endl << "      }" << endl << "    }" << endl << "  ]" << endl;
    }
  }
  stream << "}" << endl;
  stream << endl;
  if (fOut->toFilter!=0)
    fOut->toFilter->doGenerate(stream);
}

void QCPLADSPA::paintEvent ( QPaintEvent * )
{
  QPainter p(this);
  QPen pn=QPen(Qt::black);
  QPen op=p.pen();
  QBrush ob=p.brush();
  QFont of=p.font();
  pn.setWidth(2);
  p.setPen(pn);
  p.setBrush(QBrush(Qt::white,Qt::SolidPattern));
  
  p.drawRect(rect());
  
  redrawPins(p);
  
  QFont n=of;
  n.setBold(true);
  n.setPointSize(n.pointSize()+1);
  p.setFont(n);
  p.drawText(rect(),Qt::AlignCenter,"LADSPA");
  
  n.setBold(false);
  n.setPointSize(n.pointSize()-3);
  p.setPen(QPen(Qt::gray));
  p.setFont(n);
    
  p.drawText(QRect(0,height()/3,width(),height()),Qt::AlignCenter,alPlugName);
  
  p.setFont(of);
  p.setBrush(ob);
  p.setPen(op);
}

void QCPLADSPA::readFromStream( QDataStream & stream )
{
  QCPBase::readFromStream(stream);
  stream >> alPlugName >> alPlugID >> alPlugLabel >> alPlugFile;
  int cn;
  stream >> cn;
  for (int i=0;i<cn;i++)
    alCItems.append(new QControlItem(stream));
}

void QCPLADSPA::storeToStream( QDataStream & stream )
{
  QCPBase::storeToStream(stream);
  stream << alPlugName << alPlugID << alPlugLabel << alPlugFile;
  stream << (int)alCItems.count();
  for (int i=0;i<alCItems.count();i++)
    alCItems[i]->storeToStream(stream);
}

static bool sampleRateWarn=false;

int QCPLADSPA::searchSampleRate()
{
  QCPBase* w=fOut->toFilter;
  int sampleRate=-1;
  while (w!=0)
  {
    QString s=w->metaObject()->className();
    if (QCPHW* hw=qobject_cast<QCPHW*>(w))
    {
      sampleRate=hw->alRate;
      break;
    }
    if (QCPRate* hw=qobject_cast<QCPRate*>(w))
    {
      sampleRate=hw->alRate;
      break;
    }
    if (fOutputs.count()==0) break;
    w=w->fOutputs[0]->toFilter;
  }
  if (sampleRate==-1)
  {
    if (!sampleRateWarn)
      QMessageBox::warning(0,"LADSPA settings","Samplerate not defined!\n\n"
        "Please connect this LADSPA filter to SRC or HW component\n"
        "and specify Rate parameter in theirs settings.\n"
        "Samplerate adjusted to default 44100 Hz.\n"
        "This setting is used by LADSPA plugins in frequency calculations.");
    sampleRateWarn=true;
    sampleRate=44100;
  }
  return sampleRate;
}

void QCPLADSPA::showSettingsDlg()
{
  QLADSPADialog* d=new QLADSPADialog(0,searchSampleRate());

  d->setParams(alPlugLabel,alPlugID,&alCItems);

  if (d->exec()==QDialog::Rejected)
  {
    delete d;
    return;
  }
  emit componentChanged(this);

  d->getParams(alPlugLabel,alPlugID,alPlugName,alPlugFile,&alCItems);
  delete d;
  update();
}

