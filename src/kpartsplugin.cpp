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

#include <QMetaClassInfo>
#include <QLayout>
#include <QLabel>
#include <QIODevice>
#include <QTreeWidget>
#include <QMenu>
#include <QSignalMapper>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>

#include <KAction>
#include <KMenuBar>
#include <KToolBar>
#include <KStandardAction>
#include <KActionCollection>
#include <KListWidget>
#include <KMessageBox>
#include <KTemporaryFile>
#include <KFileDialog>
#include <KPushButton>
#include <KMimeType>
#include <KMimeTypeTrader>
#include <KIcon>
#include <KLocale>
#include <KRun>
#include <KDebug>
#include <KStandardDirs>

#include <qtbrowserplugin.h>
#include "kpartsplugin.h"
#include "mimetypehelper.h"
#include "mimetypestreeview.h"

static QStringList allMimeTypes, enabledMimeTypes;

class KPPServiceListDialog : public KDialog
{
public:
    KListWidget *listWidget;
    QCheckBox *checkBoxRemember;

    /**
     * Create a dialog which presents the user a list of choices. One choice is pre-selected.
     * A checkbox allows the user to store his/her preference.
     */
    KPPServiceListDialog(const QStringList &list, const QString &selected, const QString &caption, const QString &text, QWidget *parent = NULL, Qt::WFlags flags = NULL)
        : KDialog(parent, flags) {
        /// set various dialog configurations
        setWindowModality(Qt::NonModal);
        setCaption(caption);
        setButtons(KDialog::Ok);

        /// central widget for dialog
        QWidget *container = new QWidget(this);
        setMainWidget(container);
        QGridLayout *layout = new QGridLayout(container);

        /// label holding the icon in the dialog's upper left corner
        QLabel *label = new QLabel(container);
        label->setPixmap(KIconLoader::global()->loadIcon("preferences-desktop-filetype-association", KIconLoader::NoGroup, KIconLoader::SizeLarge));
        layout->addWidget(label, 0, 0, 3, 1, Qt::AlignTop);

        /// label showing the text message
        label = new QLabel(text, container);
        label->setTextFormat(Qt::RichText);
        layout->addWidget(label, 0, 1, 1, 1, Qt::AlignTop);
        label->setWordWrap(true);

        /// list with all choices for the user
        listWidget = new KListWidget(container);
        label->setBuddy(listWidget);
        for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
            QListWidgetItem *item = new QListWidgetItem(*it, listWidget);
            item->setSelected(*it == selected);
            listWidget->addItem(item);
        }
        listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        layout->addWidget(listWidget, 1, 1, 1, 1);

        /// checkbox to set remembering the default choice
        checkBoxRemember = new QCheckBox(i18n("Remember selection as default"), container);
        checkBoxRemember->setTristate(false);
        layout->addWidget(checkBoxRemember, 2, 1, 1, 1);

        /// double-clicking on list item is like pressing "Ok"
        connect(listWidget, SIGNAL(doubleClicked(QListWidgetItem *, const QPoint &)), this, SLOT(accept()));
    }

    /**
     * Static function simplifying using KPPServiceListDialog. Variable rememberChoice is a reference
     * returning the checkbox's check state.
     */
    static QString selectStringFromList(bool &rememberChoice, const QStringList &list, const QString &selected, const QString &caption, const QString &text, QWidget *parent = NULL, Qt::WFlags flags = NULL) {
        Q_ASSERT(list.size() > 0);

        KPPServiceListDialog dlg(list, selected, caption, text, parent, flags);
        dlg.activateWindow();
        dlg.raise();
        dlg.exec();
        rememberChoice = dlg.checkBoxRemember->checkState() == Qt::Checked;
        /// fetch first (and only) selected item in list
        QList<QListWidgetItem *> it(dlg.listWidget->selectedItems());
        return it.at(0)->text();
    }

};

