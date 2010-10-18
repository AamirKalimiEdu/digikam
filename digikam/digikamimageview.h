/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : Qt item view for images
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAMIMAGEVIEW_H
#define DIGIKAMIMAGEVIEW_H

// Local includes

#include "imagecategorizedview.h"

class QMimeData;

namespace Digikam
{

class ImageViewUtilities;
class DigikamImageViewPriv;

class DigikamImageView : public ImageCategorizedView
{
    Q_OBJECT

public:

    DigikamImageView(QWidget *parent = 0);
    ~DigikamImageView();

    ImageViewUtilities *utilities() const;

    virtual void setThumbnailSize(const ThumbnailSize& size);

public Q_SLOTS:

    void openInEditor(const ImageInfo& info);
    void openCurrentInEditor();

    void insertSelectedToLightTable(bool addTo);
    void insertSelectedToCurrentQueue();
    void insertSelectedToNewQueue();
    void insertSelectedToExistingQueue(int queueid);

    void deleteSelected(bool permanently = false);
    void deleteSelectedDirectly(bool permanently = false);
    void assignTagToSelected(int tagID);
    void removeTagFromSelected(int tagID);
    void assignRatingToSelected(int rating);
    void setAsAlbumThumbnail(const ImageInfo& setAsThumbnail);
    void createNewAlbumForSelected();
    void setExifOrientationOfSelected(int orientation);
    void rename();

    void assignRating(const QModelIndex &index, int rating);
    void assignTag(const QModelIndex& index, const QString& name);

    void setFaceMode(bool on);
    void addRejectionOverlay(ImageDelegate *delegate = 0);
    void addTagEditOverlay(ImageDelegate* delegate = 0);
    void addAssignNameOverlay(ImageDelegate* delegate = 0);

Q_SIGNALS:

    void previewRequested(const ImageInfo& info);
    void gotoAlbumAndImageRequested(const ImageInfo& info);
    void gotoTagAndImageRequested(int tagId);
    void gotoDateAndImageRequested(const ImageInfo& info);

protected:

    virtual void activated(const ImageInfo& info);
    virtual void showContextMenuOnInfo(QContextMenuEvent* event, const ImageInfo& info);
    virtual void showContextMenu(QContextMenuEvent* event);
    virtual void slotSetupChanged();

private Q_SLOTS:

    void slotRotateLeft();
    void slotRotateRight();
    void slotUntagFace(const QModelIndex& index);

private:

    DigikamImageViewPriv* const d;
};

} // namespace Digikam

#endif /* DIGIKAMIMAGEVIEW_H */
