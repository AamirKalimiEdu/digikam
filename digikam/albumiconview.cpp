/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : album icon view 
 * 
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2002-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

// C Ansi includes.

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

// C++ includes.

#include <cstdio>

// Qt includes.

#include <Q3DragObject>
#include <Q3ValueVector>
#include <Q3IntDict>
#include <Q3Dict>
#include <QImage>
#include <QString>
#include <QStringList>
#include <QEvent>
#include <QPainter>
#include <QPoint>
#include <QDateTime>
#include <QFileInfo>
#include <QFile>
#include <QCursor>
#include <QDataStream>
#include <QTimer>
#include <QClipboard>
#include <QPixmap>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QResizeEvent>

// KDE includes.

#include <k3urldrag.h>
#include <kurl.h>
#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kpropsdlg.h>
#include <ktrader.h>
#include <kservice.h>
#include <krun.h>
#include <kaction.h>
#include <kstandarddirs.h>
#include <kiconeffect.h>
#include <kdeversion.h>
#include <kcalendarsystem.h>
#include <kinputdialog.h>
#include <kio/jobuidelegate.h>

// LibKipi includes.

#include <libkipi/pluginloader.h>
#include <libkipi/plugin.h>

// LibKDcraw includes.

#include <libkdcraw/dcrawbinary.h>

// Local includes.

#include "constants.h"
#include "ddebug.h"
#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "dio.h"
#include "albumlister.h"
#include "albumfiletip.h"
#include "albumsettings.h"
#include "databasetransaction.h"
#include "databaseaccess.h"
#include "imagewindow.h"
#include "thumbnailsize.h"
#include "themeengine.h"
#include "dpopupmenu.h"
#include "tagspopupmenu.h"
#include "ratingpopupmenu.h"
#include "pixmapmanager.h"
#include "cameradragobject.h"
#include "dragobjects.h"
#include "dmetadata.h"
#include "albumdb.h"
#include "imageattributeswatch.h"
#include "deletedialog.h"
#include "albumiconitem.h"
#include "albumicongroupitem.h"
#include "loadingcacheinterface.h"
#include "lighttablewindow.h"
#include "statusprogressbar.h"
#include "metadatahub.h"
#include "albumiconview.h"
#include "albumiconview.moc"

namespace Digikam
{

class AlbumIconViewPrivate
{
public:

    void init()
    {
        imageLister   = 0;
        currentAlbum  = 0;
        albumSettings = 0;
        pixMan        = 0;
        toolTip       = 0;
    }

    QString                          albumTitle;
    QString                          albumDate;
    QString                          albumComments;

    QRect                            itemRect;
    QRect                            itemRatingRect;
    QRect                            itemDateRect;
    QRect                            itemModDateRect;
    QRect                            itemPixmapRect;
    QRect                            itemNameRect;
    QRect                            itemCommentsRect;
    QRect                            itemResolutionRect;
    QRect                            itemSizeRect;
    QRect                            itemTagRect;
    QRect                            bannerRect;

    QPixmap                          itemRegPixmap;
    QPixmap                          itemSelPixmap;
    QPixmap                          bannerPixmap;
    QPixmap                          ratingPixmap;

    QFont                            fnReg;
    QFont                            fnCom;
    QFont                            fnXtra;

    Q3Dict<AlbumIconItem>            itemDict;
    QHash<ImageInfo, AlbumIconItem*> itemInfoMap;

    AlbumLister                     *imageLister;
    Album                           *currentAlbum;
    const AlbumSettings             *albumSettings;
    Q3IntDict<AlbumIconGroupItem>    albumDict;
    PixmapManager                   *pixMan;

    ThumbnailSize                    thumbSize;
    