KPartsPlugin::KPartsPlugin(QWidget *parent)
    : QWidget(parent), QtNPBindable(), m_part(NULL), m_gridLayout(NULL), m_listMimeTypes(NULL), m_calledOnce(false)
{
    /// set cursor to hourglass or similar
    setCursor(Qt::WaitCursor);
    /// change focus policy to receive keyboard input
    setFocusPolicy(Qt::StrongFocus);
    /// force activation of this plugin
    QApplication::setActiveWindow(this);
    QApplication::setApplicationName("KPartsPlugin");

    /// configure temporary file
    m_tempFile.setPrefix("KPartsPlugin");
    m_tempFile.setAutoRemove(true);

    /// fetch all mime types
    MimeTypeHelper::initAllMimeTypes(allMimeTypes, enabledMimeTypes);

    /// create user interface
    setupInternalGUI();
}

KService::Ptr KPartsPlugin::selectService(const QString &format)
{
    QMap<QString, KService::Ptr> nameToKService;
    QStringList serviceList;
    /// determine preferred service as configured in the KDE settings
    KService::Ptr preferredService = KMimeTypeTrader::self()->preferredService(format, QLatin1String("KParts/ReadOnlyPart"));
    /// stick with preferred service unless there are several to choose from
    KService::Ptr service = preferredService;

    /// fetch list of available services
    KService::List list = KMimeTypeTrader::self()->query(format, QLatin1String("KParts/ReadOnlyPart"));
    for (KService::List::Iterator it = list.begin(); it != list.end(); ++it)
        /// exclude Netscape plugins (remember, this is a *KDE* plugin)
        if (!(*it)->name().contains(QLatin1String("Netscape"), Qt::CaseInsensitive)) {
            /// store both service's name and mapping from name to actual service
            serviceList << (*it)->name();
            nameToKService.insert((*it)->name(), *it);
        }

    /// fall back to plain text handler for unknown text file types
    if (serviceList.isEmpty() && format.startsWith(QLatin1String("text/"), Qt::CaseInsensitive)) {
        KService::List list = KMimeTypeTrader::self()->query(QLatin1String("text/plain"), QLatin1String("KParts/ReadOnlyPart"));
        for (KService::List::Iterator it = list.begin(); it != list.end(); ++it)
            /// exclude Netscape plugins (remember, this is a *KDE* plugin)
            if (!(*it)->name().contains(QLatin1String("Netscape"), Qt::CaseInsensitive)) {
                /// store both service's name and mapping from name to actual service
                serviceList << (*it)->name();
                nameToKService.insert((*it)->name(), *it);
            }
    }

    /// if more than one service has been found, use stored preferred service or user choice to select
    if (serviceList.count() > 1) {
        /// load configuration data for preferred services (independent from KDE's preferred service!)
        KSharedPtr<KSharedConfig> userConfig = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", configFilename), KConfig::SimpleConfig);
        KConfigGroup config(userConfig, configSectionPreferredService);

        QString name = config.readEntry(format, "");
        /// if no preference for this mime type has been saved, show selection dialog
        if (name.isEmpty()) {
            bool rememberChoice = false;
            setEnabled(false);
            name = KPPServiceListDialog::selectStringFromList(rememberChoice, serviceList, preferredService->name(), i18n("KPart Selection"), i18n("Select the KPart to be used for the mime type<br/><b>%1</b>.<br/>The default part<br/><b>%2</b><br/>has been selected.", format, preferredService->name()));
            /// if user has checked to store preference ...
            if (rememberChoice) {
                /// write updated configuration
                config.writeEntry(format, name);
                userConfig->sync();
            }
            setEnabled(true);
        }
        /// determine which service to use eventually
        if (nameToKService.contains(name))
            service = nameToKService[name];
    }

    return service;
}

