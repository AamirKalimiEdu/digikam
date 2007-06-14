/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : digiKam kipi library interface.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2005 by Ralf Holzer <ralf at well.com>
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_KIPIINTERFACE_H
#define DIGIKAM_KIPIINTERFACE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes.

#include <q3valuelist.h>
#include <qstring.h>
#include <qmap.h>

// KDE includes.

#include <kurl.h>
#include <kio/job.h>

// libKipi Includes.

#include <libkipi/interface.h>
#include <libkipi/imagecollection.h>
#include <libkipi/imageinfo.h>
#include <libkipi/imageinfoshared.h>
#include <libkipi/imagecollectionshared.h>

class QDateTime;

namespace KIPI
{
class Interface;
class ImageCollection;
class ImageInfo;
}

namespace Digikam
{

class AlbumManager;
class Album;
class PAlbum;
class TAlbum;
class AlbumDB;
class AlbumSettings;

/** DigikamImageInfo: class to get/set image information/properties in a digiKam album. */

class DigikamImageInfo : public KIPI::ImageInfoShared
{
public:
    
    DigikamImageInfo( KIPI::Interface* interface, const KUrl& url );
    ~DigikamImageInfo();
    
    virtual QString title();
    virtual void setTitle( const QString& );

    virtual QString description();
    virtual void setDescription( const QString& );

    virtual void cloneData( ImageInfoShared* other );

    virtual QDateTime time( KIPI::TimeSpec spec );
    virtual void setTime( const QDateTime& time, KIPI::TimeSpec spec = KIPI::FromInfo );
    
    virtual QMap<QString, QVariant> attributes();                    
    virtual void addAttributes(const QMap<QString, QVariant>& res);
    virtual void clearAttributes();
    
    virtual int  angle();
    virtual void setAngle( int angle );
    
private:

    PAlbum *parentAlbum();

private:

    PAlbum *palbum_;
};


/** DigikamImageCollection: class to get/set image collection information/properties in a digiKam 
    album database. */

class DigikamImageCollection : public KIPI::ImageCollectionShared
{
    
public:

    enum Type 
    { 
        AllItems, 
        SelectedItems 
    };

public:

    DigikamImageCollection( Type tp, Album *album, const QString& filter );
    ~DigikamImageCollection();
    
    virtual QString name();
    virtual QString comment();
    virtual QString category();
    virtual QDate date();
    virtual KUrl::List images();
    virtual KUrl path();
    virtual KUrl uploadPath();
    virtual KUrl uploadRoot();
    virtual QString uploadRootName();
    virtual bool isDirectory();
    virtual bool operator==(ImageCollectionShared&);
    
private:

    KUrl::List imagesFromPAlbum(PAlbum* album) const;
    KUrl::List imagesFromTAlbum(TAlbum* album) const;
    
private:

    Type     tp_;
    Album   *album_;
    QString  imgFilter_;
};


/** DigikamKipiInterface: class to interface digiKam with kipi library. */

class DigikamKipiInterface : public KIPI::Interface
{
    Q_OBJECT

public:

    DigikamKipiInterface( QObject *parent, const char *name=0);
    ~DigikamKipiInterface();

    virtual KIPI::ImageCollection currentAlbum();
    virtual KIPI::ImageCollection currentSelection();
    virtual Q3ValueList<KIPI::ImageCollection> allAlbums();
    virtual KIPI::ImageInfo info( const KUrl& );
    virtual bool addImage( const KUrl&, QString& errmsg );
    virtual void delImage( const KUrl& );
    virtual void refreshImages( const KUrl::List& urls );
    virtual int features() const;
    virtual QString fileExtensions();

public slots:

    void slotSelectionChanged( bool b );
    void slotCurrentAlbumChanged( Album *palbum );

private:

    AlbumManager *albumManager_;
};

}  // namespace Digikam

#endif  // DIGIKAM_KIPIINTERFACE_H

