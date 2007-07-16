/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-17
 * Description : low level files management interface. 
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

// C Ansi includes.

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

// C++ Includes.

#include <cstdio>

// QT includes.

#include <QFile>
#include <QByteArray>
#include <QDataStream>

// KDE includes.

#include <kio/renamedlg.h>
#include <kio/deletejob.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprotocolinfo.h>
#include <kglobalsettings.h>

// Local includes.

#include "ddebug.h"
#include "albumsettings.h"
#include "albummanager.h"
#include "albumlister.h"
#include "albumdb.h"
#include "album.h"
#include "imagelister.h"
#include "dio.h"
#include "dio_p.h"
#include "dio_p.moc"

namespace DIO
{

KIO::Job* copy(const KUrl& src, const KUrl& dest)
{
    KIO::Job* job = KIO::copy(src, dest, true);
    new Watch(job);

    return job;
}

KIO::Job* copy(const KUrl::List& srcList, const KUrl& dest)
{
    KIO::Job* job = KIO::copy(srcList, dest, true);
    new Watch(job);

    return job;
}

KIO::Job* move(const KUrl& src, const KUrl& dest)
{
    KIO::Job* job = KIO::move(src, dest, true);
    new Watch(job);

    return job;
}

KIO::Job* move(const KUrl::List& srcList, const KUrl& dest)
{
    KIO::Job* job = KIO::move(srcList, dest, true);
    new Watch(job);

    return job;
}

KIO::Job* del(const KUrl& src, bool useTrash)
{
    KIO::Job* job = 0;
    
    if (useTrash)
    {
        job = KIO::trash( src );
    }   
    else
    {
	job = KIO::del(src);
    }
    
    new Watch(job);
    return job;
}

KIO::Job* del(const KUrl::List& srcList, bool useTrash)
{
    KIO::Job* job = 0;

    if (useTrash)
    {
        job = KIO::trash( srcList);
    }   
    else
    {
        job = KIO::del(srcList);
    }
    
    new Watch(job);
    return job;
}

KIO::CopyJob *rename(const KUrl& src, const KUrl& dest)
{
    KIO::CopyJob * job = KIO::move(src, dest, false);
    new Watch(job);

    return job;

    /*
    KUrl srcdir;
    srcdir.setDirectory(src.directory());
    KUrl dstdir;
    dstdir.setDirectory(dest.directory());
    Digikam::PAlbum* srcAlbum = Digikam::AlbumManager::componentData()->findPAlbum(srcdir);
    Digikam::PAlbum* dstAlbum = Digikam::AlbumManager::componentData()->findPAlbum(dstdir);
    if (!srcAlbum || !dstAlbum)
    {
        DWarning() << "Source Album " << src.directory() << " not found" << endl;
        return false;
    }

    QString srcPath = Digikam::AlbumManager::componentData()->getLi braryPath() + src.path();
    QString dstPath = Digikam::AlbumManager::componentData()->getLi braryPath() + dest.path();
    QString newDstPath;

    bool overwrite = false;

    struct stat stbuf; 
    while (::stat(QFile::encodeName(dstPath), &stbuf) == 0)
    {
        KIO::RenameDlg_Result result =
            KIO::open_RenameDlg(i18n("Rename File"), srcPath, dstPath,
                                KIO::RenameDlg_Mode(KIO::M_SINGLE |
                                                    KIO::M_OVERWRITE),
                                newDstPath);

        dstPath = newDstPath;

        switch (result)
        {
        case KIO::R_CANCEL:
        {
            return false;
        }
        case KIO::R_OVERWRITE:
        {
            overwrite = true;
            break;
        }
        default:
            break;
        }

        if (overwrite)
            break;
    }

    Digikam::AlbumDB* db = Digikam::AlbumManager::componentData()->albumD();
    if (::rename(QFile::encodeName(srcPath), QFile::encodeName(dstPath)) == 0)
    {
        db->moveItem(srcAlbum->id(), src.fileName(),
                     dstAlbum->id(), KUrl(dstPath).fileName());
        return true;
    }

    KMessageBox::error(0, i18n("Failed to rename file\n%1")
                       .arg(src.fileName()), i18n("Rename Failed"));
    return false;
    */
}

KIO::Job* scan(const KUrl& albumURL)
{
    KIO::Job* job = Digikam::ImageLister::startListJob(albumURL,
                                                       QString(), // filter - invalid for scan
                                                       0,         // getDimension - invalid for scan
                                                       1);        // scan
    new Watch(job);

    return job;
}

bool running()
{
    return (Watch::m_runCount != 0);
}

Watch::Watch(KIO::Job* job)
{
    m_runCount++;
    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotDone(KJob*)));
}

void Watch::slotDone(KJob*)
{
    Digikam::AlbumManager::componentData()->refresh();
    Digikam::AlbumLister::componentData()->refresh();
    m_runCount--;

    delete this;
}

uint Watch::m_runCount = 0;

}  // namespace DIO
