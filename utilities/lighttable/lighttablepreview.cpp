/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : digiKam light table preview item.
 *
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


#include "lighttablepreview.h"
#include "lighttablepreview.moc"

// Qt includes.

#include <QList>
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

#include <kapplication.h>
#include <kcursor.h>
#include <kdatetable.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>
#include <krun.h>
#include <kservice.h>

// Local includes.

#include "dimg.h"
#include "albumdb.h"
#include "constants.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "ddragobjects.h"
#include "dmetadata.h"
#include "dpopupmenu.h"
#include "metadatahub.h"
#include "paniconwidget.h"
#include "previewloadthread.h"
#include "loadingdescription.h"
#include "tagspopupmenu.h"
#include "ratingpopupmenu.h"
#include "themeengine.h"

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
    d->cornerButton->setIcon(SmallIcon("transform-move"));
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

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
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
        pix.fill(ThemeEngine::instance()->baseColor());
        QPainter p(&pix);
        p.setPen(QPen(ThemeEngine::instance()->textRegColor()));
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
                          AlbumSettings::instance()->getExifRotate()));
    else
        d->previewThread->load(LoadingDescription(path, d->previewSize,
                          AlbumSettings::instance()->getExifRotate()));
}

void LightTablePreview::slotGotImagePreview(const LoadingDescription &description, const DImg& preview)
{
    if (description.filePath != d->path)
        return;

    if (preview.isNull())
    {
        QPixmap pix(visibleWidth(), visibleHeight());
        pix.fill(ThemeEngine::instance()->baseColor());
        QPainter p(&pix);
        QFileInfo info(d->path);
        p.setPen(QPen(ThemeEngine::instance()->textRegColor()));
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
        if (AlbumSettings::instance()->getExifRotate())
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
                                  AlbumSettings::instance()->getExifRotate()));
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

    QMap<QAction *, KService::Ptr> serviceMap;

    const KService::List offers = KMimeTypeTrader::self()->query(mimePtr->name());
    KService::List::ConstIterator iter;
    KService::Ptr ptr;

    QMenu openWithMenu;

    for( iter = offers.begin(); iter != offers.end(); ++iter )
    {
        ptr = *iter;
        QAction *serviceAction = openWithMenu.addAction(SmallIcon(ptr->icon()), ptr->name());
        serviceMap[serviceAction] = ptr;
    }

    if (openWithMenu.isEmpty())
        openWithMenu.menuAction()->setEnabled(false);

    DPopupMenu popmenu(this);

    //-- Zoom actions -----------------------------------------------

    QAction *zoomInAction    = popmenu.addAction(SmallIcon("zoom-in"), i18n("Zoom in"));
    QAction *zoomOutAction   = popmenu.addAction(SmallIcon("zoom-out"), i18n("Zoom out"));
    QAction *fitWindowAction = popmenu.addAction(SmallIcon("zoom-fit-best"), i18n("Fit to &Window"));

    //-- Edit actions -----------------------------------------------

    popmenu.addSeparator();
    QAction *slideshowAction = popmenu.addAction(SmallIcon("view-presentation"), i18n("SlideShow"));
    QAction *editAction = popmenu.addAction(SmallIcon("editimage"), i18n("Edit..."));
    popmenu.addMenu(&openWithMenu);
    openWithMenu.menuAction()->setText(i18n("Open With"));

    //-- Trash action -------------------------------------------

    popmenu.addSeparator();
    QAction *trashAction = popmenu.addAction(SmallIcon("user-trash"), i18n("Move to Trash"));

    // Bulk assignment/removal of tags --------------------------

    QList<qlonglong> idList;
    idList << d->imageInfo.id();

    assignTagsMenu = new TagsPopupMenu(idList, TagsPopupMenu::ASSIGN);
    removeTagsMenu = new TagsPopupMenu(idList, TagsPopupMenu::REMOVE);

    popmenu.addSeparator();

    popmenu.addMenu(assignTagsMenu);
    assignTagsMenu->menuAction()->setText(i18n("Assign Tag"));

    popmenu.addMenu(removeTagsMenu);
    removeTagsMenu->menuAction()->setText(i18n("Remove Tag"));

    connect(assignTagsMenu, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotAssignTag(int)));

    connect(removeTagsMenu, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotRemoveTag(int)));

    if (!DatabaseAccess().db()->hasTags(idList))
        removeTagsMenu->menuAction()->setEnabled(false);

    popmenu.addSeparator();

    // Assign Star Rating -------------------------------------------

    ratingMenu = new RatingPopupMenu();

    connect(ratingMenu, SIGNAL(signalRatingChanged(int)),
            this, SLOT(slotAssignRating(int)));

    popmenu.addMenu(ratingMenu);
    ratingMenu->menuAction()->setText(i18n("Assign Rating"));

    // --------------------------------------------------------

    QAction *choice = popmenu.exec(QCursor::pos());

    if (choice)
    {
        if (choice == editAction)               // Edit...
        {
            emit signalEditItem(d->imageInfo);
        }
        else if (choice == trashAction)         // Move to trash
        {
            emit signalDeleteItem(d->imageInfo);
        }
        else if (choice == slideshowAction)     // SlideShow
        {
            emit signalSlideShow();
        }
        else if (choice == zoomInAction)        // Zoom in
        {
            slotIncreaseZoom();
        }
        else if (choice == zoomOutAction)       // Zoom out
        {
            slotDecreaseZoom();
        }
        else if (choice == fitWindowAction)     // Fit to window
        {
            fitToWindow();
        }
        else if (serviceMap.contains(choice))
        {
            KService::Ptr imageServicePtr = serviceMap[choice];
            KRun::run(*imageServicePtr, url, this);
        }
    }

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
    QPalette palette;
    palette.setColor(backgroundRole(), ThemeEngine::instance()->baseColor());
    setPalette(palette);
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

    connect(pan, SIGNAL(signalSelectionMoved(const QRect&, bool)),
            this, SLOT(slotPanIconSelectionMoved(const QRect&, bool)));

    connect(pan, SIGNAL(signalHidden()),
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

void LightTablePreview::slotPanIconSelectionMoved(const QRect& r, bool b)
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
    // Set zoom for fit-in-window as minimum, but don't scale up images
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

void LightTablePreview::contentsDragEnterEvent(QDragEnterEvent *e)
{
    if (d->dragAndDropEnabled)
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
    }

    e->ignore();
}

