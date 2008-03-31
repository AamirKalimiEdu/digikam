/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Captions, Tags, and Rating properties editor
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

// Qt includes.

#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QIcon>
#include <QKeyEvent>
#include <QGridLayout>
#include <QPixmap>
#include <QEvent>
#include <QSignalMapper>

// KDE includes.

#include <kmenu.h>
#include <klocale.h>
#include <kurl.h>
#include <kcursor.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <ktextedit.h>
#include <kconfig.h>
#include <klineedit.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kselectaction.h>
#include <kvbox.h>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "kdatetimeedit.h"
#include "albumiconitem.h"
#include "albumdb.h"
#include "album.h"
#include "albumsettings.h"
#include "albumthumbnailloader.h"
#include "databasetransaction.h"
#include "tageditdlg.h"
#include "ratingwidget.h"
#include "talbumlistview.h"
#include "tagfilterview.h"
#include "imageinfo.h"
#include "imageattributeswatch.h"
#include "metadatahub.h"
#include "statusprogressbar.h"
#include "searchtextbar.h"
#include "imagedescedittab.h"
#include "imagedescedittab.moc"

#include "config.h"
#ifdef KDEPIMLIBS_FOUND
#include <kabc/stdaddressbook.h>
#endif

namespace Digikam
{

class ImageDescEditTabPriv
{

public:

    ImageDescEditTabPriv()
    {
        modified                   = false;
        ignoreImageAttributesWatch = false;
        recentTagsBtn              = 0;
        commentsEdit               = 0;
        tagsSearchBar              = 0;
        dateTimeEdit               = 0;
        tagsView                   = 0;
        ratingWidget               = 0;
        ABCMenu                    = 0;
        assignedTagsBtn            = 0;
        applyBtn                   = 0;
        revertBtn                  = 0;
        recentTagsMapper           = 0;
        newTagEdit                 = 0;
        toggleAutoTags             = TagFilterView::NoToggleAuto;
    }

    bool                           modified;
    bool                           ignoreImageAttributesWatch;

    QToolButton                   *recentTagsBtn;
    QToolButton                   *assignedTagsBtn;
    QToolButton                   *revertBtn;

    QMenu                         *ABCMenu;
    QMenu                         *moreMenu;

    QSignalMapper                 *recentTagsMapper;

    QPushButton                   *applyBtn;

    QPushButton                   *moreButton;

    KTextEdit                     *commentsEdit;

    KDateTimeEdit                 *dateTimeEdit;

    SearchTextBar                 *tagsSearchBar;
    SearchTextBar                 *newTagEdit;

    ImageInfoList                  currInfos;

    TAlbumListView                *tagsView;

    RatingWidget                  *ratingWidget;

    TagFilterView::ToggleAutoTags  toggleAutoTags;

    MetadataHub                    hub;
};

ImageDescEditTab::ImageDescEditTab(QWidget *parent)
                : QWidget(parent)
{
    d = new ImageDescEditTabPriv;

    QGridLayout *settingsLayout = new QGridLayout(this);

    // Captions/Date/Rating view -----------------------------------

    KVBox *commentsBox = new KVBox(this);
    new QLabel(i18n("Caption:"), commentsBox);
    d->commentsEdit = new KTextEdit(commentsBox);
    d->commentsEdit->setCheckSpellingEnabled(true);

    KHBox *dateBox  = new KHBox(this);
    new QLabel(i18n("Date:"), dateBox);
    d->dateTimeEdit = new KDateTimeEdit(dateBox, "datepicker");
    d->dateTimeEdit->setMaximumHeight( fontMetrics().height()+4 );

    KHBox *ratingBox = new KHBox(this);
    new QLabel(i18n("Rating:"), ratingBox);
    d->ratingWidget  = new RatingWidget(ratingBox);

    // Tags view ---------------------------------------------------

    d->newTagEdit    = new SearchTextBar(this, i18n("Enter new tag here..."));
    d->newTagEdit->setWhatsThis(i18n("Enter here the text used to create new tags. "
                                     "'/' can be used here to create a hierarchy of tags. "
                                     "',' can be used here to create more than one hierarchy at the same time."));

    d->tagsView = new TAlbumListView(this);

    KHBox *tagsSearch = new KHBox(this);
    tagsSearch->setSpacing(KDialog::spacingHint());

    d->tagsSearchBar   = new SearchTextBar(tagsSearch);

    d->assignedTagsBtn = new QToolButton(tagsSearch);
    d->assignedTagsBtn->setToolTip( i18n("Already assigned tags"));
    d->assignedTagsBtn->setIcon(KIconLoader::global()->loadIcon("tag-assigned",
                                KIconLoader::NoGroup, KIconLoader::SizeSmall));
    d->assignedTagsBtn->setCheckable(true);

    d->recentTagsBtn      = new QToolButton(tagsSearch);
    QMenu *recentTagsMenu = new QMenu(d->recentTagsBtn);
    d->recentTagsBtn->setToolTip( i18n("Recent Tags"));
    d->recentTagsBtn->setIcon(KIconLoader::global()->loadIcon("tag-recents", 
                              KIconLoader::NoGroup, KIconLoader::SizeSmall));
    d->recentTagsBtn->setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
    d->recentTagsBtn->setMenu(recentTagsMenu);
    d->recentTagsBtn->setPopupMode(QToolButton::DelayedPopup);
    d->recentTagsMapper = new QSignalMapper(this);

    // Buttons -----------------------------------------

    KHBox *buttonsBox = new KHBox(this);
    buttonsBox->setSpacing(KDialog::spacingHint());

    d->revertBtn = new QToolButton(buttonsBox);
    d->revertBtn->setIcon(SmallIcon("document-revert"));
    d->revertBtn->setMaximumHeight( fontMetrics().height()+4 );
    d->revertBtn->setToolTip( i18n("Revert all changes"));
    d->revertBtn->setEnabled(false);

    d->applyBtn = new QPushButton(i18n("Apply"), buttonsBox);
    d->applyBtn->setIcon(SmallIcon("dialog-ok-apply"));
    d->applyBtn->setMaximumHeight( fontMetrics().height()+4 );
    d->applyBtn->setEnabled(false);
    d->applyBtn->setToolTip( i18n("Apply all changes to images"));
    buttonsBox->setStretchFactor(d->applyBtn, 10);

    d->moreButton = new QPushButton(i18n("More"), buttonsBox);
    d->moreMenu   = new QMenu(this);
    d->moreButton->setMaximumHeight( fontMetrics().height()+4 );
    d->moreButton->setMenu(d->moreMenu);

    // --------------------------------------------------

    settingsLayout->addWidget(commentsBox,    0, 0, 1, 2);
    settingsLayout->addWidget(dateBox,        1, 0, 1, 2);
    settingsLayout->addWidget(ratingBox,      2, 0, 1, 2);
    settingsLayout->addWidget(d->newTagEdit,  3, 0, 1, 2);
    settingsLayout->addWidget(d->tagsView,    4, 0, 1, 2);
    settingsLayout->addWidget(tagsSearch,     5, 0, 1, 2);
    settingsLayout->addWidget(buttonsBox,     6, 0, 1, 2);
    settingsLayout->setRowStretch(4, 10);
    settingsLayout->setMargin(KDialog::spacingHint());
    settingsLayout->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------

    connect(d->tagsView, SIGNAL(signalProgressBarMode(int, const QString&)),
            this, SIGNAL(signalProgressBarMode(int, const QString&)));

    connect(d->tagsView, SIGNAL(signalProgressValue(int)),
            this, SIGNAL(signalProgressValue(int)));

    connect(d->tagsView, SIGNAL(signalItemStateChanged(TAlbumCheckListItem *)),
            this, SLOT(slotItemStateChanged(TAlbumCheckListItem *)));

    connect(d->commentsEdit, SIGNAL(textChanged()),
            this, SLOT(slotCommentChanged()));

    connect(d->dateTimeEdit, SIGNAL(dateTimeChanged(const QDateTime& )),
            this, SLOT(slotDateTimeChanged(const QDateTime&)));

    connect(d->ratingWidget, SIGNAL(signalRatingChanged(int)),
            this, SLOT(slotRatingChanged(int)));

    connect(d->tagsView, SIGNAL(rightButtonClicked(Q3ListViewItem*, const QPoint &, int)),
            this, SLOT(slotRightButtonClicked(Q3ListViewItem*, const QPoint&, int)));

    connect(d->tagsSearchBar, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTagsSearchChanged(const QString&)));