    AlbumFileTip                    *toolTip;
};

AlbumIconView::AlbumIconView(QWidget* parent)
             : IconView(parent)
{
    d = new AlbumIconViewPrivate;
    d->init();
    d->imageLister = AlbumLister::componentData();
    d->pixMan      = new PixmapManager(this);
    d->toolTip     = new AlbumFileTip(this);

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    // -- Load rating Pixmap ------------------------------------------

    d->ratingPixmap = QPixmap(KStandardDirs::locate("data", "digikam/data/rating.png"));

    QPainter painter(&d->ratingPixmap);
    painter.fillRect(0, 0, d->ratingPixmap.width(), d->ratingPixmap.height(),
                     ThemeEngine::componentData()->textSpecialRegColor());
    painter.end();
    
    // -- ImageLister connections -------------------------------------

    connect(d->imageLister, SIGNAL(signalNewFilteredItems(const ImageInfoList&)),
            this, SLOT(slotImageListerNewItems(const ImageInfoList&)));

    connect(d->imageLister, SIGNAL(signalDeleteFilteredItem(const ImageInfo &)),
            this, SLOT(slotImageListerDeleteItem(const ImageInfo &)) );

    connect(d->imageLister, SIGNAL(signalClear()),
            this, SLOT(slotImageListerClear()));

    // -- Icon connections --------------------------------------------

    connect(this, SIGNAL(signalDoubleClicked(IconItem*)),
            this, SLOT(slotDoubleClicked(IconItem*)));

    connect(this, SIGNAL(signalReturnPressed(IconItem*)),
            this, SLOT(slotDoubleClicked(IconItem*)));

    connect(this, SIGNAL(signalRightButtonClicked(IconItem*, const QPoint &)),
            this, SLOT(slotRightButtonClicked(IconItem*, const QPoint &)));

    connect(this, SIGNAL(signalRightButtonClicked(const QPoint &)),
            this, SLOT(slotRightButtonClicked(const QPoint &)));

    connect(this, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(this, SIGNAL(signalShowToolTip(IconItem*)),
            this, SLOT(slotShowToolTip(IconItem*)));

    // -- ThemeEngine connections ---------------------------------------

    connect(ThemeEngine::componentData(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // -- Pixmap manager connections ------------------------------------

    connect(d->pixMan, SIGNAL(signalPixmap(const KUrl&)),
            this, SLOT(slotGotThumbnail(const KUrl&)));

    // -- ImageAttributesWatch connections ------------------------------

    ImageAttributesWatch *watch = ImageAttributesWatch::componentData();

    connect(watch, SIGNAL(signalImageTagsChanged(qlonglong)),
            this, SLOT(slotImageAttributesChanged(qlonglong)));

    connect(watch, SIGNAL(signalImagesChanged(int)),
            this, SLOT(slotAlbumImagesChanged(int)));

    connect(watch, SIGNAL(signalImageRatingChanged(qlonglong)),
            this, SLOT(slotImageAttributesChanged(qlonglong)));

    connect(watch, SIGNAL(signalImageDateChanged(qlonglong)),
            this, SLOT(slotImageAttributesChanged(qlonglong)));

    connect(watch, SIGNAL(signalImageCaptionChanged(qlonglong)),
            this, SLOT(slotImageAttributesChanged(qlonglong)));
}

AlbumIconView::~AlbumIconView()
{
    delete d->pixMan;
    delete d->toolTip;
    delete d;
}

void AlbumIconView::applySettings(const AlbumSettings* settings)
{
    if (!settings)
        return;

    d->albumSettings = settings;

    d->imageLister->setNameFilter(d->albumSettings->getAllFileFilter());

    d->thumbSize = (ThumbnailSize::Size)d->albumSettings->getDefaultIconSize();

    setEnableToolTips(d->albumSettings->getShowToolTips());

    updateBannerRectPixmap();
    updateItemRectsPixmap();

    d->imageLister->stop();
    clear();

    d->pixMan->setThumbnailSize(d->thumbSize.size());

    if (d->currentAlbum)
    {
        d->imageLister->openAlbum(d->currentAlbum);
    }
}

void AlbumIconView::setThumbnailSize(const ThumbnailSize& thumbSize)
{
    if ( d->thumbSize != thumbSize)
    {
        d->imageLister->stop();
        clear();

        d->thumbSize = thumbSize;
        d->pixMan->setThumbnailSize(d->thumbSize.size());

        updateBannerRectPixmap();
        updateItemRectsPixmap();

        d->imageLister->openAlbum(d->currentAlbum);
    }
}

void AlbumIconView::setAlbum(Album* album)
{
    if (!album)
    {
        d->currentAlbum = 0;
        d->imageLister->stop();
        clear();

        return;
    }

    if (d->currentAlbum == album)
        return;

    d->imageLister->stop();
    clear();

    d->currentAlbum = album;
    d->imageLister->openAlbum(d->currentAlbum);

    updateBannerRectPixmap();
    updateItemRectsPixmap();
}

void AlbumIconView::refreshIcon(AlbumIconItem* item)
{
    if (!item)
        return;

    emit signalSelectionChanged();
}

void AlbumIconView::clear(bool update)
{
    emit signalCleared();

    d->pixMan->clear();
    d->itemDict.clear();
    d->albumDict.clear();
    d->itemInfoMap.clear();

    IconView::clear(update);

    emit signalSelectionChanged();
}

void AlbumIconView::slotImageListerNewItems(const ImageInfoList& itemList)
{
    if (!d->currentAlbum || d->currentAlbum->isRoot())
        return;

    for (ImageInfoList::const_iterator it = itemList.begin(); it != itemList.end(); ++it)
    {
        KUrl url( it->fileUrl() );
        url.cleanPath();

        if (d->itemInfoMap.contains(*it))
        {
            // TODO: Make sure replacing slotImageListerDeleteItem with continue here is not wrong
            //slotImageListerDeleteItem((*itMap)->imageInfo());
            continue;
        }

        AlbumIconGroupItem* group = d->albumDict.find(it->albumId());
        if (!group)
        {
            group = new AlbumIconGroupItem(this, it->albumId());
            d->albumDict.insert(it->albumId(), group);
        }

        PAlbum *album = AlbumManager::componentData()->findPAlbum(it->albumId());
        if (!album)
        {
            DWarning() << "No album for item: " << it->name()
                       << ", albumID: " << it->albumId() << endl;
            continue;
        }

        AlbumIconItem* iconItem = new AlbumIconItem(group, (*it));

        d->itemDict.insert(url.url(), iconItem);
        d->itemInfoMap.insert((*it), iconItem);
    }

    emit signalItemsAdded();
}

void AlbumIconView::slotImageListerDeleteItem(const ImageInfo &item)
{
    QHash<ImageInfo, AlbumIconItem*>::iterator itMap = d->itemInfoMap.find(item);
    if (itMap == d->itemInfoMap.end())
        return;

    AlbumIconItem* iconItem = (*itMap);

    /*
    // ?? Necessary? For what situation?
    KUrl url(item->kurl());
    url.cleanPath();

    AlbumIconItem *oldItem = d->itemDict[url.url()];

    if( oldItem &&
       (oldItem->imageInfo()->id() != iconItem->imageInfo()->id()))
    {
        return;
    }
    */

    //d->pixMan->remove(item->kurl());

    emit signalItemDeleted(iconItem);

    delete iconItem;

    d->itemInfoMap.remove(item);
    d->itemDict.remove(item.fileUrl().url());

    IconGroupItem* group = firstGroup();
    IconGroupItem* tmp;
    
    while (group)
    {
        tmp = group->nextGroup();
        
        if (group->count() == 0)
        {
            d->albumDict.remove(((AlbumIconGroupItem*)group)->albumID());
            delete group;
        }
        
        group = tmp;
    }
}

void AlbumIconView::slotImageListerClear()
{
    clear();
}

void AlbumIconView::slotDoubleClicked(IconItem *item)
{
    if (!item) return;

    if (d->albumSettings->getItemRightClickAction() == AlbumSettings::ShowPreview)
    {
        // icon effect takes too much time
        //KIconEffect::visualActivate(viewport(), contentsRectToViewport(item->rect()));
        signalPreviewItem(static_cast<AlbumIconItem *>(item));
    }
    else
    {
        KIconEffect::visualActivate(viewport(), contentsRectToViewport(item->rect()));
        slotDisplayItem(static_cast<AlbumIconItem *>(item));
    }
}

void AlbumIconView::slotRightButtonClicked(const QPoint& pos)
{
    if (!d->currentAlbum)
        return;

    if (d->currentAlbum->isRoot() ||
         (   d->currentAlbum->type() != Album::PHYSICAL
          && d->currentAlbum->type() != Album::TAG))
    {
        return;
    }

    Q3PopupMenu popmenu(this);
    KAction *paste    = KStandardAction::paste(this, SLOT(slotPaste()), 0);
    QMimeSource *data = kapp->clipboard()->data(QClipboard::Clipboard);

    if(!data || !Q3UriDrag::canDecode(data))
    {
        paste->setEnabled(false);
    }

    popmenu.addAction(paste);
    popmenu.exec(pos);
    delete paste;
}

void AlbumIconView::slotRightButtonClicked(IconItem *item, const QPoint& pos)
{
    if (!item)
        return;
    
    AlbumIconItem* iconItem = static_cast<AlbumIconItem *>(item);

    //-- Open With Actions ------------------------------------

    KMimeType::Ptr mimePtr = KMimeType::findByUrl(iconItem->imageInfo().fileUrl(), 0, true, true);

    Q3ValueVector<KService::Ptr> serviceVector;

    const KService::List offers = KServiceTypeTrader::self()->query(mimePtr->name(), "Type == 'Application'");
    KService::List::ConstIterator iter;
    KService::Ptr ptr;

    Q3PopupMenu openWithMenu;

    int index = 100;

    for( iter = offers.begin(); iter != offers.end(); ++iter )
    {
        ptr = *iter;
        openWithMenu.insertItem(SmallIcon(ptr->icon()), ptr->name(), index++);
        serviceVector.push_back(ptr); 
    }

    // --------------------------------------------------------

    DPopupMenu popmenu(this);
    popmenu.insertItem(SmallIcon("viewimage"), i18n("View..."), 18);
    popmenu.insertItem(SmallIcon("editimage"), i18n("Edit..."), 10);
    popmenu.insertItem(SmallIcon("lighttable"), i18n("Add to Light Table"), 19);
    popmenu.insertItem(i18n("Open With"), &openWithMenu, 11);

    // Merge in the KIPI plugins actions ----------------------------

    KIPI::PluginLoader* kipiPluginLoader      = KIPI::PluginLoader::componentData();
    KIPI::PluginLoader::PluginList pluginList = kipiPluginLoader->pluginList();
    
    for (KIPI::PluginLoader::PluginList::const_iterator it = pluginList.begin();
         it != pluginList.end(); ++it)
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if (plugin && (*it)->name() == "JPEGLossless")
        {
            DDebug() << "Found JPEGLossless plugin" << endl;

            QList<KAction*> actionList = plugin->actions();
            
            for (QList<KAction*>::const_iterator iter = actionList.begin();
                iter != actionList.end(); ++iter)
            {
                KAction* action = *iter;
                
                if (action->objectName().toLatin1() == QString::fromLatin1("jpeglossless_rotate"))
                {
                    popmenu.addAction(action);
                }
            }
        }
    }

    // --------------------------------------------------------

    popmenu.insertItem(SmallIcon("pencil"), i18n("Rename..."), 15);
    popmenu.insertSeparator();

    // --------------------------------------------------------

    if (d->currentAlbum)
    {
        if (d->currentAlbum->type() == Album::PHYSICAL )
            popmenu.insertItem(i18n("Set as Album Thumbnail"), 17);
        else if (d->currentAlbum->type() == Album::TAG )
            popmenu.insertItem(i18n("Set as Tag Thumbnail"), 17);
    }

    popmenu.insertSeparator();

    // --------------------------------------------------------
    
    KAction *copy     = KStandardAction::copy(this, SLOT(slotCopy()), 0);
    KAction *paste    = KStandardAction::paste(this, SLOT(slotPaste()), 0);
    QMimeSource *data = kapp->clipboard()->data(QClipboard::Clipboard);

    if(!data || !Q3UriDrag::canDecode(data))
    {
        paste->setEnabled(false);
    }    

    popmenu.addAction(copy);
    popmenu.addAction(paste);    
    popmenu.insertSeparator();

    // --------------------------------------------------------

    QList<qlonglong> selectedImageIDs;

    for (IconItem *it = firstItem(); it; it=it->nextItem())
    {
        if (it->isSelected())
        {
            AlbumIconItem *selItem = static_cast<AlbumIconItem *>(it);
            selectedImageIDs.append(selItem->imageInfo().id());
        }
    }

    popmenu.insertItem(SmallIcon("edittrash"),
                       i18np("Move to Trash", "Move %n Files to Trash" , selectedImageIDs.count() ), 16);

    popmenu.insertSeparator();

    // Bulk assignment/removal of tags --------------------------

    TagsPopupMenu* assignTagsPopup = new TagsPopupMenu(selectedImageIDs, TagsPopupMenu::ASSIGN);
    TagsPopupMenu* removeTagsPopup = new TagsPopupMenu(selectedImageIDs, TagsPopupMenu::REMOVE);

    connect(assignTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotAssignTag(int)));

    connect(removeTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotRemoveTag(int)));

    popmenu.insertItem(i18n("Assign Tag"), assignTagsPopup);

    int removeTagId = popmenu.insertItem(i18n("Remove Tag"), removeTagsPopup);

    // Performance: Only check for tags if there are <250 images selected
    if (selectedImageIDs.count() > 250 ||
        !DatabaseAccess().db()->hasTags(selectedImageIDs))
        popmenu.setItemEnabled(removeTagId, false);

    popmenu.insertSeparator();

    // Assign Star Rating -------------------------------------------

    RatingPopupMenu ratingMenu;
    
    connect(&ratingMenu, SIGNAL(activated(int)),
            this, SLOT(slotAssignRating(int)));

    popmenu.insertItem(i18n("Assign Rating"), &ratingMenu);

    // --------------------------------------------------------
        
    int id = popmenu.exec(pos);

    switch(id) 
    {
      case 10: 
      {
          slotDisplayItem(iconItem);
          break;
      }
  
      case 15: 
      {
          slotRename(iconItem);
          break;
      }
  
      case 16: 
      {
          slotDeleteSelectedItems();
          break;
      }
  
      case 17: 
      {
          slotSetAlbumThumbnail(iconItem);
          break;
      }

      case 18: 
      {
          signalPreviewItem(iconItem);
          break;
      }
  
      case 19: 
      {
          insertSelectionToLightTable();
          break;
      }

      default:
          break;
    }

    //---------------------------------------------------------------

    if (id >= 100 && id < 1000) 
    {
        KService::Ptr imageServicePtr = serviceVector[id-100];
        KUrl::List urlList;
        for (IconItem *it = firstItem(); it; it=it->nextItem())
        {
            if (it->isSelected())
            {
                AlbumIconItem *selItem = static_cast<AlbumIconItem *>(it);
                urlList.append(selItem->imageInfo().fileUrl());
            }
        }
        if (urlList.count())
            KRun::run(*imageServicePtr, urlList, this);
    }

    serviceVector.clear();
    delete assignTagsPopup;
    delete removeTagsPopup;
    delete copy;
    delete paste;
}

void AlbumIconView::slotCopy()
{
    if (!d->currentAlbum)
        return;

    KUrl::List      urls;
    KUrl::List      kioURLs;
    Q3ValueList<int> albumIDs;
    Q3ValueList<int> imageIDs;

    for (IconItem *it = firstItem(); it; it=it->nextItem())
    {
        if (it->isSelected())
        {
            AlbumIconItem *albumItem = static_cast<AlbumIconItem *>(it);
            ImageInfo info = albumItem->imageInfo();
            urls.append(info.fileUrl());
            kioURLs.append(info.databaseUrl());
            imageIDs.append(info.id());
        }
    }
    albumIDs.append(d->currentAlbum->id());

    if (urls.isEmpty())
        return;

    Q3DragObject* drag = 0;

    drag = new ItemDrag(urls, kioURLs, albumIDs, imageIDs, this);
    kapp->clipboard()->setData(drag);
}

void AlbumIconView::slotPaste()
{
    QMimeSource *data = kapp->clipboard()->data(QClipboard::Clipboard);
    if(!data || !Q3UriDrag::canDecode(data))
        return;

    if(d->currentAlbum->type() == Album::PHYSICAL)
    {
        if (Q3UriDrag::canDecode(data) &&
            d->currentAlbum->type() == Album::PHYSICAL)
        {
            PAlbum* palbum = (PAlbum*)d->currentAlbum;

            // B.K.O #119205: do not handle root album.
            if (palbum->isRoot())
                return;

            KUrl destURL(palbum->databaseUrl());

            KUrl::List srcURLs;
            K3URLDrag::decode(data, srcURLs);

            KIO::Job* job = DIO::copy(srcURLs, destURL);
            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(slotDIOResult(KJob*)));
        }
    }
}

