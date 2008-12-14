/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-18
 * Description : camera icon view
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

#include "cameraiconview.h"
#include "cameraiconview.moc"

// Qt includes.

#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QCursor>
#include <QFontMetrics>
#include <QFont>
#include <QClipboard>
#include <QDropEvent>
#include <QHash>

// KDE includes.

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmimetype.h>

// Local includes.

#include "themeengine.h"
#include "thumbnailsize.h"
#include "gpiteminfo.h"
#include "renamecustomizer.h"
#include "icongroupitem.h"
#include "dpopupmenu.h"
#include "ddragobjects.h"
#include "cameraui.h"
#include "cameraiconitem.h"

namespace Digikam
{

class CameraIconViewPriv
{

public:

    CameraIconViewPriv()
    {
        renamer   = 0;
        groupItem = 0;
        cameraUI  = 0;
        thumbSize = ThumbnailSize::Large;
    }

    int                                 thumbSize;

    QHash<QString, CameraIconViewItem*> itemDict;

    QRect                               itemRect;

    QPixmap                             itemRegPixmap;
    QPixmap                             itemSelPixmap;

    RenameCustomizer                   *renamer;

    IconGroupItem                      *groupItem;

    CameraUI                           *cameraUI;
};

CameraIconView::CameraIconView(CameraUI* ui, QWidget* parent)
              : IconView(parent), d(new CameraIconViewPriv)
{
    d->cameraUI  = ui;
    d->groupItem = new IconGroupItem(this);

    setHScrollBarMode(Q3ScrollView::AlwaysOff);
    setMinimumSize(400, 300);

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    // ----------------------------------------------------------------

    connect(this, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(this, SIGNAL(signalNewSelection(bool)),
            this, SLOT(slotUpdateDownloadNames(bool)));

    connect(this, SIGNAL(signalRightButtonClicked(IconItem*, const QPoint&)),
            this, SLOT(slotContextMenu(IconItem*, const QPoint&)));

    connect(this, SIGNAL(signalRightButtonClicked(const QPoint &)),
            this, SLOT(slotRightButtonClicked(const QPoint &)));

    connect(this, SIGNAL(signalDoubleClicked(IconItem*)),
            this, SLOT(slotDoubleClicked(IconItem*)));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // ----------------------------------------------------------------

    updateItemRectsPixmap();
    slotThemeChanged();
}

CameraIconView::~CameraIconView()
{
    clear();
    delete d;
}

QPixmap CameraIconView::itemBaseRegPixmap() const
{
    return d->itemRegPixmap;
}

QPixmap CameraIconView::itemBaseSelPixmap() const
{
    return d->itemSelPixmap;
}

void CameraIconView::setRenameCustomizer(RenameCustomizer* renamer)
{
    d->renamer = renamer;

    connect(d->renamer, SIGNAL(signalChanged()),
            this, SLOT(slotDownloadNameChanged()));
}

void CameraIconView::addItem(const GPItemInfo& info)
{
    KIconLoader* iconLoader = KIconLoader::global();
    QImage thumb;

    // Just to have a generic image thumb from desktop with KDE < 3.5.0
    KMimeType::Ptr mime = KMimeType::mimeType(info.mime == QString("image/x-raw") ?
                                              QString("image/tiff") : info.mime);

    if (mime)
    {
        thumb = iconLoader->loadIcon(mime->iconName(), KIconLoader::Desktop,
                                     ThumbnailSize::Huge, KIconLoader::DefaultState)
                                     .toImage();
    }
    else
    {
        thumb = iconLoader->loadIcon("empty", KIconLoader::Desktop,
                                     ThumbnailSize::Huge, KIconLoader::DefaultState)
                                     .toImage();
    }

    QString downloadName;

    if (d->renamer)
    {
        if (!d->renamer->useDefault())
        {
            downloadName = getTemplatedName( &info, d->itemDict.count() );
        }
        else
        {
            downloadName = getCasedName( d->renamer->changeCase(), &info);
        }
    }

    CameraIconViewItem* item = new CameraIconViewItem(d->groupItem, info, thumb, downloadName);
    d->itemDict.insert(info.folder+info.name, item);
}

void CameraIconView::removeItem(const QString& folder, const QString& file)
{
    CameraIconViewItem* item = d->itemDict.value(folder+file);
    if (!item)
        return;
    d->itemDict.remove(folder+file);

    setDelayedRearrangement(true);
    delete item;
    setDelayedRearrangement(false);
}

CameraIconViewItem* CameraIconView::findItem(const QString& folder, const QString& file)
{
    return d->itemDict.value(folder+file);
}

int CameraIconView::countItemsByFolder(QString folder)
{
    int count = 0;
    if (folder.endsWith('/')) folder.truncate(folder.length()-1);

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        QString itemFolder = iconItem->itemInfo()->folder;
        if (itemFolder.endsWith('/')) itemFolder.truncate(itemFolder.length()-1);

        if (folder == itemFolder)
            count++;
    }

    return count;
}

void CameraIconView::setThumbnail(const QString& folder, const QString& filename, const QImage& image)
{
    CameraIconViewItem* item = d->itemDict.value(folder+filename);
    if (!item)
        return;

    item->setThumbnail(image);
    item->update();
}

void CameraIconView::ensureItemVisible(CameraIconViewItem *item)
{
    IconView::ensureItemVisible(item);
}

void CameraIconView::ensureItemVisible(const GPItemInfo& itemInfo)
{
    ensureItemVisible(itemInfo.folder, itemInfo.name);
}

void CameraIconView::ensureItemVisible(const QString& folder, const QString& file)
{
    CameraIconViewItem* item = d->itemDict.value(folder+file);
    if (!item)
        return;

    ensureItemVisible(item);
}

void CameraIconView::slotDownloadNameChanged()
{
    bool hasSelection = false;
    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        if (item->isSelected())
        {
            hasSelection = true;
            break;
        }
    }