    connect(this, SIGNAL(signalTagFilterMatch(bool)),
            d->tagsSearchBar, SLOT(slotSearchResult(bool)));

    connect(d->assignedTagsBtn, SIGNAL(toggled(bool)),
            this, SLOT(slotAssignedTagsToggled(bool)));

    connect(d->newTagEdit, SIGNAL(returnPressed(const QString&)),
            this, SLOT(slotCreateNewTag()));

    connect(d->applyBtn, SIGNAL(clicked()),
            this, SLOT(slotApplyAllChanges()));

    connect(d->revertBtn, SIGNAL(clicked()),
            this, SLOT(slotRevertAllChanges()));

    connect(d->moreMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotMoreMenu()));

    connect(d->recentTagsMapper, SIGNAL(mapped(int)),
            this, SLOT(slotRecentTagsMenuActivated(int)));

    // Initialize ---------------------------------------------

    d->commentsEdit->installEventFilter(this);
    d->dateTimeEdit->installEventFilter(this);
    d->ratingWidget->installEventFilter(this);
    d->tagsView->installEventFilter(this);
    updateRecentTags();

    // Connect to album manager -------------------------------

    AlbumManager* man = AlbumManager::instance();

    connect(man, SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(man, SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(man, SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    connect(man, SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(man, SIGNAL(signalAlbumIconChanged(Album*)),
            this, SLOT(slotAlbumIconChanged(Album*)));

    connect(man, SIGNAL(signalTAlbumMoved(TAlbum*, TAlbum*)),
            this, SLOT(slotAlbumMoved(TAlbum*, TAlbum*)));

    // Connect to thumbnail loader -----------------------------

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();

    connect(loader, SIGNAL(signalThumbnail(Album *, const QPixmap&)),
            this, SLOT(slotGotThumbnailFromIcon(Album *, const QPixmap&)));

    connect(loader, SIGNAL(signalFailed(Album *)),
            this, SLOT(slotThumbnailLost(Album *)));

    connect(loader, SIGNAL(signalReloadThumbnails()),
            this, SLOT(slotReloadThumbnails()));

    // Connect to attribute watch ------------------------------

    ImageAttributesWatch *watch = ImageAttributesWatch::instance();

    connect(watch, SIGNAL(signalImageTagsChanged(qlonglong)),
            this, SLOT(slotImageTagsChanged(qlonglong)));

    connect(watch, SIGNAL(signalImagesChanged(int)),
            this, SLOT(slotImagesChanged(int)));

    connect(watch, SIGNAL(signalImageRatingChanged(qlonglong)),
            this, SLOT(slotImageRatingChanged(qlonglong)));

    connect(watch, SIGNAL(signalImageDateChanged(qlonglong)),
            this, SLOT(slotImageDateChanged(qlonglong)));

    connect(watch, SIGNAL(signalImageCaptionChanged(qlonglong)),
            this, SLOT(slotImageCaptionChanged(qlonglong)));

    // -- read config ---------------------------------------------------------

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("Tag List View"));
    d->toggleAutoTags = (TagFilterView::ToggleAutoTags)(group.readEntry("Toggle Auto Tags", 
                                                       (int)TagFilterView::NoToggleAuto));
}

ImageDescEditTab::~ImageDescEditTab()
{
    slotChangingItems();

    /*
    AlbumList tList = AlbumManager::instance().allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        (*it)->removeExtraData(this);
    }
    */

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Tag List View"));
    group.writeEntry("Toggle Auto Tags", (int)(d->toggleAutoTags));
    group.sync();

    delete d;
}

bool ImageDescEditTab::singleSelection() const
{
    return (d->currInfos.count() == 1);
}

void ImageDescEditTab::slotChangingItems()
{
    if (!d->modified)
        return;

    if (d->currInfos.isEmpty())
        return;

    if (!AlbumSettings::instance()->getApplySidebarChangesDirectly())
    {
        KDialog *dialog = new KDialog(this);

        dialog->setCaption(i18n("Apply changes?"));
        dialog->setButtons(KDialog::Yes | KDialog::No);
        dialog->setDefaultButton(KDialog::Yes);
        dialog->setEscapeButton(KDialog::No);
        dialog->setButtonGuiItem(KDialog::Yes, KStandardGuiItem::yes());
        dialog->setButtonGuiItem(KDialog::No,  KStandardGuiItem::discard());
        dialog->setModal(true);

        int changedFields = 0;
        if (d->hub.commentChanged())
            changedFields++;
        if (d->hub.dateTimeChanged())
            changedFields++;
        if (d->hub.ratingChanged())
            changedFields++;
        if (d->hub.tagsChanged())
            changedFields++;

        QString text;
        if (changedFields == 1)
        {
            if (d->hub.commentChanged())
                text = i18np("<qt><p>You have edited the image caption. ",
                             "<qt><p>You have edited the captions of %1 images. ",
                             d->currInfos.count());
            else if (d->hub.dateTimeChanged())
                text = i18np("<qt><p>You have edited the date of the image. ",
                             "<qt><p>You have edited the date of %1 images. ",
                             d->currInfos.count());
            else if (d->hub.ratingChanged())
                text = i18np("<qt><p>You have edited the rating of the image. ",
                             "<qt><p>You have edited the rating of %1 images. ",
                             d->currInfos.count());
            else if (d->hub.tagsChanged())
                text = i18np("<qt><p>You have edited the tags of the image. ",
                             "<qt><p>You have edited the tags of %1 images. ",
                             d->currInfos.count());

            text += i18n("Do you want to apply your changes?</p></qt>");
        }
        else
        {
            text = i18np("<qt><p>You have edited the metadata of the image: </p><ul>",
                         "<qt><p>You have edited the metadata of %1 images: </p><ul>",
                         d->currInfos.count());

            if (d->hub.commentChanged())
                text += i18n("<li>the caption</li>");
            if (d->hub.dateTimeChanged())
                text += i18n("<li>the date</li>");
            if (d->hub.ratingChanged())
                text += i18n("<li>the rating</li>");
            if (d->hub.tagsChanged())
                text += i18n("<li>the tags</li>");

            text += "</ul><p>";

            text += i18n("Do you want to apply your changes?</p></qt>");
        }

        bool alwaysApply = false;
        int returnCode = KMessageBox::createKMessageBox
                         (dialog, QMessageBox::Information,
                          text, QStringList(),
                          i18n("Always apply changes without confirmation"),
                          &alwaysApply, KMessageBox::Notify);

        if (alwaysApply)
            AlbumSettings::instance()->setApplySidebarChangesDirectly(true);

        if (returnCode == KDialog::User1)
            return;
        // otherwise apply
    }

    slotApplyAllChanges();
}

void ImageDescEditTab::slotApplyAllChanges()
{
    if (!d->modified)
        return;

    if (d->currInfos.isEmpty())
        return;

    bool progressInfo = (d->currInfos.count() > 1);
    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode,
                               i18n("Applying changes to images. Please wait..."));
    MetadataWriteSettings writeSettings = MetadataHub::defaultWriteSettings();

    // debugging - use this to indicate reentry from event loop (kapp->processEvents)
    // remove before final release
    if (d->ignoreImageAttributesWatch)
    {
        DWarning() << "ImageDescEditTab::slotApplyAllChanges(): re-entering from event loop!" << endl;
    }

    // we are now changing attributes ourselves
    d->ignoreImageAttributesWatch = true;
    int i=0;
    {
        DatabaseTransaction transaction;
        foreach(ImageInfo info, d->currInfos)
        {
            // apply to database
            d->hub.write(info);
            // apply to file metadata
            d->hub.write(info.filePath(), MetadataHub::FullWrite, writeSettings);

            emit signalProgressValue((int)((i++/(float)d->currInfos.count())*100.0));
            if (progressInfo)
                kapp->processEvents();
        }
    }
    d->ignoreImageAttributesWatch = false;

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());

    d->modified = false;
    d->hub.resetChanged();
    d->applyBtn->setEnabled(false);
    d->revertBtn->setEnabled(false);

    updateRecentTags();
}