void LightTablePreview::contentsDropEvent(QDropEvent *e)
{
    if (d->dragAndDropEnabled)
    {
        int           albumID;
        QList<int>    albumIDs;
        QList<int>    imageIDs;
        KUrl::List    urls;
        KUrl::List    kioURLs;
        ImageInfoList list;

        if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
        {
            for (QList<int>::const_iterator it = imageIDs.begin();
                 it != imageIDs.end(); ++it)
            {
                list << ImageInfo(*it);
            }

            emit signalDroppedItems(list);
            e->accept();
            return;
        }
        else if (DAlbumDrag::decode(e->mimeData(), urls, albumID))
        {
            QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInAlbum(albumID);

            for (QList<qlonglong>::const_iterator it = itemIDs.begin();
                it != itemIDs.end(); ++it)
            {
                list << ImageInfo(*it);
            }

            emit signalDroppedItems(list);
            e->accept();
            return;
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
        qDrawPlainRect(p, frameRect(), ThemeEngine::instance()->thumbSelColor(), lineWidth());
        qDrawPlainRect(p, frameRect(), ThemeEngine::instance()->textSelColor(), 2);
    }
    else
        qDrawPlainRect(p, frameRect(), ThemeEngine::instance()->baseColor(), lineWidth());
}

}  // namespace Digikam
