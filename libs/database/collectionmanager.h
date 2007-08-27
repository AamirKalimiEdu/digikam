/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location management
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

#ifndef COLLECTIONMANAGER_H
#define COLLECTIONMANAGER_H

// Qt includes

#include <QString>
#include <QStringList>

// KDE includes

#include <kurl.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class CollectionLocation;
class CollectionManagerPrivate;

class DIGIKAM_EXPORT CollectionManager : public QObject
{

    Q_OBJECT

public:

    static CollectionManager *instance();
    static void cleanUp();

    /**
     * Add the given file system location as new collection location.
     * Type and availability will be detected.
     * On failure returns null. This would be the case if the given
     * url is already contained in another collection location.
     */
    CollectionLocation *addLocation(const KUrl &fileUrl);

    /**
     * Removes the given location. This means that all images contained on the
     * location will be removed from the database, all tags will be lost.
     */
    void removeLocation(CollectionLocation *location);

    /**
     * Returns a list of all CollectionLocations stored in the database
     */
    QList<CollectionLocation *> allLocations();
    /**
     * Returns a list of all currently available CollectionLocations
     */
    QList<CollectionLocation *> allAvailableLocations();
    /**
     * Returns a list of the paths of all currently available CollectionLocations
     */
    QStringList allAvailableAlbumRootPaths();

    /**
     * Returns the CollectionLocation that contains the given album root.
     * The path must be an album root with isAlbumRoot() == true.
     * Returns 0 if no collection location matches.
     * Only available (or hidden, but available) locations are guaranteed to be found.
     */
    CollectionLocation *locationForAlbumRootId(int id);
    CollectionLocation *locationForAlbumRoot(const KUrl &fileUrl);
    CollectionLocation *locationForAlbumRootPath(const QString &albumRootPath);

    /**
     * Returns the CollectionLocation that contains the given path.
     * Equivalent to calling locationForAlbumRoot(albumRoot(fileUrl)).
     * Only available (or hidden, but available) locations are guaranteed to be found.
     */
    CollectionLocation *locationForUrl(const KUrl &fileUrl);
    CollectionLocation *locationForPath(const QString &filePath);

    /**
     * For a given path, the part of the path that forms the album root is returned,
     * ending without a slash. Example: "/media/fotos/Paris 2007" gives "/media/fotos".
     * Only available (or hidden, but available) album roots are guaranteed to be found.
     */
    KUrl    albumRoot(const KUrl &fileUrl);
    QString albumRootPath(const KUrl &fileUrl);
    QString albumRootPath(const QString &filePath);
    /**
     * Returns true if the given path forms an album root.
     * It will return false if the path is a path below an album root,
     * or if the the path does not belong to an album root.
     * Example: "/media/fotos/Paris 2007" is an album with album root "/media/fotos".
     *          "/media/fotos" returns true, "/media/fotos/Paris 2007" and "/media" return false.
     * Only available (or hidden, but available) album roots are guaranteed to be found.
     */
    bool    isAlbumRoot(const KUrl &fileUrl);
    /// the file path should not end with the directory slash. Using DatabaseUrl's method is fine.
    bool    isAlbumRoot(const QString &filePath);

    /**
     * Returns the album part of of the given file path, i.e. the album root path
     * at the beginning is removed and the second part, starting with "/", ending without a slash,
     * is returned. Example: "/media/fotos/Paris 2007" gives "/Paris 2007"
     * Returns a null QString if the file path is not located in an album root.
     * Returns "/" if the file path is an album root.
     */
    QString album(const KUrl &fileUrl);
    QString album(const QString &filePath);

    /**
     * Returns just one album root, out of the list of available location,
     * the one that is most suitable to serve as a default, e.g.
     * to suggest as default place when the user wants to add files.
     */
    KUrl oneAlbumRoot();
    QString oneAlbumRootPath();

private slots:

    void deviceChange(const QString &);

private:

    CollectionManager();
    ~CollectionManager();
    static CollectionManager *m_instance;
    void updateLocations();

    CollectionManagerPrivate *d;
};

}  // namespace Digikam

#endif // COLLECTIONMANAGER_H