void ImageDescEditTab::slotRevertAllChanges()
{
    if (!d->modified)
        return;

    if (d->currInfos.isEmpty())
        return;

    setInfos(d->currInfos);
}

void ImageDescEditTab::setItem(const ImageInfo &info)
{
    slotChangingItems();
    ImageInfoList list;
    if (!info.isNull())
        list << info;
    setInfos(list);
}

void ImageDescEditTab::setItems(const ImageInfoList &infos)
{
    slotChangingItems();
    setInfos(infos);
}

void ImageDescEditTab::setInfos(const ImageInfoList &infos)
{
    if (infos.isEmpty())
    {
        d->hub = MetadataHub();
        d->commentsEdit->blockSignals(true);
        d->commentsEdit->clear();
        d->commentsEdit->blockSignals(false);
        d->currInfos.clear();
        setEnabled(false);
        return;
    }

    setEnabled(true);
    d->currInfos = infos;
    d->modified  = false;
    d->hub = MetadataHub();
    d->applyBtn->setEnabled(false);
    d->revertBtn->setEnabled(false);

    foreach(ImageInfo info, d->currInfos)
    {
        d->hub.load(info);
    }

    updateComments();
    updateRating();
    updateDate();
    updateTagsView();
}

void ImageDescEditTab::slotReadFromFileMetadataToDatabase()
{
    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode, 
                               i18n("Reading metadata from files. Please wait..."));

    d->ignoreImageAttributesWatch = true;
    int i=0;
    {
        DatabaseTransaction transaction;
        foreach(ImageInfo info, d->currInfos)
        {
            // A batch operation: a hub for each single file, not the common hub
            MetadataHub fileHub(MetadataHub::NewTagsImport);
            // read in from DMetadata
            fileHub.load(info.filePath());
            // write out to database
            fileHub.write(info);

            emit signalProgressValue((int)((i++/(float)d->currInfos.count())*100.0));
            kapp->processEvents();
        }
    }
    d->ignoreImageAttributesWatch = false;

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());

    // reload everything
    setInfos(d->currInfos);
}

void ImageDescEditTab::slotWriteToFileMetadataFromDatabase()
{
    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode, 
                               i18n("Writing metadata to files. Please wait..."));
    MetadataWriteSettings writeSettings = MetadataHub::defaultWriteSettings();

    int i=0;
    foreach(ImageInfo info, d->currInfos)
    {
        MetadataHub fileHub;
        // read in from database
        fileHub.load(info);
        // write out to file DMetadata
        fileHub.write(info.filePath());

        emit signalProgressValue((int)((i++/(float)d->currInfos.count())*100.0));
        kapp->processEvents();
    }

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
}

