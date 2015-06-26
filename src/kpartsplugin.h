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

#ifndef KPARTSPLUGIN_H
#define KPARTSPLUGIN_H

#include <QWidget>
#include <QString>

#include <kparts/part.h>

#include "qtbrowserplugin.h"

class QLabel;
class QGridLayout;
class QIODevice;
class KToolBar;
class KMenuBar;
class KTemporaryFile;
class KPushButton;
class QTreeWidget;

class KPartsPlugin : public QWidget, public QtNPBindable
{
    Q_OBJECT

public:
    KPartsPlugin(QWidget *parent = NULL);

    bool readData(QIODevice *source, const QString &format, const QUrl &url = QUrl());

protected:
    bool copyIODevice(QIODevice *source, QIODevice *target);
    bool setupInternalGUI();
    KService::Ptr selectService(const QString &format);
    void enterEvent(QEvent *event);
    void setupMenuToolBars();

protected slots:
    void slotCopyUrl();
    void slotOpenTempFile();
    void slotSaveTempFile();

private:
    KParts::ReadOnlyPart *m_part;
    QGridLayout *m_gridLayout;
    QLabel *m_mimeTypeLabel;
    QLabel *m_label;
    KMenuBar *m_menubar;
    KToolBar *m_toolbar;
    QMenu *m_supportedMimeTypesMenu;
    KPushButton *m_copyUrlButton;
    KPushButton *m_openButton;
    KPushButton *m_saveButton;
    QTreeWidget *m_listMimeTypes;
    KTemporaryFile m_tempFile;
    QUrl m_url;

    bool m_calledOnce;
};

#endif
