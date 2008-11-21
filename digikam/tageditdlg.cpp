/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-01
 * Description : dialog to edit and create digiKam Tags
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */


#include "tageditdlg.h"
#include "tageditdlg.moc"

// Qt includes.

#include <QLabel>
#include <QLayout>
#include <QGridLayout>
#include <QTreeWidget>

// KDE includes.

#include <kdebug.h>
#include <klineedit.h>
#include <klocale.h>
#include <kicondialog.h>
#include <kapplication.h>
#include <kdeversion.h>
#include <kiconloader.h>
#include <kseparator.h>
#include <kstandarddirs.h>

// Local includes.

#include "album.h"
#include "syncjob.h"
#include "searchtextbar.h"

namespace Digikam
{


class TagsListCreationErrorDialog : public KDialog
{

public:

    TagsListCreationErrorDialog(QWidget* parent, const QMap<QString, QString>& errMap);
    ~TagsListCreationErrorDialog(){};
};

// ------------------------------------------------------------------------------

class TagEditDlgPriv
{
public:

    TagEditDlgPriv()
    {
        titleEdit       = 0;
        iconButton      = 0;
        resetIconButton = 0;
        mainRootAlbum   = 0;
        topLabel        = 0;
        create          = false;
    }

    bool           create;

    QLabel        *topLabel;

    QString        icon;

    QPushButton   *iconButton;
    QPushButton   *resetIconButton;