bool ImageDescEditTab::eventFilter(QObject *, QEvent *e)
{
    if ( e->type() == QEvent::KeyPress )
    {
        QKeyEvent *k = (QKeyEvent *)e;
        if (k->modifiers() == Qt::ControlModifier &&
            (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            emit signalNextItem();
            return true;
        }
        else if (k->modifiers() == Qt::ShiftModifier &&
                 (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            emit signalPrevItem();
            return true;
        }

        return false;
    }

    return false;
}

void ImageDescEditTab::populateTags()
{
    d->tagsView->clear();

    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbum *tag = (TAlbum*)(*it);
        slotAlbumAdded(tag);
    }
}

void ImageDescEditTab::slotItemStateChanged(TAlbumCheckListItem *item)
{
    TagFilterView::ToggleAutoTags oldAutoTags = d->toggleAutoTags;

    switch(d->toggleAutoTags)
    {
        case TagFilterView::Children:
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleChildTags(item->album(), item->isOn());
            d->toggleAutoTags = oldAutoTags;
            break;
        case TagFilterView::Parents:
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleParentTags(item->album(), item->isOn());
            d->toggleAutoTags = oldAutoTags;
            break;
        case TagFilterView::ChildrenAndParents:
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleChildTags(item->album(), item->isOn());
            toggleParentTags(item->album(), item->isOn());
            d->toggleAutoTags = oldAutoTags;
            break;
        default:
            break;
    }

    d->hub.setTag(item->album(), item->isOn());

    d->tagsView->blockSignals(true);
    item->setStatus(d->hub.tagStatus(item->album()));
    d->tagsView->blockSignals(false);

    slotModified();
}

void ImageDescEditTab::slotCommentChanged()
{
    // we cannot trust that the text actually changed
    // (there are bogus signals caused by spell checking, see bug 141663)
    // so we have to check before marking the metadata as modified
    if (d->hub.comment() == d->commentsEdit->toPlainText())
        return;

    d->hub.setComment(d->commentsEdit->toPlainText());
    setMetadataWidgetStatus(d->hub.commentStatus(), d->commentsEdit);
    slotModified();
}

void ImageDescEditTab::slotDateTimeChanged(const QDateTime& dateTime)
{
    d->hub.setDateTime(dateTime);
    setMetadataWidgetStatus(d->hub.dateTimeStatus(), d->dateTimeEdit);
    slotModified();
}

void ImageDescEditTab::slotRatingChanged(int rating)
{
    d->hub.setRating(rating);
    // no handling for MetadataDisjoint needed for rating,
    // we set it to 0 when disjoint, see below
    slotModified();
}

void ImageDescEditTab::slotModified()
{
    d->modified = true;
    d->applyBtn->setEnabled(true);
    d->revertBtn->setEnabled(true);
}

void ImageDescEditTab::assignRating(int rating)
{
    d->ratingWidget->setRating(rating);
}

void ImageDescEditTab::updateTagsView()
{
    d->tagsView->blockSignals(true);

    Q3ListViewItemIterator it( d->tagsView);
    while (it.current())
    {
        TAlbumCheckListItem* tItem = dynamic_cast<TAlbumCheckListItem*>(it.current());
        if (tItem)
            tItem->setStatus(d->hub.tagStatus(tItem->album()));
        ++it;
    }

    // The condition is a temporary fix not to destroy name filtering on image change.
    // See comments in these methods.
    if (d->assignedTagsBtn->isChecked())
        slotAssignedTagsToggled(d->assignedTagsBtn->isChecked());

    d->tagsView->blockSignals(false);
}

void ImageDescEditTab::updateComments()
{
    d->commentsEdit->blockSignals(true);
    d->commentsEdit->setPlainText(d->hub.comment());
    setMetadataWidgetStatus(d->hub.commentStatus(), d->commentsEdit);
    d->commentsEdit->blockSignals(false);
}

void ImageDescEditTab::updateRating()
{
    d->ratingWidget->blockSignals(true);
    if (d->hub.ratingStatus() == MetadataHub::MetadataDisjoint)
        d->ratingWidget->setRating(0);
    else
        d->ratingWidget->setRating(d->hub.rating());
    d->ratingWidget->blockSignals(false);
}

void ImageDescEditTab::updateDate()
{
    d->dateTimeEdit->blockSignals(true);
    d->dateTimeEdit->setDateTime(d->hub.dateTime());
    setMetadataWidgetStatus(d->hub.dateTimeStatus(), d->dateTimeEdit);
    d->dateTimeEdit->blockSignals(false);
}

void ImageDescEditTab::setMetadataWidgetStatus(int status, QWidget *widget)
{
    if (status == MetadataHub::MetadataDisjoint)
    {
        // For text widgets: Set text color to color of disabled text
        QPalette palette = widget->palette();
        palette.setColor(QColorGroup::Text, palette.color(QPalette::Disabled, QColorGroup::Text));
        widget->setPalette(palette);
    }
    else
    {
        widget->setPalette(QPalette());
    }
}

void ImageDescEditTab::slotRightButtonClicked(Q3ListViewItem *item, const QPoint &, int )
{
    TAlbum *album;

    if (!item)
    {
        album = AlbumManager::instance()->findTAlbum(0);
    }
    else
    {
        TAlbumCheckListItem* viewItem = dynamic_cast<TAlbumCheckListItem*>(item);

        if(!viewItem)
            album = AlbumManager::instance()->findTAlbum(0);
        else
            album = viewItem->album();
    }

    if(!album)
        return;

    KMenu popmenu(this);
    popmenu.addTitle(SmallIcon("digikam"), i18n("Tags"));
    QAction *newAction = popmenu.addAction(SmallIcon("tag-new"),  i18n("New Tag..."));

#ifdef KDEPIMLIBS_FOUND
    d->ABCMenu = new KMenu;

    connect(d->ABCMenu, SIGNAL( aboutToShow() ),
            this, SLOT( slotABCContextMenu() ));

    popmenu.addMenu(d->ABCMenu);
    d->ABCMenu->menuAction()->setIcon(SmallIcon("tag-addressbook"));
    d->ABCMenu->menuAction()->setText(i18n("Create Tag From AddressBook"));
#endif

    QAction *editAction=0, *resetIconAction=0, *deleteAction=0;
    if (!album->isRoot())
    {
        editAction      = popmenu.addAction(SmallIcon("tag-properties"), i18n("Edit Tag Properties..."));
        resetIconAction = popmenu.addAction(SmallIcon("tag-reset"),      i18n("Reset Tag Icon"));
        popmenu.addSeparator();
        deleteAction    = popmenu.addAction(SmallIcon("tag-delete"),     i18n("Delete Tag"));
    }

    popmenu.addSeparator();

    QMenu selectTagsMenu;
    QAction *selectAllTagsAction, *selectChildrenAction=0, *selectParentsAction=0;
    selectAllTagsAction = selectTagsMenu.addAction(i18n("All Tags"));
    if (item)
    {
        selectTagsMenu.addSeparator();
        selectChildrenAction = selectTagsMenu.addAction(i18n("Children"));
        selectParentsAction  = selectTagsMenu.addAction(i18n("Parents"));
    }
    popmenu.addMenu(&selectTagsMenu);
    selectTagsMenu.menuAction()->setText(i18n("Select"));

    QMenu deselectTagsMenu;
    QAction *deselectAllTagsAction, *deselectChildrenAction=0, *deselectParentsAction=0;
    deselectAllTagsAction = deselectTagsMenu.addAction(i18n("All Tags"));
    if (item)
    {
        deselectTagsMenu.addSeparator();
        deselectChildrenAction = deselectTagsMenu.addAction(i18n("Children"));
        deselectParentsAction  = deselectTagsMenu.addAction(i18n("Parents"));
    }
    popmenu.addMenu(&deselectTagsMenu);
    deselectTagsMenu.menuAction()->setText(i18n("Deselect"));

    QAction *invertAction;
    invertAction = popmenu.addAction(i18n("Invert Selection"));
    popmenu.addSeparator();


    KSelectAction *toggleAutoAction = new KSelectAction(i18n("Toggle Auto"), &popmenu);
    QAction *toggleNoneAction     = toggleAutoAction->addAction(i18n("None"));
    toggleAutoAction->menu()->addSeparator();
    QAction *toggleChildrenAction = toggleAutoAction->addAction(i18n("Children"));
    QAction *toggleParentsAction  = toggleAutoAction->addAction(i18n("Parents"));
    QAction *toggleBothAction     = toggleAutoAction->addAction(i18n("Both"));

    toggleNoneAction->setChecked(d->toggleAutoTags == TagFilterView::NoToggleAuto);
    toggleChildrenAction->setChecked(d->toggleAutoTags == TagFilterView::Children);
    toggleParentsAction->setChecked(d->toggleAutoTags == TagFilterView::Parents);
    toggleBothAction->setChecked(d->toggleAutoTags == TagFilterView::ChildrenAndParents);

    popmenu.addAction(toggleAutoAction);

    TagFilterView::ToggleAutoTags oldAutoTags = d->toggleAutoTags;

    QAction *choice = popmenu.exec((QCursor::pos()));

    if (choice)
    {
        if (choice == newAction)                    // New Tag.
        {
            tagNew(album);
        }
        else if (choice == editAction)              // Edit Tag Properties.
        {
            if (!album->isRoot())
                tagEdit(album);
        }
        else if (choice == deleteAction)            // Delete Tag.
        {
            if (!album->isRoot())
                tagDelete(album);
        }
        else if (choice == resetIconAction)         // Reset Tag Icon.
        {
            QString errMsg;
            AlbumManager::instance()->updateTAlbumIcon(album, QString("tag"), 0, errMsg);
        }
        else if (choice == selectAllTagsAction)     // Select All Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            Q3ListViewItemIterator it(d->tagsView, Q3ListViewItemIterator::NotChecked);
            while (it.current())
            {
                TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(it.current());
                if (item->isVisible())
                    item->setOn(true);
                ++it;
            }
            d->toggleAutoTags = oldAutoTags;
        }
        else if (choice == deselectAllTagsAction)    // Deselect All Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            Q3ListViewItemIterator it(d->tagsView, Q3ListViewItemIterator::Checked);
            while (it.current())
            {
                TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(it.current());
                if (item->isVisible())
                    item->setOn(false);
                ++it;
            }
            d->toggleAutoTags = oldAutoTags;
        }
        else if (choice == invertAction)             // Invert All Tags Selection.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            Q3ListViewItemIterator it(d->tagsView);
            while (it.current())
            {
                TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(it.current());
                if (item->isVisible())
                    item->setOn(!item->isOn());
                ++it;
            }
            d->toggleAutoTags = oldAutoTags;
        }
        else if (choice == selectChildrenAction)     // Select Child Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleChildTags(album, true);
            TAlbumCheckListItem *item = (TAlbumCheckListItem*)album->extraData(d->tagsView);
            item->setOn(true);
            d->toggleAutoTags = oldAutoTags;
        }
        else if (choice == deselectChildrenAction)   // Deselect Child Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleChildTags(album, false);
            TAlbumCheckListItem *item = (TAlbumCheckListItem*)album->extraData(d->tagsView);
            item->setOn(false);
            d->toggleAutoTags = oldAutoTags;
        }
        else if (choice == selectParentsAction)     // Select Parent Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleParentTags(album, true);
            TAlbumCheckListItem *item = (TAlbumCheckListItem*)album->extraData(d->tagsView);
            item->setOn(true);
            d->toggleAutoTags = oldAutoTags;
        }
        else if (choice == deselectParentsAction)   // Deselect Parent Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleParentTags(album, false);
            TAlbumCheckListItem *item = (TAlbumCheckListItem*)album->extraData(d->tagsView);
            item->setOn(false);
            d->toggleAutoTags = oldAutoTags;
        }
        else if (choice == toggleNoneAction)        // No toggle auto tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
        }
        else if (choice == toggleChildrenAction)    // Toggle auto Children tags.
        {
            d->toggleAutoTags = TagFilterView::Children;
        }
        else if (choice == toggleParentsAction)     // Toggle auto Parents tags.
        {
            d->toggleAutoTags = TagFilterView::Parents;
        }
        else if (choice == toggleBothAction)        // Toggle auto Children and Parents tags.
        {
            d->toggleAutoTags = TagFilterView::ChildrenAndParents;
        }
        else                                        // ABC menu
        {
            tagNew(album, choice->text(), "tag-people" );
        }
    }

    delete d->ABCMenu;
    d->ABCMenu = 0;
}