void AlbumIconView::slotSetAlbumThumbnail(AlbumIconItem *iconItem)
{
    if(!d->currentAlbum)
        return;

    if(d->currentAlbum->type() == Album::PHYSICAL)
    {
        PAlbum *album = static_cast<PAlbum*>(d->currentAlbum);

        QString err;
        AlbumManager::componentData()->updatePAlbumIcon( album,
                                                    iconItem->imageInfo().id(),
                                                    err );
    }
    else if (d->currentAlbum->type() == Album::TAG)
    {
        TAlbum *album = static_cast<TAlbum*>(d->currentAlbum);

        QString err;
        AlbumManager::componentData()->updateTAlbumIcon( album,
                                                    QString(),
                                                    iconItem->imageInfo().id(),
                                                    err );
    }
}

void AlbumIconView::slotRename(AlbumIconItem* item)
{
    if (!item)
        return;

    // Create a copy of the item. After entering the event loop
    // in the dialog, we cannot be sure about the item's status.
    ImageInfo renameInfo = item->imageInfo();

    QFileInfo fi(renameInfo.name());
    QString ext  = QString(".") + fi.suffix();
    QString name = fi.fileName();
    name.truncate(fi.fileName().length() - ext.length());

    bool ok;

    QString newName = KInputDialog::getText(i18n("Rename Item (%1)",fi.fileName()), 
                                            i18n("Enter new name (without extension):"),
                                            name, &ok, this);
    if (!ok)
        return;

    KUrl oldURL = renameInfo.databaseUrl();
    KUrl newURL = oldURL;
    newURL.setFileName(newName + ext);

    KIO::CopyJob* job = DIO::rename(oldURL, newURL);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotDIOResult(KJob*)));

    connect(job, SIGNAL(copyingDone(KIO::Job *, const KUrl &, const KUrl &, bool, bool)),
            this, SLOT(slotRenamed(KIO::Job*, const KUrl &, const KUrl&)));

    // The AlbumManager KDirWatch will trigger a DIO::scan.
    // When this is completed, DIO will call AlbumLister::componentData()->refresh().
    // Usually the AlbumLister will ignore changes to already listed items.
    // So the renamed item need explicitly be invalidated.
    d->imageLister->invalidateItem(renameInfo);
}

