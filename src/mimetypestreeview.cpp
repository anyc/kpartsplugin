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

#include <KLocale>
#include <KStandardDirs>
#include <KConfigGroup>

#include "mimetypestreeview.h"
#include "mimetypehelper.h"

const int PlainRole = Qt::UserRole + 7112;

MimeTypesItemModel::MimeTypesItemModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_userConfig = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", configFilename), KConfig::SimpleConfig);
}

void MimeTypesItemModel::load()
{
    QStringList mimetypeList, enabledMimeTypes;
    MimeTypeHelper::initAllMimeTypes(mimetypeList, enabledMimeTypes);

    m_mimetypeMap.clear();
    m_mimetypeKeys.clear();
    m_mimetypeBlacklisted.clear();
    m_mimetypeDescription.clear();
    for (QStringList::ConstIterator it = mimetypeList.constBegin(); it != mimetypeList.constEnd(); ++it) {
        const QStringList list = (*it).split(":");
        const QStringList mimeCat = list[0].split("/");
        if (mimeCat.length() != 2) continue;

        QStringList subTypes = m_mimetypeMap.value(mimeCat[0], QStringList());
        if (subTypes.isEmpty())
            m_mimetypeKeys.append(mimeCat[0]);
        subTypes.append(mimeCat[1]);
        subTypes.sort();
        m_mimetypeMap.insert(mimeCat[0], subTypes);

        if (!enabledMimeTypes.contains(*it))
            m_mimetypeBlacklisted.append(list[0]);
        m_mimetypeDescription.insert(list[0], list[2]);
    }
    m_mimetypeKeys.sort();

    reset();
}

void MimeTypesItemModel::save()
{
    internalResetToDefaults();

    KConfigGroup config(m_userConfig, configSectionBlacklisted);
    for (QStringList::ConstIterator it = m_mimetypeBlacklisted.constBegin(); it != m_mimetypeBlacklisted.constEnd(); ++it)
        config.writeEntry(*it, true);
    config.sync();
}

void MimeTypesItemModel::resetToDefaults()
{
    internalResetToDefaults();
    KConfigGroup config(m_userConfig, configSectionBlacklisted);
    config.sync();

    m_mimetypeBlacklisted.clear();

    reset();
}

void MimeTypesItemModel::internalResetToDefaults()
{
    KConfigGroup config(m_userConfig, configSectionBlacklisted);
    config.deleteGroup();
}


void MimeTypesItemModel::enableAllMimetypes()
{
    m_mimetypeBlacklisted.clear();
    reset();
    emit dataChanged(QModelIndex(), QModelIndex());
}

void MimeTypesItemModel::disableAllMimetypes()
{
    m_mimetypeBlacklisted.clear();
    const QStringList keys = m_mimetypeMap.keys();
    for (QStringList::ConstIterator kit = keys.constBegin(); kit != keys.constEnd(); ++kit) {
        const QStringList subTypes = m_mimetypeMap.value(*kit, QStringList());
        for (QStringList::ConstIterator sit = subTypes.constBegin(); sit != subTypes.constEnd(); ++sit) {
            const QString fullType = *kit + QLatin1Char('/') + *sit;
            m_mimetypeBlacklisted.append(fullType);
        }
    }

    reset();
    emit dataChanged(QModelIndex(), QModelIndex());
}

int MimeTypesItemModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant MimeTypesItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section != 0 || orientation != Qt::Horizontal)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return i18n("Mime Type");
    default:
        return QVariant();
    }
}

QVariant MimeTypesItemModel::data(const QModelIndex &index, int role) const
{
    if (index.parent() == QModelIndex()) {
        /// First-level node
        if (index.row() < 0 || index.row() >= m_mimetypeKeys.count())
            return QVariant();

        switch (role) {
        case PlainRole:
        case Qt::DisplayRole:
            return m_mimetypeKeys[index.row()];
        }
    } else if (index.parent().parent() == QModelIndex()) {
        /// Second-level node
        const QString cat = data(index.parent(), PlainRole).toString();
        const QStringList subTypes = m_mimetypeMap.value(cat, QStringList());
        if (index.row() < 0 || index.row() >= subTypes.count())
            return QVariant();
        const QString fullType = cat + QLatin1Char('/') + subTypes[index.row()];

        switch (role) {
        case PlainRole:
            return fullType;
        case Qt::DisplayRole:
            return subTypes[index.row()];
        case Qt::ToolTipRole:
            return m_mimetypeDescription[fullType];
        case Qt::CheckStateRole:
            return QVariant::fromValue<int>(m_mimetypeBlacklisted.contains(fullType) ? Qt::Unchecked : Qt::Checked);
        }
    }

    return QVariant();
}

bool MimeTypesItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::CheckStateRole || value.type() != QVariant::Int)
        return false;

    bool isInt = false;
    const int checkState = value.toInt(&isInt);
    if (!isInt) return false;

    const QString fullType = data(index, PlainRole).toString();
    if (checkState == Qt::Checked)
        m_mimetypeBlacklisted.removeOne(fullType);
    else if (!m_mimetypeBlacklisted.contains(fullType))
        m_mimetypeBlacklisted.append(fullType);

    emit dataChanged(index, index);

    return true;
}

bool MimeTypesItemModel::hasChildren(const QModelIndex &parent) const
{
    return parent.parent() == QModelIndex();
}

QModelIndex MimeTypesItemModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column, parent == QModelIndex() ? -1 : parent.row());
}

QModelIndex MimeTypesItemModel::parent(const QModelIndex &index) const
{
    if (index.internalId() < 0)
        return QModelIndex();
    else
        return createIndex(index.internalId(), 0, -1);
}

int MimeTypesItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent == QModelIndex()) {
        return m_mimetypeKeys.count();
    } else if (parent.parent() == QModelIndex()) {
        const QString cat = data(parent, PlainRole).toString();
        return m_mimetypeMap.value(cat, QStringList()).count();
    } else
        return 0;
}

Qt::ItemFlags MimeTypesItemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractItemModel::flags(index);
    if (index.internalId() >= 0)
        f |= Qt::ItemIsUserCheckable;
    return f;
}
