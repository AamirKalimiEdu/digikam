/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-18
 * Description : database album interface.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef THUMBNAILDB_H
#define THUMBNAILDB_H

// Qt includes

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>
#include <QtCore/QPair>
#include <QtCore/QMap>

// KDE includes

// Local includes

#include "thumbnaildatabaseaccess.h"
#include "digikam_export.h"

namespace Digikam
{

class DatabaseBackend;
class ThumbnailDBPriv;

class DIGIKAM_DATABASE_EXPORT ThumbnailDB
{
public:

    void setSetting(const QString& keyword, const QString& value);
    QString getSetting(const QString& keyword);

private:

    friend class Digikam::ThumbnailDatabaseAccess;

    ThumbnailDB(DatabaseBackend *backend);
    ~ThumbnailDB();

private:

    ThumbnailDBPriv* const d;
};

}  // namespace Digikam

#endif /* THUMBNAILDB_H */
