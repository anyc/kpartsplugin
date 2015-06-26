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

#include <QLayout>
#include <QLabel>
#include <QTreeView>
#include <QApplication>

#include <KLocale>
#include <KAboutData>
#include <KPluginFactory>
#include <KPushButton>

#include "kcm_kpartsplugin.h"
#include "mimetypestreeview.h"
#include "mimetypehelper.h"

K_PLUGIN_FACTORY(KCMKPartsPluginFactory, registerPlugin<KCMKPartsPlugin>();)
K_EXPORT_PLUGIN(KCMKPartsPluginFactory("kcm_kpartsplugin", "kcm_kpartsplugin"))

class KCMKPartsPlugin::KCMKPartsPluginPrivate
{
private:
    KCMKPartsPlugin *p;
    QTreeView *m_treeview;

public:
    MimeTypesItemModel *model;

    KCMKPartsPluginPrivate(KCMKPartsPlugin *parent)
        : p(parent), m_treeview(NULL), model(NULL) {
        // nothing
    }

    void createGui() {
        QGridLayout *layout = new QGridLayout(p);

        QLabel *label = new QLabel(i18n("Select which mime types should be supported by KPartsPlugin, the browser plugin for non-KDE browsers like Firefox or Opera."), p);
        label->setWordWrap(true);
        layout->addWidget(label, 0, 0, 1, 3);

        m_treeview = new QTreeView(p);
        layout->addWidget(m_treeview, 1, 0, 1, 3);
        layout->setColumnStretch(0, 1);
        layout->setColumnStretch(1, 0);
        layout->setColumnStretch(2, 0);

        model = new MimeTypesItemModel(m_treeview);
        m_treeview->setModel(model);
        connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), p, SLOT(internalDataChanged()));

        KPushButton *button = new KPushButton(KIcon("dialog-ok-apply"), i18n("Enable all"), p);
        layout->addWidget(button, 2, 1, 1, 1);
        connect(button, SIGNAL(clicked()), model, SLOT(enableAllMimetypes()));

        button = new KPushButton(KIcon("dialog-cancel"), i18n("Disable all"), p);
        layout->addWidget(button, 2, 2, 1, 1);
        connect(button, SIGNAL(clicked()), model, SLOT(disableAllMimetypes()));
    }
};

KCMKPartsPlugin::KCMKPartsPlugin(QWidget *parent, const QVariantList &args)
    : KCModule(KCMKPartsPluginFactory::componentData(), parent, args), d(new KCMKPartsPluginPrivate(this))
{
    KAboutData *aboutData = new KAboutData("kcm_kpartsplugin", 0, ki18n("KPartsPlugin"), "2012-07-23", ki18n("KPartsPlugin"), KAboutData::License_GPL, ki18n("2010-2012 Thomas Fischer"));
    setAboutData(aboutData);

    d->createGui();
}

KCMKPartsPlugin::~KCMKPartsPlugin()
{
    // TODO
}

void KCMKPartsPlugin::load()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    d->model->load();
    QApplication::restoreOverrideCursor();
    emit changed(false);
}

void KCMKPartsPlugin::save()
{
    d->model->save();
    emit changed(false);
}

void KCMKPartsPlugin::defaults()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    d->model->resetToDefaults();
    QApplication::restoreOverrideCursor();
    emit changed(true);
}

void KCMKPartsPlugin::internalDataChanged()
{
    emit changed(true);
}
