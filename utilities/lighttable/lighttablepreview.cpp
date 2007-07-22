/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam light table preview item.
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <Q3ValueList>
#include <Q3ValueVector>
#include <Q3PopupMenu>
#include <QPainter>
#include <QCursor>
#include <QString>
#include <QFileInfo>
#include <QToolButton>
#include <QPixmap>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QResizeEvent>
#include <QDesktopWidget>

// KDE includes.

#include <kservicetypetrader.h>
#include <kdialog.h>
#include <klocale.h>
#include <kservice.h>
#include <krun.h>
#include <kmimetype.h>
#include <kcursor.h>
#include <kdatetable.h>
#include <kiconloader.h>
#include <kapplication.h>

// Local includes.

#include "dimg.h"
#include "ddebug.h"
#include "albumdb.h"
#include "constants.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "dragobjects.h"
#include "dmetadata.h"
#include "dpopupmenu.h"
#include "metadatahub.h"
#include "paniconwidget.h"
#include "previewloadthread.h"
#include "loadingdescription.h"
#include "tagspopupmenu.h"
#include "ratingpopupmenu.h"
#include "themeengine.h"
#include "lighttablepreview.h"
#include "lighttablepreview.moc"

namespace Digikam
{

class LightTablePreviewPriv
{
public:

    LightTablePreviewPriv()
    {
        panIconPopup         = 0;
        panIconWidget        = 0;
        cornerButton         = 0;
        previewThread        = 0;
        previewPreloadThread = 0;
        hasPrev              = false;
        hasNext              = false;
        selected             = false;
        dragAndDropEnabled   = true;
        loadFullImageSize    = false;
        currentFitWindowZoom = 0;
        previewSize          = 1024;
    }

    bool               hasPrev;
    bool               hasNext;
    bool               selected;
    bool               dragAndDropEnabled;
    bool               loadFullImageSize;

    int                previewSize;

    double             currentFitWindowZoom;

    QString            path;
    QString            nextPath;
    QString            previousPath;

    QToolButton       *cornerButton;

    KPopupFrame       *panIconPopup;

    PanIconWidget     *panIconWidget;

    DImg               preview;

    ImageInfo          imageInfo;

    PreviewLoadThread *previewThread;
    PreviewLoadThread *previewPreloadThread;
};

LightTablePreview::LightTablePreview(QWidget *parent)
                 : PreviewWidget(parent)
{
    d = new LightTablePreviewPriv;

    // get preview size from screen size, but limit from VGA to WQXGA
    d->previewSize = qMax(KApplication::desktop()->height(),
                          KApplication::desktop()->width());
    if (d->previewSize < 640)
        d->previewSize = 640;
    if (d->previewSize > 2560)
        d->previewSize = 2560;

    viewport()->setAcceptDrops(true);
    setAcceptDrops(true); 

    slotThemeChanged();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->cornerButton = new QToolButton(this);
    d->cornerButton->setIcon(SmallIcon("move"));
    d->cornerButton->hide();
    d->cornerButton->setToolTip( i18n("Pan the image"));
    setCornerWidget(d->cornerButton);

    setLineWidth(5);
    setSelected(false);

    // ------------------------------------------------------------

    connect(d->cornerButton, SIGNAL(pressed()),
            this, SLOT(slotCornerButtonPressed()));

    connect(this, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));

