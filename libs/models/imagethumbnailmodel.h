/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries with support for thumbnail loading
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGETHUMBNAILMODEL_H
#define IMAGETHUMBNAILMODEL_H

// Local includes

#include "imagemodel.h"
#include "thumbnailsize.h"
#include "digikam_export.h"

namespace Digikam
{

class LoadingDescription;
class ThumbnailLoadThread;
class ImageThumbnailModelPriv;

class DIGIKAM_DATABASE_EXPORT ImageThumbnailModel : public ImageModel
{
    Q_OBJECT

public:

    /** An ImageModel that supports thumbnail loading.
     *  You need to set a ThumbnailLoadThread to enable thumbnail loading.
     *  Adjust the thumbnail size to your needs.
     *  Note that setKeepsFilePathCache is enabled per default.
     */

    ImageThumbnailModel(QObject *parent);
    ~ImageThumbnailModel();

    /** Enable thumbnail loading and set the thread that shall be used.
     *  The thumbnail size of this thread will be adjusted. */
    void setThumbnailLoadThread(ThumbnailLoadThread *thread);
    /// Set the thumbnail size to use
    void setThumbnailSize(const ThumbnailSize& thumbSize);
    /** Enable emitting dataChanged() when a thumbnail becomes available.
     *  The thumbnailAvailable() signal will be emitted in any case.
     *  Default is true.
     */
    void setEmitDataChanged(bool emitSignal);

    ThumbnailSize thumbnailSize() const;

    /** Prepare the thumbnail loading for the given indexes */
    void prepareThumbnails(const QList<QModelIndex>& indexesToPrepare);

    /** Handles the ThumbnailRole.
     *  If the pixmap is available, returns it in the QVariant.
     *  If it still needs to be loaded, returns a null QVariant and emits
     *  thumbnailAvailable() as soon as it is available.
     */
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    /**
     * You can override the current thumbnail size by giving an integer value for ThumbnailRole.
     * Set a null QVariant to use the thumbnail size set by setThumbnailSize() again.
     * The index given here is ignored for this purpose.
     */
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole);

Q_SIGNALS:

    void thumbnailAvailable(const QModelIndex& index, int requestedSize);

protected Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb);

private:

    ImageThumbnailModelPriv* const d;
};

} // namespace Digikam

#endif /* IMAGETHUMBNAILMODEL_H */