void ImageDescEditTab::slotABCContextMenu()
{
#ifdef KDEPIMLIBS_FOUND
    d->ABCMenu->clear();

    KABC::AddressBook* ab = KABC::StdAddressBook::self();
    QStringList names;
    for ( KABC::AddressBook::Iterator it = ab->begin(); it != ab->end(); ++it )
    {
        names.push_back(it->formattedName());
    }
    qSort(names);

    for ( QStringList::Iterator it = names.begin(); it != names.end(); ++it )
    {
        QString name = *it;
        if (!name.isNull() )
            d->ABCMenu->addAction(name);
    }

    if (d->ABCMenu->isEmpty())
    {
        QAction *nothingFound = d->ABCMenu->addAction(i18n("No AddressBook entries found"));
        nothingFound->setEnabled(false);
    }
#endif
}

void ImageDescEditTab::slotMoreMenu()
{
    d->moreMenu->clear();

    if (singleSelection())
    {
        d->moreMenu->addAction(i18n("Read metadata from file to database"), this, SLOT(slotReadFromFileMetadataToDatabase()));
        QAction *writeAction =
                d->moreMenu->addAction(i18n("Write metadata to each file"), this, SLOT(slotWriteToFileMetadataFromDatabase()));
        // we do not need a "Write to file" action here because the apply button will do just that
        // if selection is a single file.
        // Adding the option will confuse users: Does the apply button not write to file?
        // Removing the option will confuse users: There is not option to write to file! (not visible in single selection)
        // Disabling will confuse users: Why is it disabled?
        writeAction->setEnabled(false);
    }
    else
    {
        // We need to make clear that this action is different from the Apply button,
        // which saves the same changes to all files. These batch operations operate on each single file.
        d->moreMenu->addAction(i18n("Read metadata from each file to database"), this, SLOT(slotReadFromFileMetadataToDatabase()));
        d->moreMenu->addAction(i18n("Write metadata to each file"), this, SLOT(slotWriteToFileMetadataFromDatabase()));
    }
}