bool KPartsPlugin::readData(QIODevice *source, const QString &format, const QUrl &url)
{
    m_url = url;

    /// fetch information on mime type
    KMimeType::Ptr mimeTypeKDE = KMimeType::mimeType(format);
    if (mimeTypeKDE.isNull()) {
        /// show error message and exit gracefully
        m_label->setText(i18n("Failed to load KPart, could not interpret mime type \"%1\".", format));
        setCursor(Qt::ArrowCursor);
        return false;
    }
    m_mimeTypeLabel->setText(i18n("<b>%1</b> (%2)", format, mimeTypeKDE->comment()));

    /// prevent running this function twice
    if (m_calledOnce) {
        kWarning() << " readData was called multiple times!";
        return false;
    }
    m_calledOnce = true;

    m_copyUrlButton->setEnabled(m_url.isValid());
    if (m_url.isValid())
        m_copyUrlButton->setToolTip(m_url.toString());

    /// allow UI to update
    QCoreApplication::instance()->processEvents();

    /// copy io data into temporary file using a suffix matching to mime type
    m_label->setText(i18n("Copying data from \"%1\"...", m_url.toString()));
    m_tempFile.setSuffix(mimeTypeKDE->mainExtension());
    if (m_url.isValid())
        m_tempFile.setPrefix(QFileInfo(m_url.path()).baseName());
    copyIODevice(source, &m_tempFile);
    /// allow UI to update
    QCoreApplication::instance()->processEvents();

    /// locate and load KPart plugin for this mime type
    m_label->setText(i18n("Locating KPart ..."));
    KService::Ptr service = selectService(format);
    /// allow UI to update
    QCoreApplication::instance()->processEvents();

    if (service->isValid()) {
        m_label->setText(i18n("Initializing ..."));
        m_part = service->createInstance<KParts::ReadOnlyPart>((QWidget *)this, (QObject *)this);

        /// initialize plugin, set layout
        QWidget *partWidget = m_part->widget();

        setupMenuToolBars();

        /// open temporary file in KPart plugin
        m_part->openUrl(KUrl::fromLocalFile(m_tempFile.fileName()));

        /// clean up memory and restore curser
        delete m_label;
        delete m_listMimeTypes;
        m_listMimeTypes = NULL;

        m_toolbar->setVisible(true);
        m_menubar->setVisible(true);
        partWidget->setVisible(true);
        m_gridLayout->addWidget(partWidget, 2, 0, 2, 4);

        setCursor(Qt::ArrowCursor);

        /// enable buttons for opening in external program and saving as file
        m_openButton->setEnabled(true);
        m_saveButton->setEnabled(true);
    } else {
        /// show error message and exit gracefully
        m_label->setText(i18n("Failed to load KPart for mime type \"%1\".", format));
        setCursor(Qt::ArrowCursor);
        return false;
    }

    return true;
}

bool KPartsPlugin::copyIODevice(QIODevice *source, QIODevice *target)
{
    /// initialize buffer for copy operation
    const int bufferSize = 32768;
    char buffer[bufferSize];

    /// open input and output devices
    source->open(QIODevice::ReadOnly);
    target->open(QIODevice::WriteOnly);

    /// as long as there are bytes to load ...
    qint64 ba = source->bytesAvailable();
    while (ba > 0) {
        /// read from input, write to output
        qint64 br = source->read(buffer, bufferSize);
        qint64 bw = target->write(buffer, br);
        if (br != bw) {
            /// io operations messed up
            source->close();
            target->close();
            return false;
        }
        ba = source->bytesAvailable();
    }
    source->close();
    target->close();

    return true;
}

