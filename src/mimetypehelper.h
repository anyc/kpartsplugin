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

#ifndef MIMETYPEHELPER_H
#define MIMETYPEHELPER_H

#include <QStringList>

#include <KMimeType>

extern const QString configFilename;
extern const QString configSectionBlacklisted;
extern const QString configSectionPreferredService;

class MimeTypeHelper
{
public:

/// built-in list of mime types that should never be loaded with this plugin
/// comparison is done with "startsWith", so "inode/" covers e.g. "inode/directory"
    static const QStringList builtinBlacklisted;

    static void initAllMimeTypes(QStringList &allMimeTypes, QStringList &enabledMimeTypes);

private:
/// To order the mimetype list
    static bool mimeTypeLessThan(const KMimeType::Ptr &m1, const KMimeType::Ptr &m2);

};

#endif // MIMETYPEHELPER_H