void ImageDescEditTab::tagNew(TAlbum* parAlbum, const QString& _title, const QString& _icon) const
{
    if (!parAlbum)
        return;

    QString title           = _title;
    QString icon            = _icon;

    if (title.isNull())
    {
        if (!TagEditDlg::tagCreate(kapp->activeWindow(), parAlbum, title, icon))
            return;
    }

    QMap<QString, QString> errMap;
    AlbumList tList = TagEditDlg::createTAlbum(parAlbum, title, icon, errMap);
    TagEditDlg::showtagsListCreationError(kapp->activeWindow(), errMap);

    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbumCheckListItem* item = (TAlbumCheckListItem*)(*it)->extraData(d->tagsView);
        if (item)
        {
            item->setOn(true);
            d->tagsView->setSelected(item, true);
            d->tagsView->ensureItemVisible(item);
        }
    }
}

void ImageDescEditTab::tagDelete(TAlbum *album)
{
    if (!album || album->isRoot())
        return;

    AlbumManager *albumMan = AlbumManager::instance();

    if (album == albumMan->currentAlbum() ||
        album->isAncestorOf(albumMan->currentAlbum()))
    {
        KMessageBox::error(this, i18n("You are currently viewing items in the "
                                      "tag '%1' that you are about to delete. "
                                      "You will need to apply change first "
                                      "if you want to delete the tag.", album->title()));
        return;
    }

    // find number of subtags
    int children = 0;
    AlbumIterator iter(album);
    while(iter.current())
    {
        children++;
        ++iter;
    }

    if(children)
    {
        int result = KMessageBox::warningContinueCancel(this,
                       i18np("Tag '%2' has one subtag. "
                             "Deleting this will also delete "
                             "the subtag. "
                             "Do you want to continue?",
                             "Tag '%2' has %1 subtags. "
                             "Deleting this will also delete "
                             "the subtags. "
                             "Do you want to continue?",
                             children, 
                             album->title()));

        if(result != KMessageBox::Continue)
            return;
    }

    QString message;
    QList<qlonglong> assignedItems = DatabaseAccess().db()->getItemIDsInTag(album->id());
    if (!assignedItems.isEmpty())
    {
        message = i18np("Tag '%2' is assigned to one item. "
                        "Do you want to continue?",
                        "Tag '%2' is assigned to %1 items. "
                        "Do you want to continue?",
                        assignedItems.count(), album->title());
    }
    else
    {
        message = i18n("Delete '%1' tag?", album->title());
    }

    int result = KMessageBox::warningContinueCancel(this, message,
                                                    i18n("Delete Tag"),
                                                    KGuiItem(i18n("Delete"), 
                                                    "edit-delete"));

    if (result == KMessageBox::Continue)
    {
        QString errMsg;
        if (!albumMan->deleteTAlbum(album, errMsg))
            KMessageBox::error(this, errMsg);
    }
}

void ImageDescEditTab::tagEdit(TAlbum* album)
{
    if (!album || album->isRoot())
        return;

    QString title;
    QString icon;

    if (!TagEditDlg::tagEdit(kapp->activeWindow(), album, title, icon))
        return;

    AlbumManager *albumMan = AlbumManager::instance();
    if (album->title() != title)
    {
        QString errMsg;
        if (!albumMan->renameTAlbum(album, title, errMsg))
        {
            KMessageBox::error(this, errMsg);
            return;
        }
    }

    if (album->icon() != icon)
    {
        QString errMsg;
        if (!albumMan->updateTAlbumIcon(album, icon, 0, errMsg))
        {
            KMessageBox::error(this, errMsg);
        }
    }
}

void ImageDescEditTab::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::TAG)
        return;

    TAlbumCheckListItem* viewItem = 0;

    TAlbum* tag = dynamic_cast<TAlbum*>(a);
    if (!tag)
        return;

    if (tag->isRoot())
    {
        viewItem = new TAlbumCheckListItem(d->tagsView, tag);
    }
    else
    {
        TAlbumCheckListItem* parent = (TAlbumCheckListItem*)(tag->parent()->extraData(d->tagsView));
        if (!parent)
        {
            DWarning() << "Failed to find parent for Tag " << tag->title()
                       << endl;
            return;
        }

        viewItem = new TAlbumCheckListItem(parent, tag);
        d->tagsSearchBar->completionObject()->addItem(tag->title());
        d->newTagEdit->completionObject()->addItem(tag->tagPath());
        d->newTagEdit->completionObject()->addItem(tag->tagPath().remove(0, 1)); // without root "/"
    }

    if (viewItem)
    {
        viewItem->setOpen(true);
        setTagThumbnail(tag);
    }
}

void ImageDescEditTab::slotAlbumDeleted(Album* a)
{
    if (!a || a->isRoot() || a->type() != Album::TAG)
        return;

    TAlbum* album = (TAlbum*)a;

    d->tagsSearchBar->completionObject()->removeItem(album->title());
    d->newTagEdit->completionObject()->removeItem(album->tagPath());
    d->newTagEdit->completionObject()->removeItem(album->tagPath().remove(0, 1)); // without root "/"
    TAlbumCheckListItem* viewItem = (TAlbumCheckListItem*)(album->extraData(d->tagsView));
    delete viewItem;
    album->removeExtraData(this);
    d->hub.setTag(album, false, MetadataHub::MetadataDisjoint);
}

void ImageDescEditTab::slotAlbumsCleared()
{
    d->tagsView->clear();
    d->tagsSearchBar->completionObject()->clear();
    d->newTagEdit->completionObject()->clear();
}

void ImageDescEditTab::slotAlbumIconChanged(Album* a)
{
    if (!a || a->isRoot() || a->type() != Album::TAG)
        return;

    setTagThumbnail((TAlbum *)a);
}