void AlbumIconView::slotRenamed(KIO::Job*, const KUrl &, const KUrl&newURL)
{
    // reconstruct file path from digikamalbums:// URL
    KUrl fileURL;
    fileURL.setPath(newURL.user());
    fileURL.addPath(newURL.path());

    // refresh thumbnail
    d->pixMan->remove(fileURL);
    // clean LoadingCache as well - be pragmatic, do it here.
    LoadingCacheInterface::cleanFromCache(fileURL.path());
}

void AlbumIconView::slotDeleteSelectedItems(bool deletePermanently)
{
    KUrl::List  urlList;
    KUrl::List  kioUrlList;

    for (IconItem *it = firstItem(); it; it=it->nextItem())
    {
        if (it->isSelected()) 
        {
            AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
            ImageInfo info = iconItem->imageInfo();
            urlList.append(info.fileUrl());
            kioUrlList.append(info.databaseUrl());
        }
    }

    if (urlList.count() <= 0)
        return;

    DeleteDialog dialog(this);

    if (!dialog.confirmDeleteList(urlList,
                                  DeleteDialogMode::Files,
                                  deletePermanently ?
                                  DeleteDialogMode::NoChoiceDeletePermanently :
                                  DeleteDialogMode::NoChoiceTrash))
        return;

    bool useTrash = !dialog.shouldDelete();

    // trash does not like non-local URLs, put is not implemented
    KIO::Job* job = DIO::del(useTrash ? urlList : kioUrlList, useTrash);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotDIOResult(KJob*)));

    // The AlbumManager KDirWatch will trigger a DIO::scan.
    // When this is completed, DIO will call AlbumLister::componentData()->refresh().
}

void AlbumIconView::slotDeleteSelectedItemsDirectly(bool useTrash)
{
    // This method deletes the selected items directly, without confirmation.
    // It is not used in the default setup.

    KUrl::List kioUrlList;
    KUrl::List urlList;

    for (IconItem *it = firstItem(); it; it=it->nextItem())
    {
        if (it->isSelected())
        {
            AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
            ImageInfo info = iconItem->imageInfo();
            kioUrlList.append(info.databaseUrl());
            urlList.append(info.fileUrl());
        }
    }

    if (kioUrlList.count() <= 0)
        return;

    // trash does not like non-local URLs, put is not implemented
    KIO::Job* job = DIO::del(useTrash ? urlList : kioUrlList , useTrash);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotDIOResult(KJob*)));
}

void AlbumIconView::slotFilesModified()
{
    d->imageLister->refresh();
}

void AlbumIconView::slotFilesModified(const KUrl& url)
{
    refreshItems(url);
}

void AlbumIconView::slotImageWindowURLChanged(const KUrl &url)
{
    IconItem* item = findItem(url.url());
    if (item)
        setCurrentItem(item);
}

void AlbumIconView::slotDisplayItem(AlbumIconItem *item)
{
    if (!item) return;

    AlbumSettings *settings = AlbumSettings::componentData();

    if (!settings) return;

    QString currentFileExtension = item->imageInfo().name().section( '.', -1 );
    QString imagefilter = settings->getImageFileFilter().toLower() +
                          settings->getImageFileFilter().toUpper();

    if (KDcrawIface::DcrawBinary::componentData()->versionIsRight())
    {
        // add raw files only if dcraw is available
        imagefilter += settings->getRawFileFilter().toLower() +
                       settings->getRawFileFilter().toUpper();
    }

    // If the current item is not an image file.
    if ( !imagefilter.contains(currentFileExtension) )
    {
        KMimeType::Ptr mimePtr = KMimeType::findByUrl(item->imageInfo().fileUrl(), 0, true, true);
        const KService::List offers = KServiceTypeTrader::self()->query(mimePtr->name(), 
                                                                        "Type == 'Application'");
        
        if (offers.isEmpty())
            return;

        KService::Ptr ptr = offers.first();
        // Run the dedicated app to show the item.
        KRun::run(*ptr, item->imageInfo().fileUrl(), this);
        return;
    }

    // Run Digikam ImageEditor with all image files in the current Album.

    ImageInfoList imageInfoList;
    ImageInfo currentImageInfo;

    for (IconItem *it = firstItem() ; it ; it = it->nextItem())
    {
        AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
        QString fileExtension = iconItem->imageInfo().fileUrl().fileName().section( '.', -1 );

        if ( imagefilter.indexOf(fileExtension) != -1 )
        {
            ImageInfo info = iconItem->imageInfo();
            imageInfoList << info;
            if (iconItem == item)
                currentImageInfo = info;
        }
    }

    ImageWindow *imview = ImageWindow::imagewindow();

    imview->disconnect(this);

    connect(imview, SIGNAL(signalFileAdded(const KUrl&)),
            this, SLOT(slotFilesModified()));

    connect(imview, SIGNAL(signalFileModified(const KUrl&)),
            this, SLOT(slotFilesModified(const KUrl&)));

    connect(imview, SIGNAL(signalFileDeleted(const KUrl&)),
            this, SLOT(slotFilesModified()));

    connect(imview, SIGNAL(signalURLChanged(const KUrl&)),
            this, SLOT(slotImageWindowURLChanged(const KUrl &)));

    imview->loadImageInfos(imageInfoList,
                           currentImageInfo,
                           d->currentAlbum ? i18n("Album \"%1\"",d->currentAlbum->title()) : QString(),
                           true);

    if (imview->isHidden())
        imview->show();

    imview->raise();
    imview->setFocus();
}

