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
***************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H 1

#include <QtCore>
#include <QtGui>
#include "ui_mainwindow.h"
#include "renderarea.h"

class ZMainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
private:
    bool modified;
    ZRenderArea *renderArea;
    QLabel* statusLabel;
    QLabel* mouseLabel;
    QString workFile;
    QString programTitle;
    QTimer repaintTimer;

    void clearSchematic();

public:
    explicit ZMainWindow(QWidget *parent = nullptr);
    ~ZMainWindow() override;
    
    void generateConfigToFile(QTextStream & stream);

public Q_SLOTS:
    void fileNew();
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void fileGenerate();
    void fileGeneratePart();
    void editComponent();
    void toolAllocate();
    void helpAbout();
    void repaintWithConnections();
    void changingComponents(ZCPBase *base);
    void updateStatus();
    void loadFile(const QString& fname);
    bool saveFile(const QString& fname);

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
};
#endif