    connect(ThemeEngine::componentData(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // ------------------------------------------------------------

    slotReset(); 
}

LightTablePreview::~LightTablePreview()
{
    delete d->previewThread;
    delete d->previewPreloadThread;
    delete d;
}

void LightTablePreview::setLoadFullImageSize(bool b)
{
    d->loadFullImageSize = b;
    reload();
}

void LightTablePreview::setDragAndDropEnabled(bool b)
{
    d->dragAndDropEnabled = b;
}

void LightTablePreview::setDragAndDropMessage()
{
    if (d->dragAndDropEnabled)
    {
        QPixmap pix(visibleWidth(), visibleHeight());
        pix.fill(ThemeEngine::componentData()->baseColor());
        QPainter p(&pix);
        p.setPen(QPen(ThemeEngine::componentData()->textRegColor()));
        p.drawText(0, 0, pix.width(), pix.height(),
                   Qt::AlignCenter|Qt::TextWordWrap,
                   i18n("Drag and drop an image here"));
        p.end();
        setImage(pix.toImage());
    }
}

void LightTablePreview::setImage(const DImg& image)
{
    d->preview = image;

    updateZoomAndSize(true);

    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

DImg& LightTablePreview::getImage() const
{
    return d->preview;
}

QSize LightTablePreview::getImageSize()
{
    return d->preview.size();
}

void LightTablePreview::reload()
{
    // cache is cleaned from AlbumIconView::refreshItems
    setImagePath(d->path);
}

void LightTablePreview::setPreviousNextPaths(const QString& previous, const QString &next)
{
    d->nextPath     = next;
    d->previousPath = previous;
}

void LightTablePreview::setImagePath(const QString& path)
{
    setCursor( Qt::WaitCursor );

    d->path         = path;
    d->nextPath     = QString();
    d->previousPath = QString();

    if (d->path.isEmpty())
    {
        slotReset();
        unsetCursor();
        return;
    }

    if (!d->previewThread)
    {
        d->previewThread = new PreviewLoadThread();
        connect(d->previewThread, SIGNAL(signalImageLoaded(const LoadingDescription &, const DImg &)),
                this, SLOT(slotGotImagePreview(const LoadingDescription &, const DImg&)));
    }
    if (!d->previewPreloadThread)
    {
        d->previewPreloadThread = new PreviewLoadThread();
        connect(d->previewPreloadThread, SIGNAL(signalImageLoaded(const LoadingDescription &, const DImg &)),
                this, SLOT(slotNextPreload()));
    }

    if (d->loadFullImageSize)
        d->previewThread->loadHighQuality(LoadingDescription(path, 0,
                          AlbumSettings::componentData()->getExifRotate()));
    else
        d->previewThread->load(LoadingDescription(path, d->previewSize,
                          AlbumSettings::componentData()->getExifRotate()));
}

void LightTablePreview::slotGotImagePreview(const LoadingDescription &description, const DImg& preview)
{
    if (description.filePath != d->path)
        return;

    if (preview.isNull())
    {
        QPixmap pix(visibleWidth(), visibleHeight());
        pix.fill(ThemeEngine::componentData()->baseColor());
        QPainter p(&pix);
        QFileInfo info(d->path);
        p.setPen(QPen(ThemeEngine::componentData()->textRegColor()));
        p.drawText(0, 0, pix.width(), pix.height(),
                   Qt::AlignCenter|Qt::TextWordWrap, 
                   i18n("Unable to display preview for\n\"%1\"",
                   info.fileName()));
        p.end();
        setImage(DImg(pix.toImage()));

        emit signalPreviewLoaded(false);
    }
    else
    {
        DImg img(preview);
        if (AlbumSettings::componentData()->getExifRotate())
            d->previewThread->exifRotate(img, description.filePath);
        setImage(img);
        emit signalPreviewLoaded(true);
    }

    unsetCursor();
    slotNextPreload();
}

void LightTablePreview::slotNextPreload()
{
    QString loadPath;
    if (!d->nextPath.isNull())
    {
        loadPath    = d->nextPath;
        d->nextPath = QString();
    }
    else if (!d->previousPath.isNull())
    {
        loadPath        = d->previousPath;
        d->previousPath = QString();
    }
    else
        return;

    d->previewPreloadThread->load(LoadingDescription(loadPath, d->previewSize,
                                  AlbumSettings::componentData()->getExifRotate()));
}

void LightTablePreview::setImageInfo(const ImageInfo &info, const ImageInfo &previous, const ImageInfo &next)
{
    d->imageInfo = info;
    d->hasPrev   = !previous.isNull();
    d->hasNext   = !next.isNull();

    if (!d->imageInfo.isNull())
        setImagePath(info.filePath());
    else
    {
        setImagePath();
        setSelected(false);
    }

    setPreviousNextPaths(previous.isNull() ? QString() : previous.filePath(),
                         next.isNull()     ? QString() : next.filePath());
}

ImageInfo LightTablePreview::getImageInfo() const
{
    return d->imageInfo;
}

void LightTablePreview::slotContextMenu()
{
    RatingPopupMenu *ratingMenu     = 0;
    TagsPopupMenu   *assignTagsMenu = 0;
    TagsPopupMenu   *removeTagsMenu = 0;

    if (d->imageInfo.isNull())
        return;

    //-- Open With Actions ------------------------------------

    KUrl url(d->imageInfo.fileUrl().path());
    KMimeType::Ptr mimePtr = KMimeType::findByUrl(url, 0, true, true);

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

    DPopupMenu popmenu(this);

    //-- Zoom actions -----------------------------------------------

    popmenu.insertItem(SmallIcon("viewmag"), i18n("Zoom in"), 17);
    popmenu.insertItem(SmallIcon("viewmag-"), i18n("Zoom out"), 18);
    popmenu.insertItem(SmallIcon("view_fit_window"), i18n("Fit to &Window"), 19);

    //-- Edit actions -----------------------------------------------

    popmenu.insertSeparator();
    popmenu.insertItem(SmallIcon("slideshow"), i18n("SlideShow"), 16);
    popmenu.insertItem(SmallIcon("editimage"), i18n("Edit..."), 12);
    popmenu.insertItem(i18n("Open With"), &openWithMenu, 13);

    //-- Trash action -------------------------------------------

    popmenu.insertSeparator();
    popmenu.insertItem(SmallIcon("edittrash"), i18n("Move to Trash"), 14);

    // Bulk assignment/removal of tags --------------------------

    qlonglong id = d->imageInfo.id();
    QList<qlonglong> idList;
    idList << d->imageInfo.id();

    assignTagsMenu = new TagsPopupMenu(idList, TagsPopupMenu::ASSIGN);
    removeTagsMenu = new TagsPopupMenu(idList, TagsPopupMenu::REMOVE);

    popmenu.insertSeparator();

    popmenu.insertItem(i18n("Assign Tag"), assignTagsMenu);
    int i = popmenu.insertItem(i18n("Remove Tag"), removeTagsMenu);

    connect(assignTagsMenu, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotAssignTag(int)));

    connect(removeTagsMenu, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotRemoveTag(int)));

    if (!DatabaseAccess().db()->hasTags(idList))
        popmenu.setItemEnabled(i, false);

    popmenu.insertSeparator();

    // Assign Star Rating -------------------------------------------

    ratingMenu = new RatingPopupMenu();

    connect(ratingMenu, SIGNAL(activated(int)),
            this, SLOT(slotAssignRating(int)));

    popmenu.insertItem(i18n("Assign Rating"), ratingMenu);

    // --------------------------------------------------------

    int idm = popmenu.exec(QCursor::pos());

    switch(idm) 
    {
        case 12:     // Edit...
        {
            emit signalEditItem(d->imageInfo);
            break;
        }

        case 14:     // Move to trash
        {
            emit signalDeleteItem(d->imageInfo);
            break;
        }

        case 16:     // SlideShow
        {
            emit signalSlideShow();
            break;
        }

        case 17:     // Zoom in
        {
            slotIncreaseZoom();
            break;
        }

        case 18:     // Zoom out
        {
            slotDecreaseZoom();
            break;
        }

        case 19:     // Fit to window
        {
            fitToWindow();
            break;
        }

        default:
            break;
    }

    // Open With...
    if (idm >= 100 && idm < 1000) 
    {
        KService::Ptr imageServicePtr = serviceVector[idm-100];
        KRun::run(*imageServicePtr, url, this);
    }

    serviceVector.clear();
    delete assignTagsMenu;
    delete removeTagsMenu;
    delete ratingMenu;
}

void LightTablePreview::slotAssignTag(int tagID)
{
    if (!d->imageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setTag(tagID, true);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTablePreview::slotRemoveTag(int tagID)
{
    if (!d->imageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setTag(tagID, false);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTablePreview::slotAssignRating(int rating)
{
    rating = qMin(RatingMax, qMax(RatingMin, rating));
    if (!d->imageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setRating(rating);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTablePreview::slotThemeChanged()
{
    setBackgroundColor(ThemeEngine::componentData()->baseColor());
    frameChanged();
}

void LightTablePreview::slotCornerButtonPressed()
{
    if (d->panIconPopup)
    {
        d->panIconPopup->hide();
        delete d->panIconPopup;
        d->panIconPopup = 0;
    }

    d->panIconPopup    = new KPopupFrame(this);
    PanIconWidget *pan = new PanIconWidget(d->panIconPopup);
    pan->setImage(180, 120, getImage());
    d->panIconPopup->setMainWidget(pan);

    QRect r((int)(contentsX()    / zoomFactor()), (int)(contentsY()     / zoomFactor()),
            (int)(visibleWidth() / zoomFactor()), (int)(visibleHeight() / zoomFactor()));
    pan->setRegionSelection(r);
    pan->setMouseFocus();

    connect(pan, SIGNAL(signalSelectionMoved(QRect, bool)),
            this, SLOT(slotPanIconSelectionMoved(QRect, bool)));

    connect(pan, SIGNAL(signalHiden()),
            this, SLOT(slotPanIconHiden()));

    QPoint g = mapToGlobal(viewport()->pos());
    g.setX(g.x()+ viewport()->size().width());
    g.setY(g.y()+ viewport()->size().height());
    d->panIconPopup->popup(QPoint(g.x() - d->panIconPopup->width(), 
                                  g.y() - d->panIconPopup->height()));

    pan->setCursorToLocalRegionSelectionCenter();
}

void LightTablePreview::slotPanIconHiden()
{
    d->cornerButton->blockSignals(true);
    d->cornerButton->animateClick();
    d->cornerButton->blockSignals(false);
}

void LightTablePreview::slotPanIconSelectionMoved(QRect r, bool b)
{
    setContentsPos((int)(r.x()*zoomFactor()), (int)(r.y()*zoomFactor()));

    if (b)
    {
        d->panIconPopup->hide();
        delete d->panIconPopup;
        d->panIconPopup = 0;
        slotPanIconHiden();
    }
}

void LightTablePreview::zoomFactorChanged(double zoom)
{
    updateScrollBars();

    if (horizontalScrollBar()->isVisible() || verticalScrollBar()->isVisible())
        d->cornerButton->show();
    else
        d->cornerButton->hide();        

    PreviewWidget::zoomFactorChanged(zoom);
}

void LightTablePreview::resizeEvent(QResizeEvent* e)
{
    if (!e) return;

    Q3ScrollView::resizeEvent(e);

    if (d->imageInfo.isNull())
    {
        d->cornerButton->hide();
        setDragAndDropMessage();
    }

    updateZoomAndSize(false);
}

void LightTablePreview::updateZoomAndSize(bool alwaysFitToWindow)
{
    // Set zoom for fit-in-window as minimum, but dont scale up images
    // that are smaller than the available space, only scale down.
    double zoom = calcAutoZoomFactor(ZoomInOnly);
    setZoomMin(zoom);
    setZoomMax(zoom*12.0);

    // Is currently the zoom factor set to fit to window? Then set it again to fit the new size.
    if (zoomFactor() < zoom || alwaysFitToWindow || zoomFactor() == d->currentFitWindowZoom)
    {
        setZoomFactor(zoom);
    }

    // store which zoom factor means it is fit to window
    d->currentFitWindowZoom = zoom;

    updateContentsSize();
}

int LightTablePreview::previewWidth()
{
    return d->preview.width();
}

int LightTablePreview::previewHeight()
{
    return d->preview.height();
}

bool LightTablePreview::previewIsNull()
{
    return d->preview.isNull();
}

void LightTablePreview::resetPreview()
{
    d->preview   = DImg();
    d->path      = QString();
    d->imageInfo = ImageInfo();

    setDragAndDropMessage();
    updateZoomAndSize(true);
    viewport()->setUpdatesEnabled(true);
    viewport()->update();
    emit signalPreviewLoaded(false);
}

void LightTablePreview::paintPreview(QPixmap *pix, int sx, int sy, int sw, int sh)
{
    DImg img     = d->preview.smoothScaleSection(sx, sy, sw, sh, tileSize(), tileSize());
    QPixmap pix2 = img.convertToPixmap();
    QPainter p(pix);
    p.drawPixmap(0, 0, pix2, 0, 0, pix2.width(), pix2.height());
    p.end();
}

void LightTablePreview::contentsDragMoveEvent(QDragMoveEvent *e)
{
    if (d->dragAndDropEnabled)
    {
        int             albumID;
        Q3ValueList<int> albumIDs;
        Q3ValueList<int> imageIDs;
        KUrl::List      urls;
        KUrl::List      kioURLs;

        if (ItemDrag::decode(e, urls, kioURLs, albumIDs, imageIDs) ||
            AlbumDrag::decode(e, urls, albumID) ||
            TagDrag::canDecode(e))
        {
            e->accept();
            return;
        }
    }

    e->ignore();
}

void LightTablePreview::contentsDropEvent(QDropEvent *e)
{
    if (d->dragAndDropEnabled)
    {
        int             albumID;
        Q3ValueList<int> albumIDs;
        Q3ValueList<int> imageIDs;
        KUrl::List      urls;
        KUrl::List      kioURLs;  
        ImageInfoList   list;

        if (ItemDrag::decode(e, urls, kioURLs, albumIDs, imageIDs))
        {
            for (Q3ValueList<int>::const_iterator it = imageIDs.begin();
                 it != imageIDs.end(); ++it)
            {
                list << ImageInfo(*it);
            }

            emit signalDroppedItems(list);
            e->accept();
            return;
        }
        else if (AlbumDrag::decode(e, urls, albumID))
        {
            Q3ValueList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInAlbum(albumID);

            for (Q3ValueList<qlonglong>::const_iterator it = itemIDs.begin();
                it != itemIDs.end(); ++it)
            {
                list << ImageInfo(*it);
            }

            emit signalDroppedItems(list);
            e->accept();
            return;
        }
        else if(TagDrag::canDecode(e))
        {
            QByteArray  ba = e->encodedData("digikam/tag-id");
            QDataStream ds(ba);
            int tagID;
            ds >> tagID;

            Q3ValueList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInTag(tagID, true);
            ImageInfoList imageInfoList;

            for (Q3ValueList<qlonglong>::const_iterator it = itemIDs.begin();
                it != itemIDs.end(); ++it)
            {
                list << ImageInfo(*it);
            }

            emit signalDroppedItems(list);
            e->accept();
            return;
        }
    }

    e->ignore();
}

void LightTablePreview::setSelected(bool sel)
{
    if (d->selected != sel)
    {
        d->selected = sel;
        frameChanged();
    }
}

bool LightTablePreview::isSelected()
{
    return d->selected;
}

void LightTablePreview::drawFrame(QPainter *p)
{
    if (d->selected)
    {
        qDrawPlainRect(p, frameRect(), ThemeEngine::componentData()->thumbSelColor(), lineWidth());
        qDrawPlainRect(p, frameRect(), ThemeEngine::componentData()->textSelColor(), 2);
    }
    else 
        qDrawPlainRect(p, frameRect(), ThemeEngine::componentData()->baseColor(), lineWidth());
}

}  // NameSpace Digikam