void AlbumIconView::insertSelectionToLightTable()
{
    // Run Light Table with all selected image files in the current Album.

    ImageInfoList imageInfoList;

    for (IconItem *it = firstItem() ; it ; it = it->nextItem())
    {
        if ((*it).isSelected())
        {
            AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
            imageInfoList << iconItem->imageInfo();
        }
    }

    insertToLightTable(imageInfoList, imageInfoList.first());
}

void AlbumIconView::insertToLightTable(const ImageInfoList& list, const ImageInfo &current)
{
    LightTableWindow *ltview = LightTableWindow::lightTableWindow();

    ltview->disconnect(this);

    connect(ltview, SIGNAL(signalFileDeleted(const KUrl&)),
           this, SLOT(slotFilesModified()));

    connect(this, SIGNAL(signalItemsUpdated(const KUrl::List&)),
           ltview, SLOT(slotItemsUpdated(const KUrl::List&)));

    if (ltview->isHidden())
        ltview->show();

    ltview->raise();
    ltview->setFocus();
    ltview->loadImageInfos(list, current);
}

// ------------------------------------------------------------------------------

AlbumIconItem* AlbumIconView::firstSelectedItem() const
{
    AlbumIconItem *iconItem = 0;
    for (IconItem *it = firstItem(); it; it = it->nextItem())
    {
        if (it->isSelected())
        {
            iconItem = static_cast<AlbumIconItem *>(it);
            break;
        }
    }

    return iconItem;
}

const AlbumSettings* AlbumIconView::settings() const
{
    return d->albumSettings;
}

ThumbnailSize AlbumIconView::thumbnailSize() const
{
    return d->thumbSize;
}

void AlbumIconView::resizeEvent(QResizeEvent *e)
{
    IconView::resizeEvent(e);

    if (d->bannerRect.width() != frameRect().width())
        updateBannerRectPixmap();
}

// -- DnD ---------------------------------------------------

void AlbumIconView::startDrag()
{
    if (!d->currentAlbum)
        return;

    KUrl::List       urls;
    KUrl::List       kioURLs;
    Q3ValueList<int> albumIDs;
    Q3ValueList<int> imageIDs;

    for (IconItem *it = firstItem(); it; it=it->nextItem())
    {
        if (it->isSelected())
        {
            AlbumIconItem *albumItem = static_cast<AlbumIconItem *>(it);
            ImageInfo info = albumItem->imageInfo();
            urls.append(info.fileUrl());
            kioURLs.append(info.databaseUrl());
            imageIDs.append(info.id());
        }
    }
    albumIDs.append(d->currentAlbum->id());

    if (urls.isEmpty())
        return;

    QPixmap icon(DesktopIcon("image", 48));
    int w = icon.width();
    int h = icon.height();

    QPixmap pix(w+4,h+4);
    QString text(QString::number(urls.count()));

    QPainter p(&pix);
    p.fillRect(0, 0, w+4, h+4, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, w+4, h+4);
    p.drawPixmap(2, 2, icon);
    QRect r = p.boundingRect(2,2,w,h,Qt::AlignLeft|Qt::AlignTop,text);
    r.setWidth(qMax(r.width(),r.height()));
    r.setHeight(qMax(r.width(),r.height()));
    p.fillRect(r, QColor(0,80,0));
    p.setPen(Qt::white);
    QFont f(font());
    f.setBold(true);
    p.setFont(f);
    p.drawText(r, Qt::AlignCenter, text);
    p.end();

    Q3DragObject* drag = 0;

    drag = new ItemDrag(urls, kioURLs, albumIDs, imageIDs, this);
    if (drag)
    {
        drag->setPixmap(pix);
        drag->drag();
    }
}

void AlbumIconView::contentsDragMoveEvent(QDragMoveEvent *event)
{
    if (!d->currentAlbum || (AlbumDrag::canDecode(event) ||
                             !Q3UriDrag::canDecode(event) &&
                             !CameraDragObject::canDecode(event) &&
                             !TagListDrag::canDecode(event) &&
                             !TagDrag::canDecode(event))
        || event->source() == this) 
    {
        event->ignore();
        return;
    }
    event->accept();
}

