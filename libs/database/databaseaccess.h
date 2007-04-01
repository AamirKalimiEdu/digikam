/* ============================================================
 * Authors: Marcel Wiesweg
 * Date   : 2007-03-18
 * Description : database interface.
 * 
 * Copyright 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DATABASEACCESS_H
#define DATABASEACCESS_H

#include "digikam_export.h"
#include "databaseparameters.h"

namespace Digikam
{

class AlbumDB;
class DatabaseAccessStaticPriv;

class DIGIKAM_EXPORT DatabaseAccess
{
public:

    /** The DatabaseAccess provides access to the database:
      * Create an instance of this class on the stack to retrieve a pointer to the database.
      * While you hold an instance of DatabaseAccess, the database access is locked for other threads,
      * but _not_ for other processes. This is due to the fact that while databases allow
      * concurrent access (of course), their client libs may not be thread-safe.
      */

    /**
      * Create a DatabaseAccess object for the default database.
      * Note that when initializing your app, setParameters need to be called
      * (in a not-yet-multithreaded context) for this to work.
      */
    DatabaseAccess();
    ~DatabaseAccess();

    /**
      * Retrieve a pointer to the database (this is what you want)
      */
    AlbumDB *db();

    /**
      * Return the default parameters
      */
    static DatabaseParameters parameters();

    /**
      * Set the default parameters.
      * Call this function at least once in the starting phase of your application,
      * when no other threads will yet access the database, to initialize DatabaseAcccess.
      * After this initial call, it is thread-safe to call this function again.
      * In a subsequent call, if the parameters are identical, nothing is done.
      * If the parameters change, the current database will be closed.
      * When parameters have been set or changed, the new one will be opened on-demand,
      * i.e. when the first DatabaseAccess object is constructed.
      */
    static void setParameters(const DatabaseParameters &parameters);

    /**
      * Clean up the database access.
      * When this function has been called, the access can be restored by calling setParameters.
      * Construction a database access object otherwise after calling this method will crash.
      */
    static void cleanUpDatabase();

    //TODO: remove
    static void setAlbumRoot(const QString &root);
    static QString albumRoot();

private:

    static DatabaseAccessStaticPriv *d;
};

}

#endif

