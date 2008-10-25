/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Container for image info objects
 *
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

#ifndef IMAGEINFOLIST_H
#define IMAGEINFOLIST_H

// Qt includes.

#include <QList>

// Local includes.

#include "imageinfo.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageInfo;

class DIGIKAM_DATABASE_EXPORT ImageInfoList : public QList<ImageInfo>
{

public:

    //TODO: Connect change signals from album db,
    // to provide facilities to update contained ImageInfos
};

typedef ImageInfoList::iterator ImageInfoListIterator;

}  // namespace Digikam

#endif // IMAGEINFOLIST_H

