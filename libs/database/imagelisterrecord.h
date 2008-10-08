/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Data set for image lister
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGELISTERRECORD_H
#define IMAGELISTERRECORD_H

// Qt includes.

#include <QString>
#include <QDataStream>
#include <QDateTime>
#include <QSize>

// KDE includes.

#include "digikam_export.h"
#include "albuminfo.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ImageListerRecord
{

public:

    ImageListerRecord()
    {
        imageID     = -1;
        albumID     = -1;
        albumRootID = -1;
        rating      = -1;
        fileSize    = -1;
    }

    qlonglong  imageID;
    int        albumID;
    int        albumRootID;
    QString    name;

    int        rating;
    DatabaseItem::Category category;
    QString    format;
    QDateTime  creationDate;
    QDateTime  modificationDate;
    int        fileSize;
    QSize      imageSize;
};

DIGIKAM_DATABASE_EXPORT QDataStream &operator<<(QDataStream &os, const ImageListerRecord &record);
DIGIKAM_DATABASE_EXPORT QDataStream &operator>>(QDataStream &ds, ImageListerRecord &record);

}  // namespace Digikam

#endif // IMAGELISTERRECORD_H
