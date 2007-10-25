/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Access to comments of an image in the database
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

#ifndef IMAGECOMMENTS_H
#define IMAGECOMMENTS_H

// Qt includes

#include <QString>
#include <QDateTime>
#include <QSharedDataPointer>

// Local includes

#include "digikam_export.h"
#include "albuminfo.h"
#include "databaseaccess.h"

namespace Digikam
{

class ImageCommentsPriv;

class DIGIKAM_EXPORT ImageComments
{
    /**
     * The ImageComments class shall provide short-lived objects that provide read/write access
     * to the comments stored in the database. It is a mere wrapper around the less
     * convenient access methods in AlbumDB.
     * Database results are cached, but the object will not listen to database changes from other places.
     *
     * Changes are applied to the database only after calling apply(), which you can call any time
     * and which will in any case be called from the destructor.
     */

public:

    /** Create a null ImageComments object */
    ImageComments();

    /**
     * Create a ImageComments object for the image with the specified id.
     */
    ImageComments(qlonglong imageid);
    /**
     * Create a ImageComments object for the image with the specified id.
     * The existing DatabaseAccess object will be used to access the database.
     */
    ImageComments(DatabaseAccess &access, qlonglong imageid);

    ImageComments(const ImageComments &other);
    ~ImageComments();

    bool isNull() const;

    enum LanguageChoiceBehavior
    {
        /**
         *  Return only a comment if the language code
         *  (at least the language code, the country part may differ)
         *  is identical. Else returns a null QString.
         */
        ReturnMatchingLanguageOnly,
        /// If no matching language as above is found, return the default language.
        ReturnMatchingOrDefaultLanguage,
        /** If no matching or default language is found, return the first comment.
         *  Returns a null string only if no comment is available.
         */
        ReturnMatchingDefaultOrFirstLanguage
    };

    enum UniqueBehavior
    {
        /** Allow only one comment per language. Default setting. */
        UniquePerLanguage,
        /** Allow multiple comments per language, each with a different author */
        UniquePerLanguageAndAuthor
    };

    /**
     * Changes the behavior to unique comments per language, see the enum above for possible
     * values.
     * Note: This is _not_ a property of the database, but only of this single ImageComments object,
     */
    void setUniqueBehavior(UniqueBehavior behavior);



    /**
     * This methods presents one of the comment strings of the available comment
     * as the default value, when you just want to have one string.
     * Optionally also returns the index with which you can access further information about the comment.
     */
    QString defaultComment(int *index = 0) const;

    /**
     * Returns a comment for the specified language.
     * Matching behavior can be specified.
     * Optionally also returns the index with which you can access further information about the comment.
     */
    QString commentForLanguage(const QString &languageCode, int *index = 0,
                               LanguageChoiceBehavior behavior = ReturnMatchingDefaultOrFirstLanguage) const;

    /** Returns the number of comments available. */
    int numberOfComments() const;

    /// Access individual properties. Please ensure that the specified index is a valid index

    DatabaseComment::Type type(int index) const;
    QString language(int index) const; /// RFC 3066 notation, or "x-default"
    QString author(int index) const;
    QDateTime date(int index) const;
    QString comment(int index) const;



    /**
     * Add a new comment to the list of normal image comments, specified with language and author.
     * Checking for unique comments is done as set by setUniqueBehavior.
     * If you pass a null string as language, it will be translated to the language code designating
     * the default language ("x-default").
     * If you just want to change the one comment of the image, call addComment(myComment);
     */
    void addComment(const QString &comment,
                    const QString &language = QString(),
                    const QString &author = QString(),
                    const QDateTime &date = QDateTime(),
                    DatabaseComment::Type type = DatabaseComment::Comment);

    /** Convenience method to add a comment of type Headline. Calls addComment, see above for more info. */
    void addHeadline(const QString &comment,
                     const QString &language = QString(),
                     const QString &author = QString(),
                     const QDateTime &date = QDateTime());
    /** Convenience method to add a comment of type Headline. Calls addComment, see above for more info. */
    void addTitle(const QString &comment,
                  const QString &language = QString(),
                  const QString &author = QString(),
                  const QDateTime &date = QDateTime());

    /**
     * Access individual properties.
     * Please ensure that the specified index is a valid index
     */

    void changeComment(int index, const QString &comment);
    void changeLanguage(int index, const QString &language);
    void changeAuthor(int index, const QString &author);
    void changeDate(int index, const QDateTime &date);
    void changeType(int index, DatabaseComment::Type type);

    /**
     * Apply all changes.
     */
    void apply();
    void apply(DatabaseAccess &access);

    // If you need more methods, add your methods here!

protected:

    void addCommentDirect(const QString &comment,
                          const QString &language,
                          const QString &author,
                          DatabaseComment::Type type,
                          const QDateTime &date);

    QSharedDataPointer<ImageCommentsPriv> d;
};



} // namespace Digikam


#endif