bool KPartsPlugin::setupInternalGUI()
{
    setLayout(m_gridLayout = new QGridLayout(this));

    /// set streching for columns and rows
    m_gridLayout->setColumnStretch(0, 10);
    m_gridLayout->setColumnStretch(1, 1);
    m_gridLayout->setColumnStretch(2, 0);
    m_gridLayout->setColumnStretch(3, 0);
    m_gridLayout->setRowStretch(0, 0);
    m_gridLayout->setRowStretch(1, 0);
    m_gridLayout->setRowStretch(2, 0);
    m_gridLayout->setRowStretch(3, 1);

    /// configure central message label
    m_label = new QLabel(i18n("Initializing ..."), this);
    m_label->setAlignment(Qt::AlignCenter);

    /// add label about which mime type has been loaded
    m_mimeTypeLabel = new QLabel(this);
    m_mimeTypeLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_mimeTypeLabel->setWordWrap(true);

    /// add push button to copy URL into clipboard
    m_copyUrlButton = new KPushButton(KIcon("edit-copy"), i18n("Copy URL"), this);
    connect(m_copyUrlButton, SIGNAL(clicked()), this, SLOT(slotCopyUrl()));
    m_copyUrlButton->setEnabled(false);

    /// add push button to open current file in external program
    m_openButton = new KPushButton(KIcon("document-open"), i18n("Open in Program ..."), this);
    connect(m_openButton, SIGNAL(clicked()), this, SLOT(slotOpenTempFile()));
    m_openButton->setEnabled(false);

    /// add push button to save current file in an user-specified file
    m_saveButton = new KPushButton(KIcon("document-save-as"), i18n("Save Copy ..."), this);
    connect(m_saveButton, SIGNAL(clicked()), this, SLOT(slotSaveTempFile()));
    m_saveButton->setEnabled(false);

    /// setup large list with mime types shown while data for file is transferred
    m_listMimeTypes = new QTreeWidget(this);
    QFontMetrics fm(m_listMimeTypes->font());
    const int x = fm.width("AwIkKJ0235W");
    m_listMimeTypes->setSelectionMode(QAbstractItemView::NoSelection);
    m_listMimeTypes->setHeaderLabels(QStringList() << i18n("Mime type") << i18n("Extension") << i18n("Description"));
    m_listMimeTypes->setColumnWidth(0, x * 6);
    m_listMimeTypes->setColumnWidth(1, x);
    m_listMimeTypes->setColumnWidth(2, x * 12);

    /// fill list of mime types
    QMap<QString, QMenu *> subMenuMap;
    for (QStringList::ConstIterator it = allMimeTypes.constBegin(); it != allMimeTypes.constEnd(); ++it) {
        /// setup list item
        KIcon icon = enabledMimeTypes.contains(*it) ? KIcon("dialog-ok-apply") : KIcon("dialog-cancel");
        QTreeWidgetItem *item = new QTreeWidgetItem(m_listMimeTypes);
        item->setIcon(0, icon);
        QStringList list = (*it).split(":");
        item->setText(0, list[0]);
        item->setText(1, list[1]);
        item->setText(2, list[2]);
    }

    m_menubar = new KMenuBar(this);
    m_menubar->setBackgroundRole(QPalette::Window);
    m_menubar->setVisible(false);

    m_toolbar = new KToolBar(this);
    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolbar->setBackgroundRole(QPalette::Window);
    m_toolbar->setVisible(false);

    /// add widgets to layout
    m_gridLayout->addWidget(m_menubar, 0, 0, 1, 1);
    m_gridLayout->addWidget(m_mimeTypeLabel, 0, 1, 1, 3);
    m_gridLayout->addWidget(m_toolbar, 1, 0, 1, 1);
    m_gridLayout->addWidget(m_copyUrlButton, 1, 1, 1, 1);
    m_gridLayout->addWidget(m_openButton, 1, 2, 1, 1);
    m_gridLayout->addWidget(m_saveButton, 1, 3, 1, 1);
    m_gridLayout->addWidget(m_label, 2, 0, 1, 4);
    m_gridLayout->addWidget(m_listMimeTypes, 3, 0, 1, 4);

    m_label->setText(i18n("Waiting for data ..."));

    return true;
}

void KPartsPlugin::slotCopyUrl()
{
    if (m_url.isValid())
        QApplication::clipboard()->setText(m_url.toString());
}

void KPartsPlugin::slotOpenTempFile()
{
    /// use KDE subsystem to open temporary file with proper application
    KRun::runUrl(KUrl::fromLocalFile(m_tempFile.fileName()), mimeType(), this);
}

void KPartsPlugin::slotSaveTempFile()
{
    /// determine file name to save data to
    const QString defaultFilename = m_url.isValid() ? QFileInfo(m_url.path()).fileName() : QString();
    QString fileName = KFileDialog::getSaveFileName(KUrl::fromLocalFile(defaultFilename), mimeType(), this);
    if (fileName.isEmpty() || fileName.isNull()) return;

    /// save data by copying temporary file to new file
    QFile file(fileName);
    copyIODevice(&m_tempFile, &file);
}

void KPartsPlugin::enterEvent(QEvent *event)
{
    /** code from Jeremy Sanders */
    // this is required because firefox stops sending keyboard
    // events to the plugin after opening windows (e.g. download dialog)
    // setting the active window brings the events back
    if (QApplication::activeWindow() == NULL) {
        QApplication::setActiveWindow(this);
    }

    QWidget::enterEvent(event);
}

