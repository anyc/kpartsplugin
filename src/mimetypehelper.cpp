/***************************************************************************
 *   Copyright (C) 2012 by Thomas Fischer                                  *
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

#include <KMimeType>
#include <KDebug>
#include <KConfigGroup>
#include <KMimeTypeTrader>
#include <KStandardDirs>

#include "mimetypehelper.h"

const QString configFilename(QLatin1String("kpartsplugin-mimetypes.rc"));
const QString configSectionBlacklisted(QLatin1String("Blacklisted"));
const QString configSectionPreferredService(QLatin1String("PreferredService"));

/// built-in list of mime types that should never be loaded with this plugin
/// comparison is done with "startsWith", so "inode/" covers e.g. "inode/directory"
const QStringList MimeTypeHelper::builtinBlacklisted = QStringList() << QLatin1String("all/") << QLatin1String("x-") << QLatin1String("inode/") << QLatin1String("application/x-shockwave") << QLatin1String("application/futuresplash") << QLatin1String("application/force-download") << QLatin1String("application/x-force-download") << QLatin1String("application/googletalk") << QLatin1String("interface/") << QLatin1String("message/") << QLatin1String("multipart/") << QLatin1String("application/x-java") << QLatin1String("application/x-php") << QLatin1String("application/x-xpinstall") << QLatin1String("application/java-archive") << QLatin1String("video/x-javafx") << QLatin1String("application/atom+xml") << QLatin1String("application/ecmascript");


void MimeTypeHelper::initAllMimeTypes(QStringList &allMimeTypes, QStringList &enabledMimeTypes)
{
    if (!allMimeTypes.isEmpty()) return;

    /// load configuration to check which mime types are black-listed by user
    KSharedConfigPtr userConfig = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", configFilename), KConfig::SimpleConfig);
    KConfigGroup config(userConfig, configSectionBlacklisted);

    /// fetch complete list of mime types from KDE subsystem and sort them
    KMimeType::List mimetypes = KMimeType::allMimeTypes();
    qSort(mimetypes.begin(), mimetypes.end(), mimeTypeLessThan);

    /// go through each mime type
    foreach(const KSharedPtr<KMimeType> &mimetypePtr, mimetypes) {
        const QString mimetype = mimetypePtr->name();

        /// ignore special mime types
        bool isBuiltinBlacklisted = false;
        for (QStringList::ConstIterator bbit = builtinBlacklisted.constBegin(); !isBuiltinBlacklisted && bbit != builtinBlacklisted.constEnd(); ++bbit)
            isBuiltinBlacklisted |= mimetype.startsWith(*bbit);
        if (isBuiltinBlacklisted) {
            kDebug() << "Skipping blacklisted (built-in) mime type " << mimetype;
            continue;
        }

        /// fetch additional info (extension and description)
        const QString extension = mimetypePtr->mainExtension().mid(1);
        const QString description = mimetypePtr->comment();

        /// search for read-only parts that can display this mimetype
        KService::List list = KMimeTypeTrader::self()->query(mimetype, QLatin1String("KParts/ReadOnlyPart"));
        if (!list.isEmpty()) {
            const QString key = QString(QLatin1String("%1:%2:%3")).arg(mimetype).arg(extension).arg(description);
            /// add three-column info to list of all mime types
            allMimeTypes.append(key);
            if (!config.readEntry(mimetype, false)) {
                /// add three-column info to list of enabled mime types only if not blacklisted
                enabledMimeTypes.append(key);
            } else
                kDebug() << "Skipping blacklisted (user config) mime type " << mimetype;
        }
    }
}

/// To order the mimetype list
bool MimeTypeHelper::mimeTypeLessThan(const KMimeType::Ptr &m1, const KMimeType::Ptr &m2)
{
    return m1->name() < m2->name();
}