void AlbumIconView::contentsDropEvent(QDropEvent *event)
{
    // TODO: need to rework this with specific to in which
    // groupitem items are dropped

    if (!d->currentAlbum || (AlbumDrag::canDecode(event) ||
                             !Q3UriDrag::canDecode(event) &&
                             !CameraDragObject::canDecode(event) &&
                             !TagListDrag::canDecode(event) &&
                             !TagDrag::canDecode(event))
         || event->source() == this)
    {
        event->ignore();
        return;
    }

    if (Q3UriDrag::canDecode(event) &&
        d->currentAlbum->type() == Album::PHYSICAL)
    {
        PAlbum* palbum = (PAlbum*)d->currentAlbum;
        KUrl destURL(palbum->databaseUrl());

        KUrl::List srcURLs = KUrl::List::fromMimeData( event->mimeData() );

        Q3PopupMenu popMenu(this);
        popMenu.insertItem( SmallIcon("goto"), i18n("&Move Here"), 10 );
        popMenu.insertItem( SmallIcon("editcopy"), i18n("&Copy Here"), 11 );
        popMenu.insertSeparator(-1);
        popMenu.insertItem( SmallIcon("cancel"), i18n("C&ancel") );

        popMenu.setMouseTracking(true);
        int id = popMenu.exec(QCursor::pos());
        switch(id) 
        {
            case 10: 
            {
                KIO::Job* job = DIO::move(srcURLs, destURL);
                connect(job, SIGNAL(result(KJob*)),
                        this, SLOT(slotDIOResult(KJob*)));
                break;
            }
            case 11: 
            {
                KIO::Job* job = DIO::copy(srcURLs, destURL);
                connect(job, SIGNAL(result(KJob*)),
                        this, SLOT(slotDIOResult(KJob*)));
                break;
            }
            default:
                break;
        }
    }
    else if(TagDrag::canDecode(event))
    {
        QByteArray ba = event->encodedData("digikam/tag-id");
        QDataStream ds(&ba, QIODevice::ReadOnly);
        int tagID;
        ds >> tagID;

        AlbumManager* man = AlbumManager::componentData();
        TAlbum* talbum    = man->findTAlbum(tagID);

        if (talbum)
        {
            Q3PopupMenu popMenu(this);

            bool moreItemsSelected = false;
            bool itemDropped = false;

            AlbumIconItem *albumItem = findItem(event->pos());
            if (albumItem)
                itemDropped = true;

            for (IconItem *it = firstItem(); it; it = it->nextItem())
            {
                if (it->isSelected() && it != albumItem)
                {
                    moreItemsSelected = true;
                    break;
                }
            }

            if (moreItemsSelected)
                popMenu.insertItem(SmallIcon("tag"), 
                                   i18n("Assign '%1' to &Selected Items",talbum->tagPath().mid(1)), 10);

            if (itemDropped)
                popMenu.insertItem(SmallIcon("tag"),
                                   i18n("Assign '%1' to &This Item",talbum->tagPath().mid(1)),   12);

            popMenu.insertItem(SmallIcon("tag"), 
                               i18n("Assign '%1' to &All Items",talbum->tagPath().mid(1)),          11);

            popMenu.insertSeparator(-1);
            popMenu.insertItem(SmallIcon("cancel"), i18n("&Cancel"));

            popMenu.setMouseTracking(true);
            int id = popMenu.exec(QCursor::pos());
            switch(id) 
            {
                case 10:    // Selected Items
                {
                    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode, 
                                               i18n("Assigning image tags. Please wait..."));

                    // always give a copy of the image infos (the "true"). Else there were crashes reported.
                    changeTagOnImageInfos(selectedImageInfos(), QList<int>() << tagID, true, true);

                    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
                    break;
                }
                case 11:    // All Items
                {
                    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode, 
                                               i18n("Assigning image tags. Please wait..."));

                    changeTagOnImageInfos(allImageInfos(), QList<int>() << tagID, true, true);

                    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
                    break;
                }
                case 12:    // Dropped Item only.
                {
                    AlbumIconItem *albumItem = findItem(event->pos());
                    if (albumItem)
                    {
                        ImageInfoList infos;
                        infos << albumItem->imageInfo();
                        changeTagOnImageInfos(infos, QList<int>() << tagID, true, false);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
    else if(TagListDrag::canDecode(event))
    {
        QByteArray ba = event->encodedData("digikam/taglist");
        QDataStream ds(&ba, QIODevice::ReadOnly);
        Q3ValueList<int> tagIDs;
        ds >> tagIDs;

        Q3PopupMenu popMenu(this);

        bool moreItemsSelected = false;
        bool itemDropped = false;

        AlbumIconItem *albumItem = findItem(event->pos());
        if (albumItem)
            itemDropped = true;

        for (IconItem *it = firstItem(); it; it = it->nextItem())
        {
            if (it->isSelected() && it != albumItem)
            {
                moreItemsSelected = true;
                break;
            }
        }

        if (moreItemsSelected)
            popMenu.insertItem(SmallIcon("tag"), i18n("Assign Tags to &Selected Items"), 10);

        if (itemDropped)
            popMenu.insertItem(SmallIcon("tag"), i18n("Assign Tags to &This Item"),   12);

        popMenu.insertItem(SmallIcon("tag"), i18n("Assign Tags to &All Items"),          11);

        popMenu.insertSeparator(-1);
        popMenu.insertItem(SmallIcon("cancel"), i18n("&Cancel"));

        popMenu.setMouseTracking(true);
        int id = popMenu.exec(QCursor::pos());
        switch(id) 
        {
            case 10:    // Selected Items
            {
                emit signalProgressBarMode(StatusProgressBar::ProgressBarMode, 
                                            i18n("Assigning image tags. Please wait..."));

                changeTagOnImageInfos(selectedImageInfos(), tagIDs, true, true);

                emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
                break;
            }
            case 11:    // All Items
            {
                emit signalProgressBarMode(StatusProgressBar::ProgressBarMode, 
                                            i18n("Assigning image tags. Please wait..."));

                changeTagOnImageInfos(allImageInfos(), tagIDs, true, true);

                emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
                break;
            }
            case 12:    // Dropped item only.
            {
                AlbumIconItem *albumItem = findItem(event->pos());
                if (albumItem)
                {
                    ImageInfoList infos;
                    infos << albumItem->imageInfo();
                    changeTagOnImageInfos(infos, tagIDs, true, false);
                }
                break;
            }
            default:
                break;
        }
    }
    else 
    {
        event->ignore();
    }
}

void AlbumIconView::changeTagOnImageInfos(const ImageInfoList &list, const QList<int> &tagIDs, bool addOrRemove, bool progress)
{
    float cnt = list.count();
    int i = 0;

    {
        DatabaseTransaction transaction;
        foreach(ImageInfo info, list)
        {
            MetadataHub hub;

            hub.load(info);

            for (QList<int>::const_iterator tagIt = tagIDs.begin(); tagIt != tagIDs.end(); ++tagIt)
            {
                hub.setTag(*tagIt, addOrRemove);
            }

            hub.write(info, MetadataHub::PartialWrite);
            hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);

            if (progress)
            {
                emit signalProgressValue((int)((i++/cnt)*100.0));
                kapp->processEvents();
            }
        }
    }

    if (d->currentAlbum && d->currentAlbum->type() == Album::TAG)
    {
        d->imageLister->refresh();
    }
    updateContents();
}

bool AlbumIconView::acceptToolTip(IconItem *item, const QPoint &mousePos)
{
    AlbumIconItem *iconItem = dynamic_cast<AlbumIconItem*>(item);

    if (iconItem && iconItem->clickToOpenRect().contains(mousePos))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void AlbumIconView::slotShowToolTip(IconItem* item)
{
    d->toolTip->setIconItem(dynamic_cast<AlbumIconItem*>(item));
}

KUrl::List AlbumIconView::allItems()
{
    KUrl::List itemList;

     for (IconItem *it = firstItem(); it; it = it->nextItem())
     {
         AlbumIconItem *item = (AlbumIconItem*) it;
         itemList.append(item->imageInfo().fileUrl());
     }

    return itemList;
}

KUrl::List AlbumIconView::selectedItems()
{
    KUrl::List itemList;

     for (IconItem *it = firstItem(); it; it = it->nextItem())
     {
         if (it->isSelected())
         {
             AlbumIconItem *item = (AlbumIconItem*) it;
             itemList.append(item->imageInfo().fileUrl());
         }
     }

    return itemList;
}

ImageInfoList AlbumIconView::allImageInfos() const
{
    // Returns the list of ImageInfos of all items,
    // with the extra feature that the currentItem is the first in the list.
    ImageInfoList list;
    for (IconItem *it = firstItem(); it; it = it->nextItem())
    {
        AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
        ImageInfo info = iconItem->imageInfo();

        if (iconItem == currentItem())
            list.prepend(info);
        else
            list.append(info);
    }
    return list;
}

ImageInfoList AlbumIconView::selectedImageInfos() const
{
    // Returns the list of ImageInfos of currently selected items,
    // with the extra feature that the currentItem is the first in the list.
    ImageInfoList list;
    for (IconItem *it = firstItem(); it; it = it->nextItem())
    {
        AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
        if (it->isSelected())
        {
            ImageInfo info = iconItem->imageInfo();

            if (iconItem == currentItem())
                list.prepend(info);
            else
                list.append(info);
        }
    }
    return list;
}

void AlbumIconView::refresh()
{
    d->imageLister->stop();
    clear();

    d->imageLister->openAlbum(d->currentAlbum);
}

void AlbumIconView::refreshItems(const KUrl::List& urlList)
{
    if (!d->currentAlbum || urlList.empty())
        return;

    // we do two things here:
    // 1. refresh the imageinfo for the file
    // 2. refresh the thumbnails

    for (KUrl::List::const_iterator it = urlList.begin();
         it != urlList.end(); ++it)
    {
        AlbumIconItem* iconItem = findItem((*it).url());
        if (!iconItem)
            continue;

        ImageInfo info = iconItem->imageInfo();
        info.refresh();
        d->pixMan->remove(info.fileUrl());
        // clean LoadingCache as well - be pragmatic, do it here.
        LoadingCacheInterface::cleanFromCache((*it).path());
    }

    emit signalItemsUpdated(urlList);

    // trigger a delayed rearrangement, in case we need to resort items
    triggerRearrangement();
}

void AlbumIconView::slotGotThumbnail(const KUrl& url)
{
    AlbumIconItem* iconItem = findItem(url.url());
    if (!iconItem)
        return;

    iconItem->repaint();
}

void AlbumIconView::slotSelectionChanged()
{
    if (firstSelectedItem())
        emitItemsSelected(true);
    else
        emitItemsSelected(false);
}

void AlbumIconView::slotSetExifOrientation( int orientation )
{
    KUrl::List urlList;
    int i = 0;

    for (IconItem *it = firstItem(); it; it=it->nextItem())
    {
        if (it->isSelected()) 
        {
            AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
            urlList.append(iconItem->imageInfo().fileUrl());
        }
    }

    if (urlList.count() <= 0) return;

    KUrl::List::Iterator it;
    float cnt = (float)urlList.count();
    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode, 
                                i18n("Revising Exif Orientation tags. Please wait..."));

    for( it = urlList.begin(); it != urlList.end(); ++it )
    {
        DDebug() << "Setting Exif Orientation tag to " << orientation << endl;

        DMetadata metadata((*it).path());
        DMetadata::ImageOrientation o = (DMetadata::ImageOrientation)orientation;
        metadata.setImageOrientation(o);

        if (!metadata.applyChanges())
        {
            KMessageBox::sorry(0, i18n("Failed to revise Exif orientation for file %1."
                               ,(*it).fileName()));
            return;
        }
        else
        {
            ImageAttributesWatch::componentData()->fileMetadataChanged((*it));
        }

        emit signalProgressValue((int)((i++/cnt)*100.0));
        kapp->processEvents();
    }

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());    
    refreshItems(urlList);
}

QRect AlbumIconView::itemRect() const
{
    return d->itemRect;
}

QRect AlbumIconView::itemRatingRect() const
{
    return d->itemRatingRect;
}

QRect AlbumIconView::itemDateRect() const
{
    return d->itemDateRect;
}

QRect AlbumIconView::itemModDateRect() const
{
    return d->itemModDateRect;
}

QRect AlbumIconView::itemPixmapRect() const
{
    return d->itemPixmapRect;
}

QRect AlbumIconView::itemNameRect() const
{
    return d->itemNameRect;
}

QRect AlbumIconView::itemCommentsRect() const
{
    return d->itemCommentsRect;
}

QRect AlbumIconView::itemResolutionRect() const
{
    return d->itemResolutionRect;
}

QRect AlbumIconView::itemTagRect() const
{
    return d->itemTagRect;
}

QRect AlbumIconView::itemSizeRect() const
{
    return d->itemSizeRect;
}

QRect AlbumIconView::bannerRect() const
{
    return d->bannerRect;
}

QPixmap* AlbumIconView::itemBaseRegPixmap() const
{
    return &d->itemRegPixmap;
}

QPixmap* AlbumIconView::itemBaseSelPixmap() const
{
    return &d->itemSelPixmap;
}

QPixmap AlbumIconView::bannerPixmap() const
{
    return d->bannerPixmap;
}

QPixmap AlbumIconView::ratingPixmap() const
{
    return d->ratingPixmap;    
}

QFont AlbumIconView::itemFontReg() const
{
    return d->fnReg;
}

QFont AlbumIconView::itemFontCom() const
{
    return d->fnCom;
}

QFont AlbumIconView::itemFontXtra() const
{
    return d->fnXtra;
}

void AlbumIconView::updateBannerRectPixmap()
{
    d->bannerRect = QRect(0, 0, 0, 0);

    // Title --------------------------------------------------------
    QFont fn(font());
    int fnSize = fn.pointSize();
    bool usePointSize;
    if (fnSize > 0)
    {
        fn.setPointSize(fnSize+2);
        usePointSize = true;
    }
    else
    {
        fnSize = fn.pixelSize();
        fn.setPixelSize(fnSize+2);
        usePointSize = false;
    }

    fn.setBold(true);
    QFontMetrics fm(fn);
    QRect tr = fm.boundingRect(0, 0, frameRect().width(),
                               0xFFFFFFFF, Qt::AlignLeft | Qt::AlignVCenter,
                               "XXX");
    d->bannerRect.setHeight(tr.height());

    if (usePointSize)
        fn.setPointSize(font().pointSize());
    else
        fn.setPixelSize(font().pixelSize());

    fn.setBold(false);
    fm = QFontMetrics(fn);

    tr = fm.boundingRect(0, 0, frameRect().width(),
                         0xFFFFFFFF, Qt::AlignLeft | Qt::AlignVCenter,
                         "XXX");

    d->bannerRect.setHeight(d->bannerRect.height() + tr.height() + 10);
    d->bannerRect.setWidth(frameRect().width());

    d->bannerPixmap = ThemeEngine::componentData()->bannerPixmap(d->bannerRect.width(),
                                                            d->bannerRect.height());
}

void AlbumIconView::updateItemRectsPixmap()
{
    d->itemRect           = QRect(0,0,0,0);
    d->itemRatingRect     = QRect(0,0,0,0);
    d->itemDateRect       = QRect(0,0,0,0);
    d->itemModDateRect    = QRect(0,0,0,0);
    d->itemPixmapRect     = QRect(0,0,0,0);
    d->itemNameRect       = QRect(0,0,0,0);
    d->itemCommentsRect   = QRect(0,0,0,0);
    d->itemResolutionRect = QRect(0,0,0,0);
    d->itemSizeRect       = QRect(0,0,0,0);
    d->itemTagRect        = QRect(0,0,0,0);

    d->fnReg  = font();
    d->fnCom  = font();
    d->fnXtra = font();
    d->fnCom.setItalic(true);

    int fnSz = d->fnReg.pointSize();
    if (fnSz > 0)
    {
        d->fnCom.setPointSize(fnSz-1);
        d->fnXtra.setPointSize(fnSz-2);
    }
    else
    {
        fnSz = d->fnReg.pixelSize();
        d->fnCom.setPixelSize(fnSz-1);
        d->fnXtra.setPixelSize(fnSz-2);
    }

    int margin  = 5;
    int w = d->thumbSize.size() + 2*margin;

    QFontMetrics fm(d->fnReg);
    QRect oneRowRegRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                          Qt::AlignTop | Qt::AlignHCenter,
                                          "XXXXXXXXX");
    fm = QFontMetrics(d->fnCom);
    QRect oneRowComRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                          Qt::AlignTop | Qt::AlignHCenter,
                                          "XXXXXXXXX");
    fm = QFontMetrics(d->fnXtra);
    QRect oneRowXtraRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                           Qt::AlignTop | Qt::AlignHCenter,
                                           "XXXXXXXXX");

    int y = margin;

    d->itemPixmapRect = QRect(margin, y, w, d->thumbSize.size()+margin);
    y = d->itemPixmapRect.bottom();

    if (d->albumSettings->getIconShowRating())
    {
        d->itemRatingRect = QRect(margin, y, w, d->ratingPixmap.height());
        y = d->itemRatingRect.bottom();
    }
    
    if (d->albumSettings->getIconShowName())
    {
        d->itemNameRect = QRect(margin, y, w, oneRowRegRect.height());
        y = d->itemNameRect.bottom();
    }

    if (d->albumSettings->getIconShowComments())
    {
        d->itemCommentsRect = QRect(margin, y, w, oneRowComRect.height());
        y = d->itemCommentsRect.bottom();
    }


    if (d->albumSettings->getIconShowDate())
    {
        d->itemDateRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->itemDateRect.bottom();
    }

    if (d->albumSettings->getIconShowModDate())
    {
        d->itemModDateRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->itemModDateRect.bottom();
    }

    if (d->albumSettings->getIconShowResolution())
    {
        d->itemResolutionRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->itemResolutionRect.bottom() ;
    }

    if (d->albumSettings->getIconShowSize())
    {
        d->itemSizeRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->itemSizeRect.bottom();
    }

    if (d->albumSettings->getIconShowTags())
    {
        d->itemTagRect = QRect(margin, y, w, oneRowComRect.height());
        y = d->itemTagRect.bottom();
    }

    d->itemRect = QRect(0, 0, w+2*margin, y+margin);

    d->itemRegPixmap = ThemeEngine::componentData()->thumbRegPixmap(d->itemRect.width(),
                                                               d->itemRect.height());

    d->itemSelPixmap = ThemeEngine::componentData()->thumbSelPixmap(d->itemRect.width(),
                                                               d->itemRect.height());
}

