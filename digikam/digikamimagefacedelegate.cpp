/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2002-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#include "digikamimagefacedelegate.moc"
#include "imagedelegatepriv.h"

// Qt includes

// KDE includes

#include <kdebug.h>

// Local includes

#include "albumsettings.h"
#include "faceiface.h"
#include "imagemodel.h"
#include "digikamimagedelegatepriv.h"

namespace Digikam
{

DigikamImageFaceDelegate::DigikamImageFaceDelegate(ImageCategorizedView *parent)
             : DigikamImageDelegate(*new DigikamImageFaceDelegatePrivate, parent)
{
}

DigikamImageFaceDelegate::~DigikamImageFaceDelegate()
{
}

void DigikamImageFaceDelegate::prepareThumbnails(ImageThumbnailModel *thumbModel, const QList<QModelIndex>& indexes)
{
    //TODO
    DigikamImageDelegate::prepareThumbnails(thumbModel, indexes);
}

QPixmap DigikamImageFaceDelegate::thumbnailPixmap(const QModelIndex& index) const
{
    QVariant extraData = index.data(ImageModel::ExtraDataRole);
    if (extraData.isNull() || extraData.type() != QVariant::String)
        return DigikamImageDelegate::thumbnailPixmap(index);

    QRect rect = FaceIface::svgToRect(extraData.toString());

    // set requested thumbnail detail
    if (rect.isValid())
        const_cast<QAbstractItemModel*>(index.model())->setData(index, rect, ImageModel::ThumbnailRole);

    // parent implementation already resets the thumb size and rect set on model
    return DigikamImageDelegate::thumbnailPixmap(index);
}

void DigikamImageFaceDelegate::updateRects()
{
    DigikamImageDelegate::updateRects();
}

void DigikamImageFaceDelegate::clearModelDataCaches()
{
    DigikamImageDelegate::clearModelDataCaches();
}

} // namespace Digikam