void KPartsPlugin::setupMenuToolBars()
{
    KAction *printAction = KStandardAction::print(m_part, SLOT(slotPrint()), m_part->actionCollection());
    printAction->setEnabled(false);
    connect(m_part, SIGNAL(enablePrintAction(bool)), printAction, SLOT(setEnabled(bool)));

    QDomDocument doc = m_part->domDocument();
    QDomElement docElem = doc.documentElement();

    QDomNodeList toolbarNodes = docElem.elementsByTagName("ToolBar");
    for (int i = 0; i < toolbarNodes.count(); ++i) {
        QDomNodeList toolbarItems = toolbarNodes.at(i).childNodes();
        for (int j = 0; j < toolbarItems.count(); ++j) {
            QDomNode toolbarItem = toolbarItems.at(j);
            if (toolbarItem.nodeName() == QLatin1String("Action")) {
                QString actionName = toolbarItem.attributes().namedItem(QLatin1String("name")).nodeValue();
                m_toolbar->addAction(m_part->actionCollection()->action(actionName));
            } else if (toolbarItem.nodeName() == QLatin1String("Separator")) {
                m_toolbar->addSeparator();
            }
        }
    }

    QDomNodeList menubarNodes = docElem.elementsByTagName("MenuBar");
    for (int i = 0; i < menubarNodes.count(); ++i) {
        QDomNodeList menubarNode = menubarNodes.at(i).childNodes();
        for (int j = 0; j < menubarNode.count(); ++j) {
            QDomNode menubarItem = menubarNode.at(j);
            if (menubarItem.nodeName() == QLatin1String("Menu")) {
                QDomNodeList menuNode = menubarItem.childNodes();
                QString text;
                for (int k = 0; k < menuNode.count(); ++k) {
                    QDomNode menuItem = menuNode.at(k);
                    if (menuItem.nodeName() == QLatin1String("text")) {
                        text = menuItem.firstChild().toText().data();
                        break;
                    }
                }
                QMenu *menu = m_menubar->addMenu(text);

                for (int k = 0; k < menuNode.count(); ++k) {
                    QDomNode menuItem = menuNode.at(k);
                    if (menuItem.nodeName() == QLatin1String("Action")) {
                        QString actionName = menuItem.attributes().namedItem(QLatin1String("name")).nodeValue();
                        menu->addAction(m_part->actionCollection()->action(actionName));
                    } else if (menuItem.nodeName() == QLatin1String("Separator")) {
                        menu->addSeparator();
                    }
                }
            }
        }
    }

    QDomNodeList actionPropertiesList = docElem.elementsByTagName("ActionProperties");
    for (int i = 0; i < actionPropertiesList.count(); ++i) {
        QDomNodeList actionProperties = actionPropertiesList.at(i).childNodes();
        for (int j = 0; j < actionProperties.count(); ++j) {
            QDomNode actionNode = actionProperties.at(j);
            if (actionNode.nodeName() == QLatin1String("Action")) {
                const QString actionName = actionNode.attributes().namedItem("name").toAttr().nodeValue();
                const QString actionShortcut = actionNode.attributes().namedItem("shortcut").toAttr().value();
                QAction *action = m_part->actionCollection()->action(actionName);
                if (action != NULL)
                    action->setShortcut(QKeySequence(actionShortcut));
            }
        }
    }
}


#include "kpartsplugin.moc"


class QtNPClassList : public QtNPFactory
{
private:
    QString m_name, m_description;

public:
    QtNPClassList()
        : m_name("KParts Plugin"), m_description("File viewer using KDE's KParts technology (2012-07-23)") {
        MimeTypeHelper::initAllMimeTypes(allMimeTypes, enabledMimeTypes);
    }

    QObject *createObject(const QString &) const {
        return new KPartsPlugin();
    }

    QStringList mimeTypes() const {
        return enabledMimeTypes;
    }

    QString pluginName() const {
        return m_name;
    }

    QString pluginDescription() const {
        return m_description;
    }

};

QtNPFactory *qtns_instantiate()
{
    return new QtNPClassList;
}