void AlbumIconView::slotThemeChanged()
{
    QPalette plt(palette());
    plt.setColor(QPalette::Active, QPalette::Base, 
                 ThemeEngine::componentData()->baseColor());
    plt.setColor(QPalette::Active, QPalette::Text, 
                 ThemeEngine::componentData()->textRegColor());
    plt.setColor(QPalette::Active, QPalette::HighlightedText, 
                 ThemeEngine::componentData()->textSelColor());
    plt.setColor(QPalette::Inactive, QPalette::Base, 
                 ThemeEngine::componentData()->baseColor());
    plt.setColor(QPalette::Inactive, QPalette::Text, 
                 ThemeEngine::componentData()->textRegColor());
    plt.setColor(QPalette::Inactive, QPalette::HighlightedText, 
                 ThemeEngine::componentData()->textSelColor());
    setPalette(plt);

    QPainter painter(&d->ratingPixmap);
    painter.fillRect(0, 0, d->ratingPixmap.width(), d->ratingPixmap.height(),
                     ThemeEngine::componentData()->textSpecialRegColor());
    painter.end();
    
    updateBannerRectPixmap();
    updateItemRectsPixmap();

    viewport()->update();
}

AlbumIconItem* AlbumIconView::findItem(const QPoint& pos)
{
    return dynamic_cast<AlbumIconItem*>(IconView::findItem(pos));
}