    // connected to slotUpdateDownloadNames, and used externally
    emit signalNewSelection(hasSelection);
}

void CameraIconView::slotUpdateDownloadNames(bool hasSelection)
{
    bool useDefault = true;
    int  startIndex = 0;

    if (d->renamer)
    {
        useDefault = d->renamer->useDefault();
        startIndex = d->renamer->startIndex() -1;
    }

    bool convertLossLessJpeg = d->cameraUI->convertLosslessJpegFiles();
    QString losslessFormat   = d->cameraUI->losslessFormat();

    viewport()->setUpdatesEnabled(false);

    if (hasSelection)
    {
        // Camera items selection.

        for (IconItem* item = firstItem(); item; item = item->nextItem())
        {
            QString downloadName;
            CameraIconViewItem* viewItem = static_cast<CameraIconViewItem*>(item);

            if (item->isSelected())
            {
                if (!useDefault)
                    downloadName = getTemplatedName( viewItem->itemInfo(), startIndex );
                else
                    downloadName = getCasedName( d->renamer->changeCase(), viewItem->itemInfo() );

                startIndex++;
            }

            if (convertLossLessJpeg && !downloadName.isEmpty())
            {
                QFileInfo fi(downloadName);
                QString ext = fi.suffix().toUpper();
                if (ext == QString("JPEG") || ext == QString("JPG") || ext == QString("JPE"))
                {
                    downloadName.truncate(downloadName.length() - ext.length());
                    downloadName.append(losslessFormat.toLower());
                }
            }

            viewItem->setDownloadName( downloadName );
        }
    }
    else
    {
        // No camera item selection.

        for (IconItem* item = firstItem(); item; item = item->nextItem())
        {
            QString downloadName;
            CameraIconViewItem* viewItem = static_cast<CameraIconViewItem*>(item);

            if (!useDefault)
                downloadName = getTemplatedName( viewItem->itemInfo(), startIndex );
            else
                downloadName = getCasedName( d->renamer->changeCase(), viewItem->itemInfo() );

            if (convertLossLessJpeg)
            {
                QFileInfo fi(downloadName);
                QString ext = fi.suffix().toUpper();
                if (ext == QString("JPEG") || ext == QString("JPG") || ext == QString("JPE"))
                {
                    downloadName.truncate(downloadName.length() - ext.length());
                    downloadName.append(losslessFormat.toLower());
                }
            }

            viewItem->setDownloadName( downloadName );
            startIndex++;
        }
    }

    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

QString CameraIconView::defaultDownloadName(CameraIconViewItem *viewItem)
{
    RenameCustomizer::Case renamecase = RenameCustomizer::NONE;
    if (d->renamer)
        renamecase = d->renamer->changeCase();

    return getCasedName( renamecase, viewItem->itemInfo() );
}

QString CameraIconView::getTemplatedName(const GPItemInfo* itemInfo, int position)
{
    QString ext = itemInfo->name;
    int pos     = ext.lastIndexOf('.');
    if (pos < 0)
        ext = "";
    else
        ext = ext.right( ext.length() - pos );

    return d->renamer->newName(itemInfo->mtime, position+1, ext);
}

QString CameraIconView::getCasedName(const RenameCustomizer::Case ccase,
                                     const GPItemInfo* itemInfo)
{
    QString dname;

    switch (ccase)
    {
        case(RenameCustomizer::UPPER):
        {
            dname = itemInfo->name.toUpper();
            break;
        }
        case(RenameCustomizer::LOWER):
        {
            dname = itemInfo->name.toLower();
            break;
        }
        default:
        {
            dname = itemInfo->name;
            break;
        }
    };

    return dname;
}

void CameraIconView::slotSelectionChanged()
{
    bool selected               = false;
    CameraIconViewItem* camItem = 0;

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        if (item->isSelected())
        {
            camItem  = static_cast<CameraIconViewItem*>(item);
            selected = true;
            break;
        }
    }

    emit signalNewSelection(selected);
    emit signalSelected(camItem, selected);

    viewport()->update();
}

