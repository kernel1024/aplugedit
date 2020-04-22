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

#ifndef GENERIC_H
#define GENERIC_H

#include <QObject>
#include <QString>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QJsonValue>
#include <QLabel>
#include <QCommandLineParser>

#define QSL QStringLiteral

class ZGenericFuncs : public QObject
{
    Q_OBJECT
public:
    enum CommandLineParseResult
    {
        CommandLineOk,
        CommandLineError,
        CommandLineVersionRequested,
        CommandLineHelpRequested
    };
    Q_ENUM(CommandLineParseResult)

    ZGenericFuncs(QObject *parent);
    virtual ~ZGenericFuncs();

    static int numDigits(int n);
    static int truncDouble(double num);
    static QString getLADSPAPath();
    static void showWarningsDialog(QWidget *parent, const QString& title,
                                       const QString& text, const QStringList& warnings);
    static Qt::CheckState readTristateFromJson(const QJsonValue& value);
    static QJsonValue writeTristateToJson(Qt::CheckState state);
    static bool runnedFromQtCreator();
    static ZGenericFuncs::CommandLineParseResult parseCommandLine(QCommandLineParser &parser, QString *fileName,
                                                                  QString *errorMessage,
                                                                  bool *startMinimized);
};

class ZDescListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
private:
    Q_DISABLE_COPY(ZDescListItemDelegate)
public:
    explicit ZDescListItemDelegate(QObject *parent = nullptr);
    ~ZDescListItemDelegate() override;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class ZValidatedListEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ZValidatedListEditDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
};

class ZValidatedSpinBoxEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ZValidatedSpinBoxEditDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
};

#endif // GENERIC_H
