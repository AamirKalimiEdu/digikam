/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-26
 * Description : Drag object info container
 * 
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004 by Joern Ahrens <joern.ahrens@kdemail.net>
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

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albummanager.h"
#include "dragobjects.h"
//Added by qt3to4:
#include <Q3CString>
#include <Q3ValueList>

namespace Digikam
{

ItemDrag::ItemDrag(const KUrl::List &urls,
                   const KUrl::List &kioURLs,
                   const Q3ValueList<int>& albumIDs,
                   const Q3ValueList<int>& imageIDs,
                   QWidget* dragSource, const char* name)
    : KURLDrag(urls, dragSource, name),
      m_kioURLs(kioURLs),
      m_albumIDs(albumIDs), m_imageIDs(imageIDs)
{
}

bool ItemDrag::canDecode(const QMimeSource* e)
{
    return e->provides("digikam/item-ids")  || 
           e->provides("digikam/album-ids") ||
           e->provides("digikam/image-ids") ||
           e->provides("digikam/digikamalbums");
}

bool ItemDrag::decode(const QMimeSource* e, KUrl::List &urls, KUrl::List &kioURLs,
                      Q3ValueList<int>& albumIDs, Q3ValueList<int>& imageIDs)
{
    urls.clear();
    kioURLs.clear();
    albumIDs.clear();
    imageIDs.clear();
    
    if (KURLDrag::decode(e, urls))
    {
        QByteArray albumarray = e->encodedData("digikam/album-ids");
        QByteArray imagearray = e->encodedData("digikam/image-ids");
        QByteArray kioarray = e->encodedData("digikam/digikamalbums");
        
        if (albumarray.size() && imagearray.size() && kioarray.size())
        {
            int id;
            
            QDataStream dsAlbums(albumarray, QIODevice::ReadOnly);
            while (!dsAlbums.atEnd())
            {
                dsAlbums >> id;
                albumIDs.append(id);
            }
            
            QDataStream dsImages(imagearray, QIODevice::ReadOnly);
            while (!dsImages.atEnd())
            {
                dsImages >> id;
                imageIDs.append(id);
            }

            KUrl u;
            QDataStream dsKio(kioarray, QIODevice::ReadOnly);
            while (!dsKio.atEnd())
            {
                dsKio >> u;
                kioURLs.append(u);
            }

            return true;
        }
    }

    return false;
}

QByteArray ItemDrag::encodedData(const char* mime) const
{
    Q3CString mimetype(mime);
    
    if (mimetype == "digikam/album-ids")
    {
        QByteArray byteArray;
        QDataStream ds(byteArray, QIODevice::WriteOnly);

        Q3ValueList<int>::const_iterator it;
        for (it = m_albumIDs.begin(); it != m_albumIDs.end(); ++it)
        {
            ds << (*it);
        }
        
        return byteArray;
    }
    else if (mimetype == "digikam/image-ids")
    {
        QByteArray byteArray;
        QDataStream ds(byteArray, QIODevice::WriteOnly);

        Q3ValueList<int>::const_iterator it;
        for (it = m_imageIDs.begin(); it != m_imageIDs.end(); ++it)
        {
            ds << (*it);
        }
        
        return byteArray;
    }
    else if (mimetype == "digikam/digikamalbums")
    {
        QByteArray byteArray;
        QDataStream ds(byteArray, QIODevice::WriteOnly);

        KUrl::List::const_iterator it;
        for (it = m_kioURLs.begin(); it != m_kioURLs.end(); ++it)
        {
            ds << (*it);
        }
        
        return byteArray;
    }
    else
    {
        return KURLDrag::encodedData(mime);
    }
}

const char* ItemDrag::format(int i) const
{
    if (i == 0)
        return "text/uri-list";
    else if (i == 1)
        return "digikam/item-ids";
    else if (i == 2)
        return "digikam/album-ids";
    else if (i == 3)
        return "digikam/image-ids";
    else if (i == 4)
        return "digikam/digikamalbums";    
    else
        return 0;
}

TagDrag::TagDrag( int albumid, QWidget *dragSource, 
                  const char *name ) :
    Q3DragObject( dragSource, name )
{
    mAlbumID = albumid;
}

bool TagDrag::canDecode( const QMimeSource* e )
{
    return e->provides("digikam/tag-id");
}

const char* TagDrag::format( int i ) const
{
    if ( i == 0 )
        return "digikam/tag-id";

    return 0;
}

QByteArray TagDrag::encodedData( const char* ) const
{
    QByteArray ba;
    QDataStream ds(ba, QIODevice::WriteOnly);
    ds << mAlbumID;
    return ba;
}

AlbumDrag::AlbumDrag(const KUrl &url, int albumid, 
                     QWidget *dragSource, 
                     const char *name) :
    KURLDrag(url, dragSource, name)
{
    mAlbumID = albumid;
}

bool AlbumDrag::canDecode( const QMimeSource* e )
{
    return e->provides("digikam/album-id");
}

const char* AlbumDrag::format( int i ) const
{
    if (i == 0)
        return "text/uri-list";
    else if ( i == 1 )
        return "digikam/album-id";

    return 0;
}

QByteArray AlbumDrag::encodedData(const char *mime) const
{
    Q3CString mimetype( mime );
    if(mimetype == "digikam/album-id")
    {
        QByteArray ba;
        QDataStream ds(ba, QIODevice::WriteOnly);
        ds << mAlbumID;
        return ba;
    }
    else
    {
        return KURLDrag::encodedData(mime);
    }
}

bool AlbumDrag::decode(const QMimeSource* e, KUrl::List &urls, 
                       int &albumID)
{
    urls.clear();
    albumID = -1;
    
    if(KURLDrag::decode(e, urls))
    {
        QByteArray ba = e->encodedData("digikam/album-id");
        if (ba.size())
        {
            QDataStream ds(ba, QIODevice::ReadOnly);
            if(!ds.atEnd())
            {
                ds >> albumID;
            }
            return true;
        }
    }

    return false;
}

TagListDrag::TagListDrag(const Q3ValueList<int>& tagIDs, QWidget *dragSource,
                         const char *name)
    : Q3DragObject( dragSource, name )
{
    m_tagIDs = tagIDs;
}

bool TagListDrag::canDecode(const QMimeSource* e)
{
    return e->provides("digikam/taglist");
}

QByteArray TagListDrag::encodedData(const char*) const
{
    QByteArray ba;
    QDataStream ds(ba, QIODevice::WriteOnly);
    ds << m_tagIDs;
    return ba;
}

const char* TagListDrag::format(int i) const
{
    if ( i == 0 )
        return "digikam/taglist";

    return 0;
}

}  // namespace Digikam