CameraIconViewItem* CameraIconView::firstItemSelected()
{
    CameraIconViewItem* camItem = 0;

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        if (item->isSelected())
        {
            camItem = static_cast<CameraIconViewItem*>(item);
            break;
        }
    }

    return(camItem);
}


void CameraIconView::slotContextMenu(IconItem * item, const QPoint&)
{
    if (!item)
        return;

    // don't popup context menu if the camera is busy
    if (d->cameraUI->isBusy())
        return;

    CameraIconViewItem* camItem = static_cast<CameraIconViewItem*>(item);

    DPopupMenu menu(this);
    QAction *viewAction      = menu.addAction(SmallIcon("editimage"), i18n("&View"));
    menu.addSeparator();
    QAction *downAction      = menu.addAction(SmallIcon("computer"),i18n("Download"));
    QAction *downDelAction   = menu.addAction(SmallIcon("computer"),i18n("Download && Delete"));
    QAction *encryptedAction = menu.addAction(SmallIcon("object-locked"), i18n("Toggle Lock"));
    menu.addSeparator();
    QAction *deleteAction    = menu.addAction(SmallIcon("edit-delete"), i18n("Delete"));

    downDelAction->setEnabled(d->cameraUI->cameraDeleteSupport());
    deleteAction->setEnabled(d->cameraUI->cameraDeleteSupport());

    QAction *choice = menu.exec(QCursor::pos());

    if (choice)
    {
        if (choice == viewAction)
        {
            emit signalFileView(camItem);
        }
        else if (choice == downAction)
        {
            emit signalDownload();
        }
        else if (choice == deleteAction)
        {
            emit signalDelete();
        }
        else if (choice == encryptedAction)
        {
            emit signalToggleLock();
        }
        else if (choice == downDelAction)
        {
            emit signalDownloadAndDelete();
        }
    }
}

