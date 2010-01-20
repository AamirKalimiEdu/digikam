/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded view to show the image preview widget.
 *
 * Copyright (C) 2006-2009 Gilles Caulier  <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef IMAGEPREVIEWVIEW_H
#define IMAGEPREVIEWVIEW_H

// Qt includes

#include <QImage>
#include <QResizeEvent>
#include <QString>

// Local includes

#include "dimg.h"
#include "imageinfo.h"
#include "previewwidget.h"
#include "digikam_export.h"

class QPixmap;

namespace Digikam
{

class AlbumWidgetStack;
class ImagePreviewViewPriv;
class LoadingDescription;

class ImagePreviewView : public PreviewWidget
{
    Q_OBJECT

public:

    ImagePreviewView(QWidget *parent, AlbumWidgetStack *stack);
    ~ImagePreviewView();

    void setLoadFullImageSize(bool b);

    void setImage(const DImg& image);
    DImg& getImage() const;

    void setImageInfo(const ImageInfo& info = ImageInfo(),
                      const ImageInfo& previous = ImageInfo(),
                      const ImageInfo& next = ImageInfo());

    ImageInfo getImageInfo() const;

    void reload();
    void setImagePath(const QString& path=QString());
    void setPreviousNextPaths(const QString& previous, const QString& next);

Q_SIGNALS:

    void signalNextItem();
    void signalPrevItem();
    void signalDeleteItem();
    void signalEditItem();
    void signalPreviewLoaded(bool success);
    void signalBack2Album();
    void signalSlideShow();
    void signalInsert2LightTable();
    void signalInsert2QueueMgr();
    void signalFindSimilar();
    void signalAddToExistingQueue(int);

    void signalGotoAlbumAndItem(const ImageInfo&);
    void signalGotoDateAndItem(const ImageInfo&);
    void signalGotoTagAndItem(int);

protected:

    void resizeEvent(QResizeEvent* e);

private Q_SLOTS:

    void slotGotImagePreview(const LoadingDescription& loadingDescription, const DImg& image);
    void slotNextPreload();
    void slotContextMenu();
    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);
    void slotAssignRating(int rating);
    void slotThemeChanged();
    void slotDeleteItem();
    void slotGotoTag(int tagID);

private:

    int    previewWidth();
    int    previewHeight();
    bool   previewIsNull();
    void   resetPreview();
    QImage previewToQImage() const;

    inline void paintPreview(QPixmap *pix, int sx, int sy, int sw, int sh);

private:

    ImagePreviewViewPriv* const d;
};

}  // namespace Digikam

#endif /* IMAGEPREVIEWVIEW_H */
