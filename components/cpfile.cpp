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
#include "includes/cpfile.h"

QCPFile::QCPFile(QWidget *parent, QRenderArea *aOwner)
  : QCPBase(parent,aOwner)
{
  fInp=new QCPInput(this,this);
  fInp->pinName="in";
  fInputs.append(fInp);
  fileName="unnamed";
}

QCPFile::~QCPFile()
{
  delete fInp;
}

QSize QCPFile::minimumSizeHint() const
{
  return QSize(180,50);
}

QSize QCPFile::sizeHint() const
{
  return minimumSizeHint();
}

void QCPFile::realignPins(QPainter &)
{
  fInp->relCoord=QPoint(QCP_PINSIZE/2,height()/2);
}

void QCPFile::doInfoGenerate(QTextStream & stream)
{
  stream << "pcm." << objectName() << " {" << endl;
  stream << "  type file" << endl;
  stream << "  slave.pcm null" << endl;
  stream << "  file \"" << fileName << "\"" << endl;
  stream << "  format \"raw\"" << endl;
  stream << "}" << endl;
  stream << endl;
}

void QCPFile::paintEvent ( QPaintEvent * )
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
  p.drawText(rect(),Qt::AlignCenter,"File Writer");
  
  n.setBold(false);
  n.setPointSize(n.pointSize()-3);
  p.setPen(QPen(Qt::gray));
  p.setFont(n);
  QFileInfo fi(fileName);
  p.drawText(QRect(0,height()/3,width(),height()),Qt::AlignCenter,fi.fileName());
  
  p.setFont(of);
  p.setBrush(ob);
  p.setPen(op);
}

void QCPFile::readFromStream( QDataStream & stream )
{
  QCPBase::readFromStream(stream);
  stream >> fileName;
}

void QCPFile::storeToStream( QDataStream & stream )
{
  QCPBase::storeToStream(stream);
  stream << fileName;
}

void QCPFile::showSettingsDlg()
{
  QFileDialog d(0,tr("Choose a filename to save stream under"),"~",
                  "RAW file (*.raw)");
  d.setDefaultSuffix("raw");
  d.setAcceptMode(QFileDialog::AcceptSave);
  d.setConfirmOverwrite(true);
  d.setDirectory(QDir::currentPath());
  if (!d.exec()) return;
  if (d.selectedFiles().count()==0) return;
  fileName=d.selectedFiles()[0];
  update();
  emit componentChanged(this);
}