void CameraIconView::slotDoubleClicked(IconItem* item)
{
    if (!item)
        return;

    if (d->cameraUI->isBusy())
        return;

    emit signalFileView(static_cast<CameraIconViewItem*>(item));
}

void CameraIconView::slotSelectAll()
{
    selectAll();
}

void CameraIconView::slotSelectNone()
{
    clearSelection();
}

void CameraIconView::slotSelectInvert()
{
    invertSelection();
}

void CameraIconView::slotSelectNew()
{
    blockSignals(true);
    clearSelection();

    for (IconItem* item = firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* viewItem = static_cast<CameraIconViewItem*>(item);
        if (viewItem->itemInfo()->downloaded == GPItemInfo::NewPicture)
        {
            viewItem->setSelected(true, false);
        }
    }

    blockSignals(false);
    emit signalSelectionChanged();
}

void CameraIconView::slotSelectLocked()
{
    blockSignals(true);
    clearSelection();

    for (IconItem* item = firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* viewItem = static_cast<CameraIconViewItem*>(item);
        if (viewItem->itemInfo()->writePermissions == 0)
        {
            viewItem->setSelected(true, false);
        }
    }

    blockSignals(false);
    emit signalSelectionChanged();
}

void CameraIconView::startDrag()
{
    QStringList lst;

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        if (!item->isSelected())
            continue;

        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        QString itemPath = iconItem->itemInfo()->folder + iconItem->itemInfo()->name;
        lst.append(itemPath);
    }

    QDrag *drag = new QDrag(d->cameraUI);
    drag->setMimeData(new DCameraItemListDrag(lst));

    QPixmap icon(DesktopIcon("image-jp2", 48));
    int w = icon.width();
    int h = icon.height();

    QPixmap pix(w+4,h+4);
    QString text(QString::number(lst.count()));

    QPainter p(&pix);
    p.fillRect(0, 0, w+4, h+4, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, w+4, h+4);
    p.drawPixmap(2, 2, icon);
    QRect r = p.boundingRect(2,2,w,h,Qt::AlignLeft|Qt::AlignTop,text);
    r.setWidth(qMax(r.width(),r.height()));
    r.setHeight(qMax(r.width(),r.height()));
    p.fillRect(r, QColor(0, 80, 0));
    p.setPen(Qt::white);
    QFont f(font());
    f.setBold(true);
    p.setFont(f);
    p.drawText(r, Qt::AlignCenter, text);
    p.end();

    drag->setPixmap(pix);
    drag->exec();
}

void CameraIconView::contentsDropEvent(QDropEvent *e)
{
    // Don't popup context menu if the camera is busy or if camera do not support upload.
    if (d->cameraUI->isBusy() || d->cameraUI->cameraUploadSupport())
        return;

    if ( (!KUrl::List::canDecode(e->mimeData()) && !DCameraDragObject::canDecode(e->mimeData()) )
         || e->source() == this)
    {
        e->ignore();
        return;
    }

    KUrl::List srcURLs = KUrl::List::fromMimeData(e->mimeData());
    uploadItemPopupMenu(srcURLs);
}

void CameraIconView::slotRightButtonClicked(const QPoint&)
{
    // Don't popup context menu if the camera is busy or if camera do not support upload.
    if (d->cameraUI->isBusy() || d->cameraUI->cameraUploadSupport())
        return;

    const QMimeData *data = kapp->clipboard()->mimeData(QClipboard::Clipboard);
    if(!data || !KUrl::List::canDecode(data))
        return;

    KUrl::List srcURLs = KUrl::List::fromMimeData(data);
    uploadItemPopupMenu(srcURLs);
}

void CameraIconView::uploadItemPopupMenu(const KUrl::List& srcURLs)
{
    KMenu popMenu(this);
    popMenu.addTitle(SmallIcon("digikam"), d->cameraUI->cameraTitle());
    QAction *uploadAction = popMenu.addAction(SmallIcon("file-import"), i18n("&Upload to camera"));
    popMenu.addSeparator();
    popMenu.addAction( SmallIcon("dialog-cancel"), i18n("C&ancel") );

    popMenu.setMouseTracking(true);
    QAction *choice = popMenu.exec(QCursor::pos());
    if (choice == uploadAction)
        emit signalUpload(srcURLs);
}