AlbumIconItem* AlbumIconView::findItem(const QString& url) const
{
    return d->itemDict.find(url);
}

AlbumIconItem* AlbumIconView::nextItemToThumbnail() const
{
    QRect r(contentsX(), contentsY(), visibleWidth(), visibleHeight());
    IconItem *fItem = findFirstVisibleItem(r);
    IconItem *lItem = findLastVisibleItem(r);
    if (!fItem || !lItem)
        return 0;

    AlbumIconItem* firstItem = static_cast<AlbumIconItem*>(fItem);
    AlbumIconItem* lastItem  = static_cast<AlbumIconItem*>(lItem);
    AlbumIconItem* item      = firstItem;
    while (item)
    {
        if (item->isDirty())
            return item;
        if (item == lastItem)
            break;
        item = (AlbumIconItem*)item->nextItem();
    }

    return 0;
}

PixmapManager* AlbumIconView::pixmapManager() const
{
    return d->pixMan;
}

void AlbumIconView::slotAlbumModified()
{
    d->imageLister->stop();
    clear();

    d->imageLister->openAlbum(d->currentAlbum);

    updateBannerRectPixmap();
    updateItemRectsPixmap();
}

void AlbumIconView::slotAssignTag(int tagID)
{
    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode, 
                                i18n("Assigning image tags. Please wait..."));

    changeTagOnImageInfos(selectedImageInfos(), QList<int>() << tagID, true, true);

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
}

void AlbumIconView::slotRemoveTag(int tagID)
{
    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode, 
                                i18n("Removing image tags. Please wait..."));

    changeTagOnImageInfos(selectedImageInfos(), QList<int>() << tagID, false, true);

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
}

void AlbumIconView::slotAssignRating(int rating)
{
    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode,
                                i18n("Assigning image ratings. Please wait..."));

    int   i   = 0;
    float cnt = (float)countSelected();
    rating    = qMin(RatingMax, qMax(RatingMin, rating));

    {
        DatabaseTransaction transaction;
        for (IconItem *it = firstItem() ; it ; it = it->nextItem())
        {
            if (it->isSelected())
            {
                AlbumIconItem *albumItem = static_cast<AlbumIconItem *>(it);
                ImageInfo info           = albumItem->imageInfo();

                MetadataHub hub;
                hub.load(info);
                hub.setRating(rating);
                hub.write(info, MetadataHub::PartialWrite);
                hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);

                emit signalProgressValue((int)((i++/cnt)*100.0));
                kapp->processEvents();
            }
        }
    }

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
    updateContents();
}

void AlbumIconView::slotAssignRatingNoStar()
{
    slotAssignRating(0);
}

void AlbumIconView::slotAssignRatingOneStar()
{
    slotAssignRating(1);
}

void AlbumIconView::slotAssignRatingTwoStar()
{
    slotAssignRating(2);
}

void AlbumIconView::slotAssignRatingThreeStar()
{
    slotAssignRating(3);
}

void AlbumIconView::slotAssignRatingFourStar()
{
    slotAssignRating(4);
}

void AlbumIconView::slotAssignRatingFiveStar()
{
    slotAssignRating(5);
}

void AlbumIconView::slotDIOResult(KJob* kjob)
{
    KIO::Job *job = static_cast<KIO::Job*>(kjob);
    if (job->error())
    {
        job->ui()->setWindow(this);
        job->ui()->showErrorMessage();
    }
}

void AlbumIconView::slotImageAttributesChanged(qlonglong imageId)
{
    AlbumIconItem *firstItem = static_cast<AlbumIconItem *>(findFirstVisibleItem());
    AlbumIconItem *lastItem  = static_cast<AlbumIconItem *>(findLastVisibleItem());
    for (AlbumIconItem *item = firstItem; item;
         item = static_cast<AlbumIconItem *>(item->nextItem()))
    {
        if (item->imageInfo().id() == imageId)
        {
            updateContents();
            return;
        }
        if (item == lastItem)
            break;
    }
}

void AlbumIconView::slotAlbumImagesChanged(int /*albumId*/)
{
    updateContents();
}

}  // namespace Digikam
