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

#ifndef KCM_KPARTSPLUGIN_H
#define KCM_KPARTSPLUGIN_H

#include <kcmodule.h>

class KCMKPartsPlugin : public KCModule
{
    Q_OBJECT

public:
    KCMKPartsPlugin(QWidget *parent = 0, const QVariantList &args = QVariantList());
    ~KCMKPartsPlugin();

public slots:
    virtual void load();
    virtual void save();
    virtual void defaults();

private:
    class KCMKPartsPluginPrivate;
    KCMKPartsPluginPrivate *d;

private slots:
    void internalDataChanged();
};

#endif // KCM_KPARTSPLUGIN_H
