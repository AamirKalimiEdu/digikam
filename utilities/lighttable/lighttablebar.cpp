/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : light table thumbs bar
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes.

#include "lighttablebar.h"
#include "lighttablebar.moc"

// Qt includes.

#include <QList>
#include <QToolTip>
#include <QPixmap>
#include <QPainter>
#include <QImage>
#include <QCursor>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPolygon>
#include <QTextDocument>

// KDE includes.

#include <kdebug.h>
#include <kfileitem.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kstandarddirs.h>

// Digikam includes.

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "ddragobjects.h"
#include "imageattributeswatch.h"
#include "metadatahub.h"
#include "ratingpopupmenu.h"
#include "dpopupmenu.h"
#include "themeengine.h"

namespace Digikam
{

class LightTableBarPriv
{

public:

    LightTableBarPriv()
    {
        navigateByPair = false;
    }

    bool navigateByPair;
};

class LightTableBarItemPriv
{

public:

    LightTableBarItemPriv()
    {
        onLeftPanel  = false;
        onRightPanel = false;
    }

    bool onLeftPanel;
    bool onRightPanel;
};

LightTableBar::LightTableBar(QWidget* parent, int orientation, bool exifRotate)
             : ImagePreviewBar(parent, orientation, exifRotate)
{
    d = new LightTableBarPriv;

    // ----------------------------------------------------------------

    connect(this, SIGNAL(signalItemSelected(ThumbBarItem*)),
            this, SLOT(slotItemSelected(ThumbBarItem*)));
}

LightTableBar::~LightTableBar()
{
    delete d;
}

void LightTableBar::setNavigateByPair(bool b)
{
    d->navigateByPair = b;
}

void LightTableBar::contentsMouseReleaseEvent(QMouseEvent *e)
{
    if (!e) return;

    ThumbBarView::contentsMouseReleaseEvent(e);

    QPoint pos = QCursor::pos();
    LightTableBarItem *item = dynamic_cast<LightTableBarItem*>(findItemByPos(e->pos()));
    if (!item) return;

    RatingPopupMenu *ratingMenu = 0;

    if (e->button() == Qt::RightButton)
    {
        DPopupMenu popmenu(this);
        QAction *leftPanelAction  = popmenu.addAction(SmallIcon("arrow-left"), i18n("Show on left panel"));
        QAction *rightPanelAction = popmenu.addAction(SmallIcon("arrow-right"), i18n("Show on right panel"));
        QAction *editAction       = popmenu.addAction(SmallIcon("editimage"), i18n("Edit"));

        if (d->navigateByPair)
        {
            leftPanelAction->setEnabled(false);
            rightPanelAction->setEnabled(false);
        }

        popmenu.addSeparator();
        QAction *removeAction   = popmenu.addAction(SmallIcon("dialog-close"), i18n("Remove item"));
        QAction *clearAllAction = popmenu.addAction(SmallIcon("edit-delete-shred"), i18n("Clear all"));
        popmenu.addSeparator();

        // Assign Star Rating -------------------------------------------

        ratingMenu = new RatingPopupMenu();

        connect(ratingMenu, SIGNAL(signalRatingChanged(int)),
                this, SLOT(slotAssignRating(int)));

        popmenu.addMenu(ratingMenu);
        ratingMenu->menuAction()->setText(i18n("Assign Rating"));

        QAction *choice = popmenu.exec(pos);

        if (choice)
        {
            if (choice == leftPanelAction)          // Left panel
            {
                emit signalSetItemOnLeftPanel(item->info());
            }
            else if (choice == rightPanelAction)    // Right panel
            {
                emit signalSetItemOnRightPanel(item->info());
            }
            else if (choice == editAction)          // Edit
            {
                emit signalEditItem(item->info());
            }
            else if (choice == removeAction)        // Remove
            {
                emit signalRemoveItem(item->info());
            }
            else if (choice == clearAllAction)      // Clear All
            {
                emit signalClearAll();
            }
        }
    }

    delete ratingMenu;
}

void LightTableBar::slotAssignRating(int rating)
{
    rating = qMin(5, qMax(0, rating));
    ImageInfo info = currentItemImageInfo();
    if (!info.isNull())
    {
        MetadataHub hub;
        hub.load(info);
        hub.setRating(rating);
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTableBar::slotAssignRatingNoStar()
{
    slotAssignRating(0);
}

void LightTableBar::slotAssignRatingOneStar()
{
    slotAssignRating(1);
}

void LightTableBar::slotAssignRatingTwoStar()
{
    slotAssignRating(2);
}

void LightTableBar::slotAssignRatingThreeStar()
{
    slotAssignRating(3);
}

void LightTableBar::slotAssignRatingFourStar()
{
    slotAssignRating(4);
}

void LightTableBar::slotAssignRatingFiveStar()
{
    slotAssignRating(5);
}

void LightTableBar::setOnLeftPanel(const ImageInfo &info)
{
    for (ThumbBarItem *item = firstItem(); item; item = item->next())
    {
        LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(item);
        if (ltItem)
        {
            if (!info.isNull())
            {
                if (ltItem->info() == info)
                {
                    ltItem->setOnLeftPanel(true);
                    repaintItem(item);
                }
                else if (ltItem->isOnLeftPanel() == true)
                {
                    ltItem->setOnLeftPanel(false);
                    repaintItem(item);
                }
            }
            else if (ltItem->isOnLeftPanel() == true)
            {
                ltItem->setOnLeftPanel(false);
                repaintItem(item);
            }
        }
    }
}

void LightTableBar::setOnRightPanel(const ImageInfo &info)
{
    for (ThumbBarItem *item = firstItem(); item; item = item->next())
    {
        LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(item);
        if (ltItem)
        {
            if (!info.isNull())
            {
                if (ltItem->info() == info)
                {
                    ltItem->setOnRightPanel(true);
                    repaintItem(item);
                }
                else if (ltItem->isOnRightPanel() == true)
                {
                    ltItem->setOnRightPanel(false);
                    repaintItem(item);
                }
            }
            else if (ltItem->isOnRightPanel() == true)
            {
                ltItem->setOnRightPanel(false);
                repaintItem(item);
            }
        }
    }
}

void LightTableBar::slotItemSelected(ThumbBarItem* item)
{
    if (item)
    {
        LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(item);
        if (ltItem)
        {
            emit signalLightTableBarItemSelected(ltItem->info());
            return;
        }
    }

    emit signalLightTableBarItemSelected(ImageInfo());
}

void LightTableBar::removeItem(const ImageInfo &info)
{
    if (info.isNull()) return;

    ImagePreviewBarItem* ltItem = findItemByInfo(info);
    ThumbBarItem *item          = dynamic_cast<ThumbBarItem*>(ltItem);
    if (item) ThumbBarView::removeItem(item);
}

void LightTableBar::viewportPaintEvent(QPaintEvent* e)
{
    ThemeEngine* te = ThemeEngine::instance();
    QRect    er(e->rect());
    QPixmap  bgPix;

    if (countItems() > 0)
    {
        int cy=0, cx=0, ts=0, y1=0, y2=0, x1=0, x2=0;
        QPixmap  tile;

        if (getOrientation() == Qt::Vertical)
        {
            cy = viewportToContents(er.topLeft()).y();

            bgPix = QPixmap(contentsRect().width(), er.height());

            ts   = getTileSize() + 2*getMargin();
            tile = QPixmap(visibleWidth(), ts);

            y1 = (cy/ts)*ts;
            y2 = ((y1 + er.height())/ts +1)*ts;
        }
        else
        {
            cx = viewportToContents(er.topLeft()).x();

            bgPix = QPixmap(er.width(), contentsRect().height());

            ts   = getTileSize() + 2*getMargin();
            tile = QPixmap(ts, visibleHeight());

            x1 = (cx/ts)*ts;
            x2 = ((x1 + er.width())/ts +1)*ts;
        }

        bgPix.fill(te->baseColor());

        for (ThumbBarItem *item = firstItem(); item; item = item->next())
        {
            if (getOrientation() == Qt::Vertical)
            {
                if (y1 <= item->position() && item->position() <= y2)
                {
                    if (item == currentItem())
                        tile = te->thumbSelPixmap(tile.width(), tile.height());
                    else
                        tile = te->thumbRegPixmap(tile.width(), tile.height());

                    QPainter p(&tile);

                    if (item == currentItem())
                    {
                        p.setPen(QPen(te->textSelColor(), 3));
                        p.drawRect(2, 2, tile.width()-2, tile.height()-2);
                    }
                    else
                    {
                        p.setPen(QPen(te->textRegColor(), 1));
                        p.drawRect(0, 0, tile.width(), tile.height());
                    }

                    QPixmap pix;
                    if (pixmapForItem(item, pix))
                    {
                        int x = (tile.width()  - pix.width())/2;
                        int y = (tile.height() - pix.height())/2;

                        p.drawPixmap(x, y, pix);

                        LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(item);

                        if (ltItem->isOnLeftPanel())
                        {
                            QPixmap lPix = SmallIcon("arrow-left");
                            p.drawPixmap(getMargin(), getMargin(), lPix);
                        }
                        if (ltItem->isOnRightPanel())
                        {
                            QPixmap rPix = SmallIcon("arrow-right");
                            p.drawPixmap(tile.width() - getMargin() - rPix.width(), getMargin(), rPix);
                        }

                        QRect r(0, tile.height()-getMargin()-ratingPixmap().height(),
                                tile.width(), ratingPixmap().height());
                        int rating = ltItem->info().rating();
                        int xr     = (r.width() - rating * ratingPixmap().width())/2;
                        int wr     = rating * ratingPixmap().width();
                        p.drawTiledPixmap(xr, r.y(), wr, r.height(), ratingPixmap());
                    }

                    p.end();

                    QPainter p2(&bgPix);
                    p2.drawPixmap(0, item->position() - cy, tile);
                    p2.end();
                }
            }
            else
            {
                if (x1 <= item->position() && item->position() <= x2)
                {
                    if (item == currentItem())
                        tile = te->thumbSelPixmap(tile.width(), tile.height());
                    else
                        tile = te->thumbRegPixmap(tile.width(), tile.height());

                    QPainter p(&tile);

                    if (item == currentItem())
                    {
                        p.setPen(QPen(te->textSelColor(), 2));
                        p.drawRect(1, 1, tile.width()-1, tile.height()-1);
                    }
                    else
                    {
                        p.setPen(QPen(te->textRegColor(), 1));
                        p.drawRect(0, 0, tile.width(), tile.height());
                    }

                    QPixmap pix;
                    if (pixmapForItem(item, pix))
                    {
                        int x = (tile.width() - pix.width())/2;
                        int y = (tile.height()- pix.height())/2;
                        p.drawPixmap(x, y, pix);

                        LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(item);

                        if (ltItem->isOnLeftPanel())
                        {
                            QPixmap lPix = SmallIcon("arrow-left");
                            p.drawPixmap(getMargin(), getMargin(), lPix);
                        }
                        if (ltItem->isOnRightPanel())
                        {
                            QPixmap rPix = SmallIcon("arrow-right");
                            p.drawPixmap(tile.width() - getMargin() - rPix.width(), getMargin(), rPix);
                        }

                        QRect r(0, tile.height()-getMargin()-ratingPixmap().height(),
                                tile.width(), ratingPixmap().height());
                        int rating = ltItem->info().rating();
                        int xr     = (r.width() - rating * ratingPixmap().width())/2;
                        int wr     = rating * ratingPixmap().width();
                        p.drawTiledPixmap(xr, r.y(), wr, r.height(), ratingPixmap());
                    }

                    p.end();

                    QPainter p2(&bgPix);
                    p2.drawPixmap(item->position() - cx, 0, tile);
                    p2.end();
                }
            }
        }

        QPainter p3(viewport());

        if (getOrientation() == Qt::Vertical)
            p3.drawPixmap(0, er.y(), bgPix);
        else
            p3.drawPixmap(er.x(), 0, bgPix);

        p3.end();
    }
    else
    {
        bgPix = QPixmap(contentsRect().width(), contentsRect().height());
        bgPix.fill(te->baseColor());

        QPainter p4(&bgPix);
        p4.setPen(QPen(te->textRegColor()));
        p4.drawText(0, 0, bgPix.width(), bgPix.height(),
                    Qt::AlignCenter|Qt::TextWordWrap,
                    i18n("Drag and drop images here"));
        p4.end();

        QPainter p5(viewport());
        p5.drawPixmap(0, 0, bgPix);
        p5.end();
    }

    checkPreload();
}

void LightTableBar::startDrag()
{
    if (!currentItem()) return;

    KUrl::List urls;
    KUrl::List kioURLs;
    QList<int> albumIDs;
    QList<int> imageIDs;

    LightTableBarItem *item = dynamic_cast<LightTableBarItem*>(currentItem());

    urls.append(item->info().fileUrl());
    kioURLs.append(item->info().databaseUrl());
    imageIDs.append(item->info().id());
    albumIDs.append(item->info().albumId());

    QPixmap icon(DesktopIcon("image-jpeg2000", 48));
    int w = icon.width();
    int h = icon.height();

    QPixmap pix(w+4,h+4);
    QPainter p(&pix);
    p.fillRect(0, 0, w+4, h+4, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, w+4, h+4);
    p.drawPixmap(2, 2, icon);
    p.end();

    QDrag *drag = new QDrag(this);
    drag->setMimeData(new DItemDrag(urls, kioURLs, albumIDs, imageIDs));
    drag->setPixmap(pix);
    drag->exec();
}

void LightTableBar::contentsDragMoveEvent(QDragMoveEvent *e)
{
    int        albumID;
    QList<int> albumIDs;
    QList<int> imageIDs;
    KUrl::List urls;
    KUrl::List kioURLs;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs) ||
        DAlbumDrag::decode(e->mimeData(), urls, albumID) ||
        DTagDrag::canDecode(e->mimeData()))
    {
        e->accept();
        return;
    }

    e->ignore();
}

void LightTableBar::contentsDropEvent(QDropEvent *e)
{
    int        albumID;
    QList<int> albumIDs;
    QList<int> imageIDs;
    KUrl::List urls;
    KUrl::List kioURLs;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
    {
        ImageInfoList imageInfoList;

        for (QList<int>::const_iterator it = imageIDs.begin();
             it != imageIDs.end(); ++it)
        {
            ImageInfo info(*it);
            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
        }

        emit signalDroppedItems(imageInfoList);
        e->accept();
    }
    else if (DAlbumDrag::decode(e->mimeData(), urls, albumID))
    {
        QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInAlbum(albumID);
        ImageInfoList imageInfoList;

        for (QList<qlonglong>::const_iterator it = itemIDs.begin();
             it != itemIDs.end(); ++it)
        {
            ImageInfo info(*it);
            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
        }

        emit signalDroppedItems(imageInfoList);
        e->accept();
    }
    else if(DTagDrag::canDecode(e->mimeData()))
    {
        int tagID;
        if (!DTagDrag::decode(e->mimeData(), tagID))
            return;

        QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInTag(tagID, true);
        ImageInfoList imageInfoList;

        for (QList<qlonglong>::const_iterator it = itemIDs.begin();
             it != itemIDs.end(); ++it)
        {
            ImageInfo info(*it);
            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
        }

        emit signalDroppedItems(imageInfoList);
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

// -------------------------------------------------------------------------

LightTableBarItem::LightTableBarItem(LightTableBar *view, const ImageInfo &info)
                 : ImagePreviewBarItem(view, info.fileUrl())
{
    d = new LightTableBarItemPriv;
}

LightTableBarItem::~LightTableBarItem()
{
    delete d;
}

void LightTableBarItem::setOnLeftPanel(bool on)
{
    d->onLeftPanel = on;
}

void LightTableBarItem::setOnRightPanel(bool on)
{
    d->onRightPanel = on;
}

bool LightTableBarItem::isOnLeftPanel() const
{
    return d->onLeftPanel;
}

bool LightTableBarItem::isOnRightPanel() const
{
    return d->onRightPanel;
}

}  // namespace Digikam