QRect CameraIconView::itemRect() const
{
    return d->itemRect;
}

void CameraIconView::setThumbnailSize(int size)
{
    if ( d->thumbSize != size)
    {
        if (size > ThumbnailSize::Huge)
            d->thumbSize = ThumbnailSize::Huge;
        else if (size < ThumbnailSize::Small)
            d->thumbSize = ThumbnailSize::Small;
        else
            d->thumbSize = size;

        updateItemRectsPixmap();
        triggerRearrangement();
        emit signalThumbSizeChanged(d->thumbSize);
    }
}

int CameraIconView::thumbnailSize()
{
    return d->thumbSize;
}

void CameraIconView::updateItemRectsPixmap()
{
    QRect pixRect;
    QRect textRect;
    QRect extraRect;

    pixRect.setWidth(d->thumbSize);
    pixRect.setHeight(d->thumbSize);

    QFontMetrics fm(font());
    QRect r = QRect(fm.boundingRect(0, 0, d->thumbSize, 0xFFFFFFFF,
                                    Qt::AlignHCenter | Qt::AlignTop,
                                    "XXXXXXXXX"));
    textRect.setWidth(r.width());
    textRect.setHeight(r.height());

    QFont fn(font());
    if (fn.pointSize() > 0)
    {
        fn.setPointSize(qMax(fn.pointSize()-2, 6));
    }

    fm = QFontMetrics(fn);
    r = QRect(fm.boundingRect(0, 0, d->thumbSize, 0xFFFFFFFF,
                              Qt::AlignHCenter | Qt::AlignTop,
                              "XXXXXXXXX"));
    extraRect.setWidth(r.width());
    extraRect.setHeight(r.height());

    r = QRect();
    r.setWidth(qMax(qMax(pixRect.width(), textRect.width()), extraRect.width()) + 4);
    r.setHeight(pixRect.height() + textRect.height() + extraRect.height() + 4);

    d->itemRect = r;

    d->itemRegPixmap = ThemeEngine::instance()->thumbRegPixmap(d->itemRect.width(),
                                                               d->itemRect.height());

    d->itemSelPixmap = ThemeEngine::instance()->thumbSelPixmap(d->itemRect.width(),
                                                               d->itemRect.height());
}

void CameraIconView::slotThemeChanged()
{
    updateItemRectsPixmap();
    viewport()->update();
}

int CameraIconView::itemsDownloaded()
{
    int downloaded = 0;

    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);

        if (iconItem->itemInfo()->downloaded == GPItemInfo::DownloadedYes)
            downloaded++;
    }

    return downloaded;
}

void CameraIconView::itemsSelectionSizeInfo(unsigned long& fSize, unsigned long& dSize)
{
    fSize = 0;  // Files size
    dSize = 0;  // Estimated space requires to download and process files.
    for (IconItem* item = firstItem(); item; item = item->nextItem())
    {
        if (item->isSelected())
        {
            CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
            fSize += iconItem->itemInfo()->size;

            if (iconItem->itemInfo()->mime == QString("image/jpeg"))
            {
                if (d->cameraUI->convertLosslessJpegFiles())
                {
                    // Estimated size is around 5 x original size when JPEG=>PNG.
                    dSize += iconItem->itemInfo()->size*5;
                }
                else if (d->cameraUI->autoRotateJpegFiles())
                {
                    // We need a double size to perform rotation.
                    dSize += iconItem->itemInfo()->size*2;
                }
                else
                {
                    // Real file size is added.
                    dSize += iconItem->itemInfo()->size;
                }
            }
            else
                dSize += iconItem->itemInfo()->size;

        }
    }

    fSize /= 1024;
    dSize /= 1024;
}

}  // namespace Digikam
