/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2005-04-21
 * Description : a dio-slave to process file operations on 
 *               digiKam albums.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *
 * Lots of the file io code is copied from KDE file kioslave.
 * Copyright for the KDE file kioslave follows:
 *  Copyright (C) 2000-2002 Stephan Kulow <coolo@kde.org>
 *  Copyright (C) 2000-2002 David Faure <faure@kde.org>
 *  Copyright (C) 2000-2002 Waldo Bastian <bastian@kde.org>
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
 * ============================================================ */

#ifndef DIGIKAMALBUMS_H
#define DIGIKAMALBUMS_H

// Qt includes.

#include <q3valuelist.h>
#include <qdatetime.h>
//Added by qt3to4:
#include <Q3CString>

// KDE includes

#include <kio/slavebase.h>

class kio_digikamalbums : public KIO::SlaveBase
{

public:

    kio_digikamalbums(const Q3CString &pool_socket,
                      const Q3CString &app_socket);
    ~kio_digikamalbums();

    void special(const QByteArray& data);

    void get( const KUrl& url );
    void put( const KUrl& url, int _mode, bool _overwrite, bool _resume );
    void copy( const KUrl &src, const KUrl &dest, int mode, bool overwrite );
    void rename( const KUrl &src, const KUrl &dest, bool overwrite );

    void stat( const KUrl& url );
    void listDir( const KUrl& url );
    void mkdir( const KUrl& url, int permissions );
    void chmod( const KUrl& url, int permissions );
    void del( const KUrl& url, bool isfile);

private:

    void createDigikamPropsUDSEntry(KIO::UDSEntry& entry);

    bool createUDSEntry(const QString& path, KIO::UDSEntry& entry);
    bool file_get( const KUrl& url );
    bool file_put( const KUrl& url, int _mode, bool _overwrite, bool _resume );
    bool file_copy( const KUrl &src, const KUrl &dest, int mode, bool overwrite );
    bool file_rename( const KUrl &src, const KUrl &dest, bool overwrite );
    bool file_stat( const KUrl& url );
    bool file_listDir( const KUrl& url );
    bool file_mkdir( const KUrl& url, int permissions );
    bool file_chmod( const KUrl& url, int permissions );
    bool file_del( const KUrl& url, bool isfile);

};


#endif /* DIGIKAMALBUMS_H */