void ImageDescEditTab::slotAlbumMoved(TAlbum* tag, TAlbum* newParent)
{
    if (!tag || !newParent)
        return;

    TAlbumCheckListItem* item = (TAlbumCheckListItem*)tag->extraData(d->tagsView);
    if (!item)
        return;

    if (item->parent())
    {
        Q3ListViewItem* oldPItem = item->parent();
        oldPItem->takeItem(item);
    }
    else
    {
        d->tagsView->takeItem(item);
    }

    TAlbumCheckListItem* newPItem = (TAlbumCheckListItem*)newParent->extraData(d->tagsView);
    if (newPItem)
        newPItem->insertItem(item);
    else
        d->tagsView->insertItem(item);
}

void ImageDescEditTab::slotAlbumRenamed(Album* a)
{
    if (!a || a->isRoot() || a->type() != Album::TAG)
        return;

    TAlbum* album = (TAlbum*)a;
    d->tagsSearchBar->completionObject()->addItem(album->title());
    d->newTagEdit->completionObject()->addItem(album->tagPath());
    d->newTagEdit->completionObject()->addItem(album->tagPath().remove(0, 1)); // without root "/"
    slotTagsSearchChanged(d->tagsSearchBar->text());
    TAlbumCheckListItem* viewItem = (TAlbumCheckListItem*)(album->extraData(d->tagsView));
    if (!viewItem)
    {
        DWarning() << "Failed to find view item for Tag "
                   << album->title() << endl;
        return;
    }

    viewItem->setText(0, album->title());
}

void ImageDescEditTab::toggleChildTags(TAlbum *album, bool b)
{
    if (!album)
        return;

    AlbumIterator it(album);
    while ( it.current() )
    {
        TAlbum *ta                = (TAlbum*)it.current();
        TAlbumCheckListItem *item = (TAlbumCheckListItem*)ta->extraData(d->tagsView);
        if (item)
            if (item->isVisible())
                item->setOn(b);
        ++it;
    }
}

void ImageDescEditTab::toggleParentTags(TAlbum *album, bool b)
{
    if (!album)
        return;

    Q3ListViewItemIterator it(d->tagsView);
    while (it.current())
    {
        TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(it.current());
        if (item->isVisible())
        {
            if (!item->album())
                continue;
            if (item->album() == album->parent())
            {
                item->setOn(b);
                toggleParentTags(item->album() , b);
            }
        }
        ++it;
    }
}

void ImageDescEditTab::setTagThumbnail(TAlbum *album)
{
    if(!album)
        return;

    TAlbumCheckListItem* item = (TAlbumCheckListItem*) album->extraData(d->tagsView);
    if(!item)
        return;

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();
    QPixmap icon;
    if (!loader->getTagThumbnail(album, icon))
    {
        if (icon.isNull())
        {
            item->setPixmap(0, loader->getStandardTagIcon(album));
        }
        else
        {
            QPixmap blendedIcon = loader->blendIcons(loader->getStandardTagIcon(), icon);
            item->setPixmap(0, blendedIcon);
        }
    }
}

void ImageDescEditTab::slotGotThumbnailFromIcon(Album *album, const QPixmap& thumbnail)
{
    if(!album || album->type() != Album::TAG)
        return;

    // update item in tags tree
    TAlbumCheckListItem* item = (TAlbumCheckListItem*)album->extraData(d->tagsView);
    if(!item)
        return;

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();
    QPixmap blendedIcon          = loader->blendIcons(loader->getStandardTagIcon(), thumbnail);
    item->setPixmap(0, blendedIcon);

    // update item in recent tags popup menu, if found there in
    QMenu *menu = dynamic_cast<QMenu *>(d->recentTagsBtn->menu());
    if (menu)
    {
        QAction *action = qobject_cast<QAction *>(d->recentTagsMapper->mapping(album->id()));
        if (action)
            action->setIcon(thumbnail);
    }
}

void ImageDescEditTab::slotThumbnailLost(Album *)
{
    // we already set the standard icon before loading
}

void ImageDescEditTab::slotReloadThumbnails()
{
    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbum* tag  = (TAlbum*)(*it);
        setTagThumbnail(tag);
    }
}

void ImageDescEditTab::slotImageTagsChanged(qlonglong imageId)
{
    // don't lose modifications
    if (d->ignoreImageAttributesWatch || d->modified)
        return;

    reloadForMetadataChange(imageId);
}

void ImageDescEditTab::slotImagesChanged(int albumId)
{
    if (d->ignoreImageAttributesWatch || d->modified)
        return;

    Album *a = AlbumManager::instance()->findAlbum(albumId);
    if (d->currInfos.isEmpty() || !a || a->isRoot() || a->type() != Album::TAG)
        return;

    setInfos(d->currInfos);
}

void ImageDescEditTab::slotImageRatingChanged(qlonglong imageId)
{
    if (d->ignoreImageAttributesWatch || d->modified)
        return;

    reloadForMetadataChange(imageId);
}

void ImageDescEditTab::slotImageCaptionChanged(qlonglong imageId)
{
    if (d->ignoreImageAttributesWatch || d->modified)
        return;

    reloadForMetadataChange(imageId);
}

void ImageDescEditTab::slotImageDateChanged(qlonglong imageId)
{
    if (d->ignoreImageAttributesWatch || d->modified)
        return;

    reloadForMetadataChange(imageId);
}

// private common code for above methods
void ImageDescEditTab::reloadForMetadataChange(qlonglong imageId)
{
    if (d->currInfos.isEmpty())
        return;

    if (singleSelection())
    {
        if (d->currInfos.first().id() == imageId)
            setInfos(d->currInfos);
    }
    else
    {
        // if image id is in our list, update
        foreach(ImageInfo info, d->currInfos)
        {
            if (info.id() == imageId)
            {
                setInfos(d->currInfos);
                return;
            }
        }
    }
}

void ImageDescEditTab::updateRecentTags()
{
    QMenu *menu = dynamic_cast<QMenu *>(d->recentTagsBtn->menu());
    if (!menu) return;

    menu->clear();

    AlbumList recentTags = AlbumManager::instance()->getRecentlyAssignedTags();

    if (recentTags.isEmpty())
    {
        QAction *noTagsAction = menu->addAction(i18n("No Recently Assigned Tags"));
        noTagsAction->setEnabled(false);
    }
    else
    {
        for (AlbumList::const_iterator it = recentTags.begin();
             it != recentTags.end(); ++it)
        {
            TAlbum* album = static_cast<TAlbum*>(*it);
            if (album)
            {
                AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();
                QPixmap icon;
                if (!loader->getTagThumbnail(album, icon))
                {
                    if (icon.isNull())
                    {
                        icon = loader->getStandardTagIcon(album, AlbumThumbnailLoader::SmallerSize);
                    }
                }
                QString text = album->title() + " (" + ((TAlbum*)album->parent())->prettyUrl() + ')';
                QAction *action = menu->addAction(icon, text, d->recentTagsMapper, SLOT(map()));
                d->recentTagsMapper->setMapping(action, album->id());
            }
        }
    }
}