    TAlbum        *mainRootAlbum;
    SearchTextBar *titleEdit;
};

TagEditDlg::TagEditDlg(QWidget *parent, TAlbum* album, bool create)
          : KDialog(parent)
{
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setModal(true);
    setHelp("tagscreation.anchor", "digikam");

    if (create) setCaption(i18n("New Tag"));
    else        setCaption(i18n("Edit Tag"));

    d = new TagEditDlgPriv;
    d->mainRootAlbum = album;
    d->create        = create;

    QWidget *page = new QWidget(this);
    setMainWidget(page);

    // --------------------------------------------------------

    QGridLayout* grid = new QGridLayout(page);
    QLabel *logo      = new QLabel(page);
    logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                            .scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    d->topLabel = new QLabel(page);
    d->topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->topLabel->setWordWrap(false);

    KSeparator *line = new KSeparator(Qt::Horizontal, page);

    // --------------------------------------------------------

    QLabel *titleLabel = new QLabel(page);
    titleLabel->setText(i18n("&Title:"));

    d->titleEdit = new SearchTextBar(page, "TagEditDlgTitleEdit", i18n("Enter tag name here..."));
    titleLabel->setBuddy(d->titleEdit);

    QLabel *tipLabel = new QLabel(page);
    tipLabel->setTextFormat(Qt::RichText);
    tipLabel->setWordWrap(true);
    tipLabel->setText(i18n("<p>To create new tags, you can use the following rules:</p>"
                           "<p><ul><li>'/' can be used to create a tags hierarchy.<br/>"
                           "Ex.: <i>\"Country/City/Paris\"</i></li>"
                           "<li>',' can be used to create more than one tags hierarchy at the same time.<br/>"
                           "Ex.: <i>\"City/Paris, Monument/Notre-Dame\"</i></li>"
                           "<li>If a tag hierarchy starts with '/', root tag album is used as parent.</li></ul></p>"
                           ));

    if (d->create)
    {
        AlbumList tList = AlbumManager::instance()->allTAlbums();
        for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
        {
            TAlbum *tag = dynamic_cast<TAlbum*>(*it);
            d->titleEdit->completionObject()->addItem(tag->tagPath());
        }
    }
    else
    {
        d->titleEdit->setText(d->mainRootAlbum->title());
        tipLabel->hide();
    }

    QLabel *iconTextLabel = new QLabel(page);
    iconTextLabel->setText(i18n("&Icon:"));

    d->iconButton = new QPushButton(page);
    d->iconButton->setFixedSize(40, 40);
    iconTextLabel->setBuddy(d->iconButton);

    // In create mode, by default assign the icon of the parent (if not root) to this new tag.
    if (create && !album->isRoot())
        d->icon = album->icon();
    else
        d->icon = album->icon();

    d->iconButton->setIcon(SyncJob::getTagThumbnail(album));

    d->resetIconButton = new QPushButton(KIcon("view-refresh"), i18n("Reset"), page);
    if (create) d->resetIconButton->hide();

    // --------------------------------------------------------

    grid->addWidget(logo,               0, 0, 4, 1);
    grid->addWidget(d->topLabel,        0, 1, 1, 4);
    grid->addWidget(line,               1, 1, 1, 4);
    grid->addWidget(tipLabel,           2, 1, 1, 4);
    grid->addWidget(titleLabel,         3, 1, 1, 1);
    grid->addWidget(d->titleEdit,       3, 2, 1, 3);
    grid->addWidget(iconTextLabel,      4, 1, 1, 1);
    grid->addWidget(d->iconButton,      4, 2, 1, 1);
    grid->addWidget(d->resetIconButton, 4, 3, 1, 1);
    grid->setColumnStretch(4, 10);
    grid->setRowStretch(5, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(d->iconButton, SIGNAL(clicked()),
            this, SLOT(slotIconChanged()));

    connect(d->resetIconButton, SIGNAL(clicked()),
            this, SLOT(slotIconResetClicked()));

    connect(d->titleEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTitleChanged(const QString&)));

    // --------------------------------------------------------

    slotTitleChanged(d->titleEdit->text());
    d->titleEdit->setFocus();
    adjustSize();
}

TagEditDlg::~TagEditDlg()
{
    delete d;
}

QString TagEditDlg::title() const
{
    return d->titleEdit->text();
}

QString TagEditDlg::icon() const
{
    return d->icon;
}

void TagEditDlg::slotIconResetClicked()
{
    d->icon = QString("tag");
    d->iconButton->setIcon(KIconLoader::global()->loadIcon(d->icon, KIconLoader::NoGroup, 20));
}

void TagEditDlg::slotIconChanged()
{
    KIconDialog dlg(this);
    dlg.setup(KIconLoader::NoGroup, KIconLoader::Application, false, 20, false, false, false);
    QString icon = dlg.openDialog();

    if (icon.isEmpty() || icon == d->icon)
        return;

    d->icon = icon;
    d->iconButton->setIcon(KIconLoader::global()->loadIcon(d->icon, KIconLoader::NoGroup, 20));
}

void TagEditDlg::slotTitleChanged(const QString& newtitle)
{
    QString tagName = d->mainRootAlbum->tagPath();
    if (tagName.endsWith('/') && !d->mainRootAlbum->isRoot())
        tagName.truncate(tagName.length()-1);

    if (d->create)
    {
        if (d->titleEdit->text().startsWith('/'))
        {
            d->topLabel->setText(i18n("<b>Create New Tag</b>"));
        }
        else
        {
            d->topLabel->setText(i18n("<b>Create New Tag in<br/>"
                                      "<i>\"%1\"</i></b>", tagName));
        }
    }
    else
    {
        d->topLabel->setText(i18n("<b>Properties of Tag<br/>"
                                  "<i>\"%1\"</i></b>", tagName));
    }

    enableButtonOk(!newtitle.isEmpty());
}

bool TagEditDlg::tagEdit(QWidget *parent, TAlbum* album, QString& title, QString& icon)
{
    TagEditDlg dlg(parent, album);

    bool valRet = dlg.exec();
    if (valRet == QDialog::Accepted)
    {
        title = dlg.title();
        icon  = dlg.icon();
    }

    return valRet;
}

bool TagEditDlg::tagCreate(QWidget *parent, TAlbum* album, QString& title, QString& icon)
{
    TagEditDlg dlg(parent, album, true);

    bool valRet = dlg.exec();
    if (valRet == QDialog::Accepted)
    {
        title = dlg.title();
        icon  = dlg.icon();
    }

    return valRet;
}

AlbumList TagEditDlg::createTAlbum(TAlbum *mainRootAlbum, const QString& tagStr, const QString& icon,
                                   QMap<QString, QString>& errMap)
{
    errMap.clear();
    AlbumList createdTagsList;

    // Check if new tags are include in a list of tags hierarchy separated by ','.
    // Ex: /Country/France/people,/City/France/Paris

    const QStringList tagsHierarchies = tagStr.split(',', QString::SkipEmptyParts);
    if (tagsHierarchies.isEmpty())
        return createdTagsList;

    for (QStringList::const_iterator it = tagsHierarchies.constBegin(); it != tagsHierarchies.constEnd(); ++it)
    {
        QString hierarchy = (*it).trimmed();
        if (!hierarchy.isEmpty())
        {
            // Check if new tags is a hierarchy of tags separated by '/'.

            TAlbum *root = 0;

            if (hierarchy.startsWith('/') || !mainRootAlbum)
                root = AlbumManager::instance()->findTAlbum(0);
            else
                root = mainRootAlbum;

            QStringList tagsList = hierarchy.split('/', QString::SkipEmptyParts);
            kDebug(50003) << tagsList << endl;

            if (!tagsList.isEmpty())
            {
                for (QStringList::iterator it2 = tagsList.begin(); it2 != tagsList.end(); ++it2)
                {
                    QString tagPath, errMsg;
                    QString tag = (*it2).trimmed();
                    if (root->isRoot())
                        tagPath = QString("/%1").arg(tag);
                    else
                        tagPath = QString("%1/%2").arg(root->tagPath()).arg(tag);

                    kDebug(50003) << tag << " :: " << tagPath << endl;

                    if (!tag.isEmpty())
                    {
                        // Tag already exist ?
                        TAlbum* album = AlbumManager::instance()->findTAlbum(tagPath);
                        if (!album)
                        {
                            root = AlbumManager::instance()->createTAlbum(root, tag, icon, errMsg);
                        }
                        else
                        {
                            root = album;
                            if (*it2 == tagsList.last())
                                errMap.insert(tagPath, i18n("Tag name already exists"));
                        }

                        if (root)
                            createdTagsList.append(root);
                    }

                    // Sanity check if tag creation failed.
                    if (!root)
                    {
                        errMap.insert(tagPath, errMsg);
                        break;
                    }
                }
            }
        }
    }

    return createdTagsList;
}

void TagEditDlg::showtagsListCreationError(QWidget* parent, const QMap<QString, QString>& errMap)
{
    if (!errMap.isEmpty())
    {
        TagsListCreationErrorDialog dlg(parent, errMap);
        dlg.exec();
    }
}

// ------------------------------------------------------------------------------

TagsListCreationErrorDialog::TagsListCreationErrorDialog(QWidget* parent, const QMap<QString, QString>& errMap)
                           : KDialog(parent)
{
    setButtons(Help|Ok);
    setDefaultButton(Ok);
    setModal(true);
    setHelp("tagscreation.anchor", "digikam");
    setCaption(i18n("Tag creation Error"));

    QWidget *box = new QWidget(this);
    setMainWidget(box);
    QVBoxLayout* vLay = new QVBoxLayout(box);

    QLabel *label       = new QLabel(i18n("Error been occurred during Tag creation:"), box);
    QTreeWidget *listView = new QTreeWidget(box);
    listView->setHeaderLabels(QStringList() << i18n("Tag Path") << i18n("Error"));
    listView->setRootIsDecorated(false);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);
    listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    vLay->addWidget(label);
    vLay->addWidget(listView);
    vLay->setMargin(0);
    vLay->setSpacing(0);

    for (QMap<QString, QString>::const_iterator it = errMap.begin() ; it != errMap.end() ; ++it)
        new QTreeWidgetItem(listView, QStringList() << it.key() << it.value());

    adjustSize();
}

}  // namespace Digikam
