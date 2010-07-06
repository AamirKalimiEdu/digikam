/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : Handling accesses to one image and associated data
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2007-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEINFO_H
#define IMAGEINFO_H

// Qt includes

#include <QString>
#include <QDateTime>
#include <QList>
#include <QSize>

// KDE includes

#include <kurl.h>

// Local includes

#include "albuminfo.h"
#include "digikam_export.h"
#include "dshareddata.h"
#include "databaseurl.h"
#include "imageinfolist.h"

namespace Digikam
{

class DImageHistory;
class ImageComments;
class ImageCommonContainer;
class ImageCopyright;
class ImageExtendedProperties;
class ImageInfoData;
class ImageListerRecord;
class ImageMetadataContainer;
class ImagePosition;
class ImageTagPair;
class PhotoInfoContainer;
class Template;

/**
 * The ImageInfo class contains provides access to the database for a single image.
 * The properties can be read and written. Information will be cached.
 */
class DIGIKAM_DATABASE_EXPORT ImageInfo
{
public:

    /**
     * Constructor
     * Creates a null image info
     */
    ImageInfo();

    /**
     * Constructor. Creates an ImageInfo object without any cached data initially.
     * @param    ID       unique ID for this image
     */
    explicit ImageInfo(qlonglong ID);

    /**
     * Constructor. Creates an ImageInfo object from a file url.
     */
    ImageInfo(const KUrl& url);

    /**
     * Constructor. Creates an ImageInfo object where the provided information
     * will initially be available cached, without database access.
     */
    ImageInfo(const ImageListerRecord& record);

    ImageInfo(const ImageInfo& info);

    /**
     * Destructor
     */
    ~ImageInfo();

    ImageInfo& operator=(const ImageInfo& info);

    bool operator==(const ImageInfo& info) const;
    bool operator!=(const ImageInfo& info) const { return !operator==(info); }
    bool operator<(const ImageInfo& info) const;
    uint hash() const;

    /**
     * Returns if this objects contains valid data
     */
    bool      isNull() const;

    /**
     * @return the name of the image
     */
    QString   name() const;

    /**
     * @return the datetime of the image
     */
    QDateTime dateTime() const;

    /**
     * @return the modification datetime of the image
     */
    QDateTime modDateTime() const;

    /**
     * @return the filesize of the image
     */
    uint      fileSize() const;

    /**
     * @return the dimensions of the image (valid only if dimensions
     * have been requested)
     */
    QSize     dimensions() const;

    /**
     * Returns the digikamalbums:// URL.
     * The returned object can be used as a KUrl.
     * Always use this for KIO operations
     */
    DatabaseUrl databaseUrl() const;

    /**
     * Returns the file:// url.
     * This is equivalent to databaseUrl().fileUrl()
     */
    KUrl       fileUrl() const;

    /**
     * Equivalent to fileUrl().path()
     */
    // Deprecate?
    QString   filePath() const;

    /**
     * @return the unique image id for this item
     */
    qlonglong   id() const;

    /**
     * @return the id of the PAlbum to which this item belongs
     */
    int       albumId() const;

    /**
     * The album root id
     */
    int albumRootId() const;

    /**
     * @return the caption for this item
     */
    QString   comment() const;

    /**
     * Returns the rating
     */
    int       rating() const;

    /**
     * Returns the category of the item: Image, Audio, Video
     */
    DatabaseItem::Category category() const;

    /** Returns the image format / mimetype as a standardized
     *  string (see DBSCHEMA.ODS).
     */
    QString   format() const;

    /**
     * @return a list of IDs of tags assigned to this item
     * @see tagNames
     * @see tagPaths
     * @see Album::id()
     */
    QList<int> tagIds() const;

    /**
     * Retrieve the ImageComments object for this item.
     * This object allows full read and write access to all comments
     * and their properties.
     * You need to hold DatabaseAccess to ensure the validity.
     * For simple, cached read access see comment().
     */
    ImageComments imageComments(DatabaseAccess& access) const;

    /**
     * Retrieve the ImageCopyright object for this item.
     * This object allows full read and write access to all copyright
     * values.
     */
    ImageCopyright imageCopyright() const;

    /**
     * Retrieve the ImageExtendedProperties object for this item.
     * This object allows full read and write access to all extended properties
     * values.
     */
    ImageExtendedProperties imageExtendedProperties() const;

    /**
     * Retrieve the ImagePosition object for this item.
     */
    ImagePosition imagePosition() const;

    /**
     * Retrieve an ImageTagPair object for a single tag, or for all
     * image/tag pairs for which properties are available
     * (not necessarily the assigned tags)
     */
    ImageTagPair imageTagPair(int tagId) const;
    QList<ImageTagPair> availableImageTagPairs() const;

    /**
     * Retrieves and sets the image history from the database
     */
    DImageHistory imageHistory() const;
    void setImageHistory(const DImageHistory& history);

    /**
     * Retrieves and sets this' images UUID
     */
    QString uuid() const;
    void setUuid(const QString& uuid);

    /**
     * Retrieve information about the image,
     * in form of numbers and user presentable strings,
     * for certain defined fields of information (see databaseinfocontainers.h)
     */
    ImageCommonContainer   imageCommonContainer() const;
    ImageMetadataContainer imageMetadataContainer() const;
    PhotoInfoContainer     photoInfoContainer() const;


    /**
     * Retrieve metadata template information about the image.
     */
    Template metadataTemplate() const;

    /**
     * Set metadata template information (write it to database)
     * @param t the new template data.
     */
    void setMetadataTemplate(const Template& t);

    /**
     * Remove all template info about the image from database.
     */
    void removeMetadataTemplate();

    /**
     * Set the date and time (write it to database)
     * @param dateTime the new date and time.
     */
    void setDateTime(const QDateTime& dateTime);

    /**
     * Adds a tag to the item (writes it to database)
     * @param tagID the ID of the tag to add
     */
    void        setTag(int tagID);

    /**
     * Adds tags in the list to the item.
     * Tags are created if they do not yet exist
     */
    void        addTagPaths(const QStringList& tagPaths);

    /**
     * Remove a tag from the item (removes it from database)
     * @param tagID the ID of the tag to remove
     */
    void        removeTag(int tagID);

    /**
     * Remove all tags from the item (removes it from database)
     */
    void        removeAllTags();

    /**
     * Set the rating for the item
     */
    void        setRating(int value);


    /**
     * Copy database information of this item to a newly created item
     * @param  dstAlbumID  destination album id
     * @param  dstFileName new filename
     * @return an ImageInfo object of the new item
     */
    //TODO: Move to album?
    ImageInfo   copyItem(int dstAlbumID, const QString& dstFileName);

private:

    friend class ImageInfoCache;
    friend class ImageInfoList;

    DSharedDataPointer<ImageInfoData> m_data;
};

inline uint qHash(const ImageInfo& info) { return info.hash(); }

DIGIKAM_DATABASE_EXPORT QDebug& operator<<(QDebug& stream, const ImageInfo& info);

}  // namespace Digikam

Q_DECLARE_TYPEINFO(Digikam::ImageInfo, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(Digikam::ImageInfo)

#endif /* IMAGEINFO_H */
