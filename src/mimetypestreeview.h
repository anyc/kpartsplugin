/***************************************************************************
 *   Copyright (C) 2010-2012 by Thomas Fischer                             *
 *   fischer@unix-ag.uni-kl.de                                             *
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

#ifndef MIMETYPESTREEVIEW_H
#define MIMETYPESTREEVIEW_H

#include <QAbstractItemModel>
#include <QMap>
#include <QStringList>

#include <KSharedConfig>

class MimeTypesItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    MimeTypesItemModel(QObject *parent = NULL);
    void load();
    void save();
    void resetToDefaults();

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

public slots:
    void enableAllMimetypes();
    void disableAllMimetypes();

private:
    QMap<QString, QStringList> m_mimetypeMap;
    QStringList m_mimetypeKeys;
    QStringList m_mimetypeBlacklisted;
    QMap<QString, QString> m_mimetypeDescription;

    KSharedPtr<KSharedConfig> m_userConfig;

    void internalResetToDefaults();
};

#endif // MIMETYPESTREEVIEW_H

