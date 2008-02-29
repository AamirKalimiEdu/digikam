/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-02-29
 * Description : Drag object info containers.
 * 
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DDRAGOBJECTS_H
#define DDRAGOBJECTS_H

// Qt includes.

#include <QMimeData>
#include <QList>
#include <QStringList>

// KDE includes.

#include <kurl.h>

class QWidget;

namespace Digikam
{

/**
 * Provides a drag object for a list of KURLs with its related database IDs.
 *
 * Images can be moved through ItemDrag. It is possible to move them on
 * another application which understands KURLDrag, like konqueror, to
 * copy the images. Digikam can use the IDs, if ItemDrag is dropped
 * on digikam itself.
 * The kioURLs are internally used with the digikamalbums kioslave.
 * The "normal" KURLDrag urls are used for external drops (k3b, gimp, ...)
 */
class DItemDrag : public QMimeData
{
public:

    DItemDrag(const KUrl::List& urls, 
              const KUrl::List& kioURLs,
              const QList<int>& albumIDs,
              const QList<int>& imageIDs,
              const char* name=0);

    static bool canDecode(const QMimeData* e);
    static bool decode(const QMimeData* e,
                       KUrl::List &urls,
                       KUrl::List &kioURLs,
                       QList<int>& albumIDs,
                       QList<int>& imageIDs);

protected:

    virtual const char* format(int i) const;
    virtual QByteArray encodedData(const char* mime) const;
};

// ------------------------------------------------------------------------

/**
 * Provides a drag object for a tag
 *
 * When a tag is moved through drag'n'drop an object of this class 
 * is created.
 */
class DTagDrag : public QMimeData
{
public:

    DTagDrag(int albumid, const char *name=0);
    static bool canDecode(const QMimeData* e);

protected:

    QByteArray  encodedData(const char* mime) const;
    const char* format(int i) const;
};

// ------------------------------------------------------------------------

/**
 * Provides a drag object for an album
 *
 * When an album is moved through drag'n'drop an object of this class 
 * is created.
 */
class DAlbumDrag : public QMimeData
{
public:

    DAlbumDrag(const KUrl &url, int albumid, const char *name=0);
    static bool canDecode(const QMimeData* e);
    static bool decode(const QMimeData* e, KUrl::List &urls, int &albumID);

protected:

    QByteArray  encodedData(const char* mime) const;
    const char* format(int i) const;
};

// ------------------------------------------------------------------------

/**
 * Provides a drag object for a list of tags
 *
 * When a tag is moved through drag'n'drop an object of this class 
 * is created.
 */
class DTagListDrag : public QMimeData
{
public:

    DTagListDrag(const QList<int>& tagIDs, const char *name=0);
    static bool canDecode(const QMimeData* e);

protected:

    QByteArray  encodedData(const char* mime) const;
    const char* format(int i) const;
};

// ------------------------------------------------------------------------

/**
 * Provides a drag object for a list of camera items
 *
 * When a camera item is moved through drag'n'drop an object of this class
 * is created.
 */
class DCameraItemListDrag : public QMimeData
{
public:

    DCameraItemListDrag(const QStringList& cameraItemPaths, const char *name=0);
    static bool canDecode(const QMimeData* e);

protected:

    QByteArray  encodedData(const char* mime) const;
    const char* format(int i) const;
};

}  // namespace Digikam

#endif /* DDRAGOBJECTS_H */
