/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Listing information from database.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGELISTER_H
#define IMAGELISTER_H

// Qt includes.

#include <QString>

// KDE includes.

#include <kio/job.h>

// Local includes.

#include "digikam_export.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "databaseaccess.h"
#include "databaseurl.h"
#include "imagelisterrecord.h"
#include "imagelisterreceiver.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ImageLister
{
public:

    /**
     * Create a TransferJob for the "special" method of one of the database ioslaves,
     * referenced by the URL.
     * @param extraValue If -1, nothing is sent. If it takes another value,
     *                   this value will be sent as a second parameter.
     */
    static KIO::TransferJob *startListJob(const DatabaseUrl &url, int extraValue = -1);

    ImageLister();

    /**
     * Adjust the setting if album or tags will be listed recursively (i.e. including subalbums / subtags)
     */
    void setRecursive(bool recursive);

    /**
     * Convenience method for Album, Tag and Date URLs, _not_ for Search URLs.
     */
    void list(ImageListerReceiver *receiver,
              const DatabaseUrl &url);

    /**
      * List images in the Album (physical album) specified by albumRoot, album.
      * The results will be fed to the specified receiver.
      */
    void listAlbum(ImageListerReceiver *receiver,
                   int albumRootId, const QString &album);

    /**
     * List the images which have assigned the tag specified by tagId
     */
    void listTag(ImageListerReceiver *receiver, int tagId);
    /**
      * List those images whose date lies in the range beginning with startDate (inclusive)
      * and ending before endDate (exclusive).
      */
    void listDateRange(ImageListerReceiver *receiver, const QDate &startDate, const QDate &endDate);

    /**
     * Execute the search specified by search XML
     * @param xml SearchXml describing the query
     * @param limit limit the count of the result set. If limit = 0, then no limit is set.
     */
    void listSearch(ImageListerReceiver *receiver,
                    const QString &xml,
                    int limit = 0);

    /**
     * Execute the search specified by search XML describing a Haar search
     * @param xml SearchXml describing the query
     * @param limit limit the count of the result set. If limit = 0, then no limit is set.
     */
    void listHaarSearch(ImageListerReceiver *receiver,
                        const QString &xml);

private:

    void listFromIdList(ImageListerReceiver *receiver, QList<qlonglong> imageIds);

private:

    bool m_recursive;
};

}  // namespace Digikam

#endif // IMAGELISTER_H