void ImageDescEditTab::slotRecentTagsMenuActivated(int id)
{
    AlbumManager* albumMan = AlbumManager::instance();

    if (id > 0)
    {
        TAlbum* album = albumMan->findTAlbum(id);
        if (album)
        {
            TAlbumCheckListItem* viewItem = (TAlbumCheckListItem*)album->extraData(d->tagsView);
            if (viewItem)
            {
                viewItem->setOn(true);
                d->tagsView->setSelected(viewItem, true);
                d->tagsView->ensureItemVisible(viewItem);
            }
        }
    }
}

void ImageDescEditTab::slotTagsSearchChanged(const QString& filter)
{
    //TODO: this will destroy assigned-tags filtering. Unify in one method.
    QString search = filter.toLower();

    bool atleastOneMatch = false;

    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbum* tag  = (TAlbum*)(*it);

        // don't touch the root Tag
        if (tag->isRoot())
            continue;

        bool match = tag->title().toLower().contains(search);
        if (!match)
        {
            // check if any of the parents match the search
            Album* parent = tag->parent();
            while (parent && !parent->isRoot())
            {
                if (parent->title().toLower().contains(search))
                {
                    match = true;
                    break;
                }

                parent = parent->parent();
            }
        }

        if (!match)
        {
            // check if any of the children match the search
            AlbumIterator it(tag);
            while (it.current())
            {
                if ((*it)->title().toLower().contains(search))
                {
                    match = true;
                    break;
                }
                ++it;
            }
        }

        TAlbumCheckListItem* viewItem = (TAlbumCheckListItem*)(tag->extraData(d->tagsView));

        if (match)
        {
            atleastOneMatch = true;

            if (viewItem)
                viewItem->setVisible(true);
        }
        else
        {
            if (viewItem)
            {
                viewItem->setVisible(false);
            }
        }
    }

    if (search.isEmpty())
    {
        TAlbum* root = AlbumManager::instance()->findTAlbum(0);
        TAlbumCheckListItem* rootItem = (TAlbumCheckListItem*)(root->extraData(d->tagsView));
        if (rootItem)
            rootItem->setText(0, root->title());
    }
    else
    {
        TAlbum* root = AlbumManager::instance()->findTAlbum(0);
        TAlbumCheckListItem* rootItem = (TAlbumCheckListItem*)(root->extraData(d->tagsView));
        if (rootItem)
            rootItem->setText(0, i18n("Found Tags"));
    }

    emit signalTagFilterMatch(atleastOneMatch);
}

void ImageDescEditTab::slotAssignedTagsToggled(bool t)
{
    //TODO: this will destroy name filtering. Unify in one method.
    Q3ListViewItemIterator it(d->tagsView);
    while (it.current())
    {
        TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(it.current());
        TAlbum *tag               = item->album();
        if (tag)
        {
            if (!tag->isRoot())
            {
                if (t)
                {
                    MetadataHub::TagStatus status = d->hub.tagStatus(item->album());
                    bool tagAssigned = (status == MetadataHub::MetadataAvailable && status.hasTag)
                                        || status == MetadataHub::MetadataDisjoint;
                    item->setVisible(tagAssigned);

                    if (tagAssigned)
                    {
                        Album* parent = tag->parent();
                        while (parent && !parent->isRoot())
                        {
                            TAlbumCheckListItem *pitem = (TAlbumCheckListItem*)parent->extraData(d->tagsView);
                            pitem->setVisible(true);
                            parent = parent->parent();
                        }
                    }
                }
                else
                {
                    item->setVisible(true);
                }
            }
        }
        ++it;
    }

    // correct visibilities afterwards:
    // As QListViewItem::setVisible works recursively on all it's children
    // we have to correct this
    if (t)
    {
        it = d->tagsView;
        while (it.current())
        {
            TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(it.current());
            TAlbum *tag               = item->album();
            if (tag)
            {
                if (!tag->isRoot())
                {
                    // only if the current item is not marked as tagged, check all children 
                    MetadataHub::TagStatus status = d->hub.tagStatus(item->album());
                    bool tagAssigned = (status == MetadataHub::MetadataAvailable && status.hasTag)
                                        || status == MetadataHub::MetadataDisjoint;
                    if (!tagAssigned)
                    {
                        bool somethingIsSet         = false;
                        Q3ListViewItem* nextSibling  = (*it)->nextSibling();
                        Q3ListViewItemIterator tmpIt = it;
                        ++tmpIt;
                        while (*tmpIt != nextSibling )
                        {
                            TAlbumCheckListItem* tmpItem = dynamic_cast<TAlbumCheckListItem*>(tmpIt.current());
                            MetadataHub::TagStatus tmpStatus = d->hub.tagStatus(tmpItem->album());
                            bool tmpTagAssigned = (tmpStatus == MetadataHub::MetadataAvailable && tmpStatus.hasTag)
                                                || tmpStatus == MetadataHub::MetadataDisjoint;
                            if(tmpTagAssigned)
                            {
                                somethingIsSet = true;
                            }
                            ++tmpIt;
                        }
                        if (!somethingIsSet) 
                        {
                            item->setVisible(false);
                        }
                    }
                }
            }
            ++it;
        }
    }

    TAlbum *root                  = AlbumManager::instance()->findTAlbum(0);
    TAlbumCheckListItem *rootItem = (TAlbumCheckListItem*)(root->extraData(d->tagsView));
    if (rootItem)
    {
        if (t)
            rootItem->setText(0, i18n("Assigned Tags"));
        else
            rootItem->setText(0, root->title());
    }
}

void ImageDescEditTab::refreshTagsView()
{
    d->tagsView->refresh();
}

void ImageDescEditTab::slotCreateNewTag()
{
    QString tagStr = d->newTagEdit->text();
    if (tagStr.isEmpty()) return;

    TAlbum *mainRootAlbum     = 0;
    TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(d->tagsView->selectedItem());
    if (item) 
        mainRootAlbum = item->album();

    QMap<QString, QString> errMap;
    AlbumList tList = TagEditDlg::createTAlbum(mainRootAlbum, tagStr, QString("tag"), errMap);

    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbumCheckListItem* item = (TAlbumCheckListItem*)(*it)->extraData(d->tagsView);
        if (item)
        {
            item->setOn(true);
            d->tagsView->ensureItemVisible(item);
        }
    }

    d->newTagEdit->clear();
}

}  // NameSpace Digikam
