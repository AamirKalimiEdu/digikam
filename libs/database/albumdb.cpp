/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-18
 * Description :database album interface.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

/** @file albumdb.cpp */

// C Ansi includes.

extern "C"
{
#include <sys/time.h>
}

// C++ includes.

#include <cstdio>
#include <cstdlib>
#include <ctime>

// Qt includes.

#include <QFile>
#include <QFileInfo>
#include <QDir>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "databaseattributeswatch.h"
#include "databasebackend.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "albumdb.h"

namespace Digikam
{

class AlbumDBPriv
{

public:

    AlbumDBPriv()
    {
        db = 0;
    }

    DatabaseBackend *db;
    QList<int>  recentlyAssignedTags;
};

AlbumDB::AlbumDB(DatabaseBackend *backend)
{
    d = new AlbumDBPriv;
    d->db = backend;
}

AlbumDB::~AlbumDB()
{
    delete d;
}

QList<AlbumRootInfo> AlbumDB::getAlbumRoots()
{
    QList<AlbumRootInfo> list;

    QList<QVariant> values;
    d->db->execSql( "SELECT id, status, absolutePath, type, uuid, specificPath FROM AlbumRoots;", &values );

    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        AlbumRootInfo info;
        info.id           = (*it).toInt();
        ++it;
        info.status       = (*it).toInt();
        ++it;
        info.type         = (*it).toInt();
        ++it;
        info.absolutePath = (*it).toString();
        ++it;
        info.uuid         = (*it).toString();
        ++it;
        info.specificPath = (*it).toString();
        ++it;

        list << info;
    }

    return list;
}

QList<AlbumRootInfo> AlbumDB::getAlbumRootsWithStatus(int status)
{
    QList<AlbumRootInfo> list;

    QList<QVariant> values;
    d->db->execSql( "SELECT id, absolutePath, type, uuid, specificPath FROM AlbumRoots "
                    " WHERE status=?;", status, &values );

    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        AlbumRootInfo info;
        info.id           = (*it).toInt();
        ++it;
        info.status       = status;
        info.absolutePath = (*it).toString();
        ++it;
        info.type         = (*it).toInt();
        ++it;
        info.uuid         = (*it).toString();
        ++it;
        info.specificPath = (*it).toString();
        ++it;

        list << info;
    }

    return list;
}

int AlbumDB::addAlbumRoot(int type, const QString &absolutePath, const QString &uuid, const QString &specificPath)
{
    d->db->execSql( QString("REPLACE INTO AlbumRoots (type, status, absolutePath, uuid, specificPath) "
                            "VALUES(?, 0, ?, ?, ?);"),
                    type, absolutePath, uuid, specificPath);

    int id = d->db->lastInsertedRow();
    return id;
}

void AlbumDB::deleteAlbumRoot(int rootId)
{
    d->db->execSql( QString("DELETE FROM AlbumRoots WHERE id=?;"),
                    rootId );
}

int AlbumDB::getAlbumRootStatus(int rootId)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT status FROM AlbumRoots WHERE id=?;"),
                    rootId, &values );

    if (!values.isEmpty())
        return values.first().toInt();
    else
        return -1;
}

void AlbumDB::setAlbumRootStatus(int rootId, int status, const QString &absolutePath)
{
    if (absolutePath.isNull())
        d->db->execSql( QString("UPDATE AlbumRoots SET status=? WHERE id=?;"),
                        status, rootId );
    else
        d->db->execSql( QString("UPDATE AlbumRoots SET status=?, absolutePath=? WHERE id=?;"),
                        status, absolutePath, rootId );
}

#define OLDCODE
#ifdef OLDCODE
AlbumInfo::List AlbumDB::scanAlbums()
{
    AlbumInfo::List aList;

    QString basePath(DatabaseAccess::albumRoot());

    QList<QVariant> values;
    d->db->execSql( "SELECT A.id, A.url, A.date, A.caption, A.collection, B.url, I.name \n "
                    "FROM Albums AS A \n "
                    "  LEFT OUTER JOIN Images AS I ON A.icon=I.id \n"
                    "  LEFT OUTER JOIN Albums AS B ON B.id=I.dirid;", &values);

    QString iconAlbumUrl, iconName;

    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        AlbumInfo info;

        info.albumRoot = basePath;
        info.id = (*it).toInt();
        ++it;
        info.url = (*it).toString();
        ++it;
        info.date = QDate::fromString((*it).toString(), Qt::ISODate);
        ++it;
        info.caption = (*it).toString();
        ++it;
        info.collection = (*it).toString();
        ++it;
        iconAlbumUrl = (*it).toString();
        ++it;
        iconName = (*it).toString();
        ++it;

        if (!iconName.isEmpty())
        {
            info.icon = basePath + iconAlbumUrl + '/' + iconName;
        }

        aList.append(info);
    }

    return aList;
}

TagInfo::List AlbumDB::scanTags()
{
    TagInfo::List tList;

    QString basePath(DatabaseAccess::albumRoot());

    QList<QVariant> values;
    d->db->execSql( "SELECT T.id, T.pid, T.name, A.url, I.name, T.iconkde \n "
                    "FROM Tags AS T LEFT OUTER JOIN Images AS I ON I.id=T.icon \n "
                    "  LEFT OUTER JOIN Albums AS A ON A.id=I.dirid; ", &values );

    QString iconName, iconKDE, albumURL;

    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        TagInfo info;

        info.id     = (*it).toInt();
        ++it;
        info.pid    = (*it).toInt();
        ++it;
        info.name   = (*it).toString();
        ++it;
        albumURL    = (*it).toString();
        ++it;
        iconName    = (*it).toString();
        ++it;
        iconKDE     = (*it).toString();
        ++it;

        if ( albumURL.isEmpty() )
        {
            info.icon = iconKDE;
        }
        else
        {
            info.icon = basePath + albumURL + '/' + iconName;
        }

        tList.append(info);
    }

    return tList;
}

#else
AlbumInfo::List AlbumDB::scanAlbums()
{
    AlbumInfo::List aList;

    QList<QVariant> values;
    d->db->execSql( "SELECT R.absolutePath, A.id, A.relativePath, A.date, A.caption, A.collection, B.relativePath, I.name \n "
                    "FROM Albums AS A \n "
                    "  LEFT OUTER JOIN Images AS I ON A.icon=I.id \n"
                    "  LEFT OUTER JOIN Albums AS B ON B.id=I.dirid \n"
                    "  LEFT OUTER JOIN AlbumRoots AS R ON R.id=B.id;", &values);

    QString iconAlbumUrl, iconName;

    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        AlbumInfo info;

        info.albumRoot = (*it).toString();
        ++it;
        info.id = (*it).toInt();
        ++it;
        info.url = (*it).toString();
        ++it;
        info.date = QDate::fromString((*it).toString(), Qt::ISODate);
        ++it;
        info.caption = (*it).toString();
        ++it;
        info.collection = (*it).toString();
        ++it;
        iconAlbumUrl = (*it).toString();
        ++it;
        iconName = (*it).toString();
        ++it;

        if (!iconName.isEmpty())
        {
            info.icon = info.albumRoot + iconAlbumUrl + '/' + iconName;
        }

        aList.append(info);
    }

    return aList;
}

TagInfo::List AlbumDB::scanTags()
{
    TagInfo::List tList;

    QList<QVariant> values;
    d->db->execSql( "SELECT T.id, T.pid, T.name, A.relativePath, I.name, T.iconkde, R.absolutePath \n "
                    "FROM Tags AS T \n"
                    "  LEFT OUTER JOIN Images AS I ON I.id=T.icon \n "
                    "  LEFT OUTER JOIN Albums AS A ON A.id=I.dirid; "
                    "  LEFT OUTER JOIN AlbumRoots AS R ON R.id=A.albumRoot;", &values );

    QString iconName, iconKDE, albumURL, basePath;

    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        TagInfo info;

        info.id     = (*it).toInt();
        ++it;
        info.pid    = (*it).toInt();
        ++it;
        info.name   = (*it).toString();
        ++it;
        albumURL    = (*it).toString();
        ++it;
        iconName    = (*it).toString();
        ++it;
        iconKDE     = (*it).toString();
        ++it;
        basePath    = (*it).toString();
        ++it;

        if ( albumURL.isEmpty() )
        {
            info.icon = iconKDE;
        }
        else
        {
            info.icon = basePath + albumURL + '/' + iconName;
        }

        tList.append(info);
    }

    return tList;
}
#endif

SearchInfo::List AlbumDB::scanSearches()
{
    SearchInfo::List searchList;

    QList<QVariant> values;
    d->db->execSql( "SELECT id, name, url FROM Searches;", &values);

    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        SearchInfo info;

        info.id   = (*it).toInt();
        ++it;
        info.name = (*it).toString();
        ++it;
        info.url  = (*it).toString();
        ++it;

        searchList.append(info);
    }

    return searchList;
}

#ifdef OLDCODE
QList<AlbumShortInfo> AlbumDB::getAlbumShortInfos()
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT id, url from Albums;"),
                    &values);

    QList<AlbumShortInfo> albumList;

    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        AlbumShortInfo info;

        info.id        = (*it).toInt();
        ++it;
        info.url       = (*it).toString();
        ++it;
        info.albumRoot = DatabaseAccess::albumRoot();

        albumList << info;
    }

    return albumList;
}

QStringList AlbumDB::getSubalbumsForPath(const QString &albumRoot,
                                         const QString& path,
                                         bool onlyDirectSubalbums)
{
    QString subURL = path;
    if (!path.endsWith("/"))
        subURL += '/';
    subURL = (subURL);

    QList<QVariant> values;

    if (onlyDirectSubalbums)
    {
        d->db->execSql( QString("SELECT id, url FROM Albums WHERE url LIKE '") +
                        subURL + QString("%' ") + QString("AND url NOT LIKE '") +
                        subURL + QString("%/%'; "),
                        &values );
    }
    else
    {
        d->db->execSql( QString("SELECT id, url FROM Albums WHERE url LIKE '") +
                        subURL + QString("%'; "),
                        &values );
    }

    QStringList subalbums;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end(); ++it)
        subalbums << it->toString();
    return subalbums;
}

int AlbumDB::addAlbum(const QString &albumRoot, const QString& url,
                      const QString& caption,
                      const QDate& date, const QString& collection)
{
    d->db->execSql( QString("REPLACE INTO Albums (url, date, caption, collection) "
                            "VALUES(?, ?, ?, ?);"),
                    url,
                    date.toString(Qt::ISODate),
                    caption,
                    collection);

    int id = d->db->lastInsertedRow();
    return id;
}
#else
QList<AlbumShortInfo> AlbumDB::getAlbumShortInfos()
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT Albums.id, Albums.relativePath, AlbumRoots.absolutePath from Albums "
                            " LEFT OUTER JOIN AlbumRoots ON AlbumRoots.id=Albums.albumRoot; "),
                    &values);

    QList<AlbumShortInfo> albumList;

    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        AlbumShortInfo info;

        info.id        = (*it).toInt();
        ++it;
        info.url       = (*it).toString();
        ++it;
        info.albumRoot = (*it).toString();
        ++it;

        albumList << info;
    }

    return albumList;
}

QStringList AlbumDB::getSubalbumsForPath(const QString &albumRoot,
                                         const QString& path,
                                         bool onlyDirectSubalbums)
{
    CollectionLocation *location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);
    if (!location)
        return QStringList();

    QString subURL = path;
    if (!path.endsWith("/"))
        subURL += '/';
    subURL = (subURL);

    QList<QVariant> values;

    if (onlyDirectSubalbums)
    {
        d->db->execSql( QString("SELECT relativePath FROM Albums WHERE albumRoot=? AND relativePath LIKE '") +
                        subURL + QString("%' ") + QString("AND relativePath NOT LIKE '") +
                        subURL + QString("%/%'; "),
                        location->id(),
                        &values );
    }
    else
    {
        d->db->execSql( QString("SELECT relativePath FROM Albums WHERE albumRoot=? AND relativePath LIKE '") +
                        subURL + QString("%'; "),
                        location->id(),
                        &values );
    }

    QStringList subalbums;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end(); ++it)
        subalbums << location->albumRootPath() + it->toString();
    return subalbums;
}

int AlbumDB::addAlbum(const QString &albumRoot, const QString& relativePath,
                      const QString& caption,
                      const QDate& date, const QString& collection)
{
    CollectionLocation *location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);
    if (!location)
        return -1;

    QList<QVariant> boundValues;
    boundValues << location->id() << relativePath << date.toString(Qt::ISODate) << caption << collection;

    d->db->execSql( QString("REPLACE INTO Albums (albumRoot, relativePath, date, caption, collection) "
                            "VALUES(?, ?, ?, ?, ?);"),
                    boundValues);

    int id = d->db->lastInsertedRow();
    return id;
}

#endif

void AlbumDB::setAlbumCaption(int albumID, const QString& caption)
{
    d->db->execSql( QString("UPDATE Albums SET caption=? WHERE id=?;"),
                    caption, albumID );
}

void AlbumDB::setAlbumCollection(int albumID, const QString& collection)
{
    d->db->execSql( QString("UPDATE Albums SET collection=? WHERE id=?;"),
                    collection, albumID );
}

void AlbumDB::setAlbumDate(int albumID, const QDate& date)
{
    d->db->execSql( QString("UPDATE Albums SET date=? WHERE id=?;"),
                    date.toString(Qt::ISODate),
                    albumID );
}

void AlbumDB::setAlbumIcon(int albumID, qlonglong iconID)
{
    d->db->execSql( QString("UPDATE Albums SET icon=? WHERE id=?;"),
                    iconID,
                    albumID );
}

#ifdef OLDCODE
QString AlbumDB::getAlbumIcon(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT B.url, I.name \n "
                            "FROM Albums AS A \n "
                            "  LEFT OUTER JOIN Images AS I ON I.id=A.icon \n "
                            "  LEFT OUTER JOIN Albums AS B ON B.id=I.dirid \n "
                            "WHERE A.id=?;"),
                    albumID, &values );
    if (values.isEmpty())
        return QString();

    QList<QVariant>::iterator it = values.begin();
    QString url  = (*it).toString();
    ++it;
    QString icon = (*it).toString();
    if (icon.isEmpty())
        return QString();

    QString basePath(DatabaseAccess::albumRoot());
    basePath += url;
    basePath += '/' + icon;

    return basePath;
}
#else
QString AlbumDB::getAlbumIcon(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT B.relativePath, I.name, R.absolutePath \n "
                            "FROM Albums AS A \n "
                            "  LEFT OUTER JOIN Images AS I ON I.id=A.icon \n "
                            "  LEFT OUTER JOIN Albums AS B ON B.id=I.dirid \n "
                            "  LEFT OUTER JOIN AlbumRoots AS R ON R.id=B.albumRoot \n "
                            "WHERE A.id=?;"),
                    albumID, &values );
    if (values.isEmpty())
        return QString();

    QList<QVariant>::iterator it = values.begin();
    QString path = (*it).toString();
    ++it;
    QString icon = (*it).toString();
    ++it;
    if (icon.isEmpty())
        return QString();

    QString basePath = (*it).toString();
    basePath += path;
    basePath += '/' + icon;

    return basePath;
}
#endif

void AlbumDB::deleteAlbum(int albumID)
{
    d->db->execSql( QString("DELETE FROM Albums WHERE id=?;"),
                    albumID );
}

int AlbumDB::addTag(int parentTagID, const QString& name, const QString& iconKDE,
                    qlonglong iconID)
{
    if (!d->db->execSql( QString("INSERT INTO Tags (pid, name) "
                                 "VALUES( ?, ?);"),
                         parentTagID, 
                         name) )
    {
        return -1;
    }

    int id = d->db->lastInsertedRow();

    if (!iconKDE.isEmpty())
    {
        d->db->execSql( QString("UPDATE Tags SET iconkde=? WHERE id=?;"),
                        iconKDE,
                        id );
    }
    else
    {
        d->db->execSql( QString("UPDATE Tags SET icon=? WHERE id=?;"),
                        iconID,
                        id);
    }

    return id;
}

void AlbumDB::deleteTag(int tagID)
{
    d->db->execSql( QString("DELETE FROM Tags WHERE id=?;"),
                    tagID );
}

void AlbumDB::setTagIcon(int tagID, const QString& iconKDE, qlonglong iconID)
{
    if (!iconKDE.isEmpty())
    {
        d->db->execSql( QString("UPDATE Tags SET iconkde=?, icon=0 WHERE id=?;"),
                        iconKDE, tagID );
    }
    else
    {
        d->db->execSql( QString("UPDATE Tags SET icon=? WHERE id=?;"),
                        iconID, tagID );
    }
}

#ifdef OLDCODE
QString AlbumDB::getTagIcon(int tagID)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT A.url, I.name, T.iconkde \n "
                            "FROM Tags AS T \n "
                            "  LEFT OUTER JOIN Images AS I ON I.id=T.icon \n "
                            "  LEFT OUTER JOIN Albums AS A ON A.id=I.dirid \n "
                            "WHERE T.id=?;"),
                    tagID, &values );

    if (values.isEmpty())
        return QString();

    QString basePath(DatabaseAccess::albumRoot());

    QString iconName, iconKDE, albumURL, icon;

    QList<QVariant>::iterator it = values.begin();

    albumURL    = (*it).toString();
    ++it;
    iconName    = (*it).toString();
    ++it;
    iconKDE     = (*it).toString();
    ++it;

    if ( albumURL.isEmpty() )
    {
        icon = iconKDE;
    }
    else
    {
        icon = basePath + albumURL + '/' + iconName;
    }

    return icon;
}
#else
QString AlbumDB::getTagIcon(int tagID)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT A.relativePath, I.name, T.iconkde, R.absolutePath \n "
                            "FROM Tags AS T \n "
                            "  LEFT OUTER JOIN Images AS I ON I.id=T.icon \n "
                            "  LEFT OUTER JOIN Albums AS A ON A.id=I.dirid \n "
                            "  LEFT OUTER JOIN AlbumRoots AS R ON R.id=A.albumRoot \n "
                            "WHERE T.id=?;"),
                    tagID, &values );

    if (values.isEmpty())
        return QString();

    QString iconName, iconKDE, albumURL, icon, basePath;

    QList<QVariant>::iterator it = values.begin();

    albumURL    = (*it).toString();
    ++it;
    iconName    = (*it).toString();
    ++it;
    iconKDE     = (*it).toString();
    ++it;
    basePath    = (*it).toString();
    ++it;

    if ( albumURL.isEmpty() )
    {
        icon = iconKDE;
    }
    else
    {
        icon = basePath + albumURL + '/' + iconName;
    }

    return icon;
}
#endif

void AlbumDB::setTagParentID(int tagID, int newParentTagID)
{
    d->db->execSql( QString("UPDATE Tags SET pid=? WHERE id=?;"),
                    newParentTagID, tagID );
}

int AlbumDB::addSearch(const QString& name, const KUrl& url)
{
    if (!d->db->execSql(QString("INSERT INTO Searches (name, url) \n"
                                "VALUES(?, ?);"),
                        name, url.url()) )
    {
        return -1;
    }

    return d->db->lastInsertedRow();
}

void AlbumDB::updateSearch(int searchID, const QString& name,
               const KUrl& url)
{
    d->db->execSql(QString("UPDATE Searches SET name=?, url=? "
                            "WHERE id=?"),
                    name, url.url(), searchID);
}

void AlbumDB::deleteSearch(int searchID)
{
    d->db->execSql( QString("DELETE FROM Searches WHERE id=?"),
                    searchID );
}

void AlbumDB::setSetting(const QString& keyword,
                         const QString& value )
{
    d->db->execSql( QString("REPLACE into Settings VALUES (?,?);"),
                    keyword, value );
}

QString AlbumDB::getSetting(const QString& keyword)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT value FROM Settings "
                            "WHERE keyword=?;"),
                    keyword, &values );

    if (values.isEmpty())
        return QString();
    else
        return values.first().toString();
}

QString AlbumDB::getItemCaption(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT caption FROM Images "
                            "WHERE id=?;"),
                    imageID, &values );

    if (!values.isEmpty())
        return values.first().toString();
    else
        return QString();
}

QString AlbumDB::getItemCaption(int albumID, const QString& name)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT caption FROM Images "
                            "WHERE dirid=? AND name=?;"),
                    albumID,
                    name,
                    &values );

    if (!values.isEmpty())
        return values.first().toString();
    else
        return QString();
}

QDateTime AlbumDB::getItemDate(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT datetime FROM Images "
                            "WHERE id=?;"),
                    imageID,
                    &values );

    if (!values.isEmpty())
        return QDateTime::fromString(values.first().toString(), Qt::ISODate);
    else
        return QDateTime();
}

QDateTime AlbumDB::getItemDate(int albumID, const QString& name)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT datetime FROM Images "
                            "WHERE dirid=? AND name=?;"),
                    albumID,
                    name,
                    &values );

    if (values.isEmpty())
        return QDateTime::fromString(values.first().toString(), Qt::ISODate);
    else
        return QDateTime();
}

qlonglong AlbumDB::getImageId(int albumID, const QString& name)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT id FROM Images "
                            "WHERE dirid=? AND name=?;"),
                    albumID,
                    name,
                    &values );

    if (values.isEmpty())
        return -1;
    else
        return (values[0]).toLongLong();
}

QStringList AlbumDB::getItemTagNames(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT name FROM Tags \n "
                            "WHERE id IN (SELECT tagid FROM ImageTags \n "
                            "             WHERE imageid=?) \n "
                            "ORDER BY name;"),
                    imageID,
                    &values );

    QStringList names;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end(); ++it)
        names << it->toString();
    return names;
}

QList<int> AlbumDB::getItemTagIDs(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT tagid FROM ImageTags \n "
                            "WHERE imageID=?;"),
                    imageID,
                    &values );

    QList<int> ids;

    if (values.isEmpty())
        return ids;

    for (QList<QVariant>::iterator it=values.begin(); it != values.end(); ++it)
        ids << it->toInt();
    return ids;
}

#ifdef OLDCODE
ItemShortInfo AlbumDB::getItemShortInfo(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT Images.name, Albums.url, Albums.id "
                            "FROM Images "
                            "LEFT OUTER JOIN Albums "
                            "ON Albums.id=Images.dirid "
                            "WHERE Images.id=?;"),
                    imageID,
                    &values );

    ItemShortInfo info;

    if (!values.isEmpty())
    {
        info.id        = imageID;
        info.itemName  = values[0].toString();
        info.albumRoot = DatabaseAccess::albumRoot();
        info.album     = values[1].toString();
        info.albumID   = values[2].toInt();
    }

    return info;
}
#else
ItemShortInfo AlbumDB::getItemShortInfo(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT Images.name, AlbumRoots.absolutePath, Albums.relativePath, Albums.id "
                            "FROM Images "
                            "  LEFT OUTER JOIN Albums ON Albums.id=Images.dirid "
                            "  LEFT OUTER JOIN AlbumRoots ON AlbumRoots.id=Albums.albumRoot "
                            "WHERE Images.id=?;"),
                    imageID,
                    &values );

    ItemShortInfo info;

    if (!values.isEmpty())
    {
        info.id        = imageID;
        info.itemName  = values[0].toString();
        info.albumRoot = values[1].toString();
        info.album     = values[2].toString();
        info.albumID   = values[3].toInt();
    }

    return info;
}
#endif

bool AlbumDB::hasTags(const QList<qlonglong>& imageIDList)
{
    QList<int> ids;

    if (imageIDList.isEmpty())
        return false;

    QList<QVariant> values;
    QList<QVariant> boundValues;

    QString sql = QString("SELECT count(tagid) FROM ImageTags "
                          "WHERE imageid=? ");
    boundValues << imageIDList.first();

    QList<qlonglong>::const_iterator it = imageIDList.begin();
    ++it;
    for (; it != imageIDList.end(); ++it)
    {
        sql += QString(" OR imageid=? ");
        boundValues << (*it);
        ++it;
    }

    sql += QString(";");
    d->db->execSql( sql, boundValues, &values );

    if (values.isEmpty() || values.first().toInt() == 0)
        return false;
    else
        return true;
}

QList<int> AlbumDB::getItemCommonTagIDs(const QList<qlonglong>& imageIDList)
{
    QList<int> ids;

    if (imageIDList.isEmpty())
        return ids;

    QList<QVariant> values;
    QList<QVariant> boundValues;

    QString sql = QString("SELECT DISTINCT tagid FROM ImageTags "
                          "WHERE imageid=? ");
    boundValues << imageIDList.first();

    QList<qlonglong>::const_iterator it = imageIDList.begin();
    ++it;
    for (; it != imageIDList.end(); ++it)
    {
        sql += QString(" OR imageid=? ");
        boundValues << (*it);
        ++it;
    }

    sql += QString(";");
    d->db->execSql( sql, boundValues, &values );

    if (values.isEmpty())
        return ids;

    for (QList<QVariant>::iterator it=values.begin(); it != values.end(); ++it)
        ids << it->toInt();
    return ids;
}

void AlbumDB::setItemCaption(qlonglong imageID,const QString& caption)
{
    QList<QVariant> values;

    d->db->execSql( QString("UPDATE Images SET caption=? "
                            "WHERE id=?;"),
                    caption,
                    imageID );

    DatabaseAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageComment);
}

void AlbumDB::setItemCaption(int albumID, const QString& name, const QString& caption)
{
    /*
    QList<QVariant> values;

    d->db->execSql( QString("UPDATE Images SET caption=? "
                     "WHERE dirid=? AND name=?;")
             .(caption,
                  QString::number(albumID),
                  (name)) );
    */

    // easier because of attributes watch
    return setItemCaption(getImageId(albumID, name), caption);
}

void AlbumDB::addItemTag(qlonglong imageID, int tagID)
{
    d->db->execSql( QString("REPLACE INTO ImageTags (imageid, tagid) "
                            "VALUES(?, ?);"),
                    imageID,
                    tagID );

    DatabaseAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageTags);

    if (!d->recentlyAssignedTags.contains(tagID))
    {
        d->recentlyAssignedTags.push_front(tagID);
        if (d->recentlyAssignedTags.size() > 10)
            d->recentlyAssignedTags.pop_back();
    }
}

void AlbumDB::addItemTag(int albumID, const QString& name, int tagID)
{
    /*
    d->db->execSql( QString("REPLACE INTO ImageTags (imageid, tagid) \n "
                     "(SELECT id, ? FROM Images \n "
                     " WHERE dirid=? AND name=?);")
             .tagID
             .albumID
             .(name) );
    */

    // easier because of attributes watch
    return addItemTag(getImageId(albumID, name), tagID);
}

QList<int> AlbumDB::getRecentlyAssignedTags() const
{
    return d->recentlyAssignedTags;    
}

void AlbumDB::removeItemTag(qlonglong imageID, int tagID)
{
    d->db->execSql( QString("DELETE FROM ImageTags "
                            "WHERE imageID=? AND tagid=?;"),
                    imageID,
                    tagID );

    DatabaseAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageTags);
}

void AlbumDB::removeItemAllTags(qlonglong imageID)
{
    d->db->execSql( QString("DELETE FROM ImageTags "
                            "WHERE imageID=?;"),
                    imageID );

    DatabaseAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageTags);
}

QStringList AlbumDB::getItemNamesInAlbum(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT Images.name "
                            "FROM Images "
                            "WHERE Images.dirid=?"),
                    albumID, &values );

    QStringList names;
    for (QList<QVariant>::iterator it=values.begin(); it != values.end(); ++it)
        names << it->toString();
    return names;
}

#ifdef OLDCODE
QStringList AlbumDB::getAllItemURLsWithoutDate()
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT Albums.url||'/'||Images.name "
                            "FROM Images, Albums "
                            "WHERE Images.dirid=Albums.Id "
                            "AND (Images.datetime is null or "
                            "     Images.datetime == '');"),
                    &values );

    QString libraryPath = DatabaseAccess::albumRoot() + '/';
    QStringList urls;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end();++it)
        urls << libraryPath + it->toString();
    return urls;
}
#else
QStringList AlbumDB::getAllItemURLsWithoutDate()
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT AlbumRoots.absolutePath||Albums.relativePath||'/'||Images.name "
                            "FROM Images "
                            "  LEFT OUTER JOIN Albums ON Images.dirid=Albums.id "
                            "  LEFT OUTER JOIN AlbumRoots ON AlbumRoots.id=Albums.albumRoot "
                            "WHERE (Images.datetime is null or "
                            "       Images.datetime == '');"),
                    &values );

    QStringList urls;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end(); ++it)
        urls << it->toString();

    return urls;
}
#endif

QList<QPair<QString, QDateTime> > AlbumDB::getItemsAndDate()
{
    QList<QVariant> values;
    d->db->execSql( "SELECT name, datetime FROM Images;", &values );

    QList<QPair<QString, QDateTime> > data;
    for ( QList<QVariant>::iterator it = values.begin(); it != values.end(); )
    {
        QPair<QString, QDateTime> pair;
        pair.first  = (*it).toString();
        ++it;
        pair.second = QDateTime::fromString( (*it).toString(),  Qt::ISODate );
        ++it;

        if (!pair.second.isValid())
            continue;

        data << pair;
    }
    return data;
}

#ifdef OLDCODE
int AlbumDB::getAlbumForPath(const QString &albumRoot, const QString& folder, bool create)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT id FROM Albums WHERE url =?;"),
                    folder, &values);

    int albumID = -1;
    if (values.isEmpty())
    {
        if (create)
            albumID = addAlbum(albumRoot, folder, QString(), QDate::currentDate(), QString());
    }
    else
    {
        albumID = values.first().toInt();
    }

    return albumID;
}
#else
int AlbumDB::getAlbumForPath(const QString &albumRoot, const QString& folder, bool create)
{
    CollectionLocation *location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);
    if (!location)
        return -1;

    QList<QVariant> values;
    d->db->execSql( QString("SELECT id FROM Albums WHERE albumRoot=? AND relativePath=?;"),
                    location->id(), folder, &values);

    int albumID = -1;
    if (values.isEmpty())
    {
        if (create)
            albumID = addAlbum(albumRoot, folder, QString(), QDate::currentDate(), QString());
    }
    else
    {
        albumID = values.first().toInt();
    }

    return albumID;
}
#endif

qlonglong AlbumDB::addItem(int albumID,
                         const QString& name,
                         const QDateTime& datetime,
                         const QString& comment,
                         int rating,
                         const QStringList &keywordsList)
{
    d->db->execSql ( QString ("REPLACE INTO Images "
                            "( caption , datetime, name, dirid ) "
                            " VALUES (?,?,?,?) " ),
                    comment,
                    datetime.toString(Qt::ISODate),
                    name,
                    albumID );

    qlonglong item = d->db->lastInsertedRow();

    // Set Rating value to item in database.

    if ( item != -1 && rating != -1 )
        setItemRating(item, rating);

    // Set existing tags in database or create new tags if not exist.

    if ( item != -1 && !keywordsList.isEmpty() )
    {
        QList<int> tagIDs = getTagsFromTagPaths(keywordsList, true);
        for (QList<int>::iterator it = tagIDs.begin(); it != tagIDs.end(); ++it)
        {
            addItemTag(item, *it);
        }
    }

    return item;
}

QList<int> AlbumDB::getTagsFromTagPaths(const QStringList &keywordsList, bool create)
{
    if (keywordsList.isEmpty())
        return QList<int>();

    QList<int> tagIDs;

    QStringList keywordsList2Create;

    // Create a list of the tags currently in database

    TagInfo::List currentTagsList;

    QList<QVariant> values;
    d->db->execSql( "SELECT id, pid, name FROM Tags;", &values );

    for (QList<QVariant>::iterator it = values.begin(); it != values.end();)
    {
        TagInfo info;

        info.id   = (*it).toInt();
        ++it;
        info.pid  = (*it).toInt();
        ++it;
        info.name = (*it).toString();
        ++it;
        currentTagsList.append(info);
    }

    // For every tag in keywordsList, scan taglist to check if tag already exists.

    for (QStringList::const_iterator kwd = keywordsList.begin();
        kwd != keywordsList.end(); ++kwd )
    {
        // split full tag "url" into list of single tag names
        QStringList tagHierarchy = (*kwd).split('/', QString::SkipEmptyParts);
        if (tagHierarchy.isEmpty())
            continue;

        // last entry in list is the actual tag name
        bool foundTag   = false;
        QString tagName = tagHierarchy.back();
        tagHierarchy.pop_back();

        for (TagInfo::List::iterator tag = currentTagsList.begin();
            tag != currentTagsList.end(); ++tag )
        {
            // There might be multiple tags with the same name, but in different
            // hierarchies. We must check them all until we find the correct hierarchy
            if ((*tag).name == tagName)
            {
                int parentID = (*tag).pid;

                // Check hierarchy, from bottom to top
                bool foundParentTag                 = true;
                QStringList::iterator parentTagName = tagHierarchy.end();

                while (foundParentTag && parentTagName != tagHierarchy.begin())
                {
                    --parentTagName;

                    foundParentTag = false;

                    for (TagInfo::List::iterator parentTag = currentTagsList.begin();
                        parentTag != currentTagsList.end(); ++parentTag )
                    {
                        // check if name is the same, and if ID is identical
                        // to the parent ID we got from the child tag
                        if ( (*parentTag).id == parentID &&
                            (*parentTag).name == (*parentTagName) )
                        {
                            parentID       = (*parentTag).pid;
                            foundParentTag = true;
                            break;
                        }
                    }

                    // If we traversed the list without a match,
                    // foundParentTag will be false, the while loop breaks.
                }

                // If we managed to traverse the full hierarchy,
                // we have our tag.
                if (foundParentTag)
                {
                    // add to result list
                    tagIDs.append((*tag).id);
                    foundTag = true;
                    break;
                }
            }
        }

        if (!foundTag)
            keywordsList2Create.append(*kwd);
    }

    // If tags do not exist in database, create them.

    if (create && !keywordsList2Create.isEmpty())
    {
        for (QStringList::iterator kwd = keywordsList2Create.begin();
            kwd != keywordsList2Create.end(); ++kwd )
        {
            // split full tag "url" into list of single tag names
            QStringList tagHierarchy = (*kwd).split('/', QString::SkipEmptyParts);

            if (tagHierarchy.isEmpty())
                continue;

            int  parentTagID      = 0;
            int  tagID            = 0;
            bool parentTagExisted = true;

            // Traverse hierarchy from top to bottom
            for (QStringList::iterator tagName = tagHierarchy.begin();
                tagName != tagHierarchy.end(); ++tagName)
            {
                tagID = 0;

                // if the parent tag did not exist, we need not check if the child exists
                if (parentTagExisted)
                {
                    for (TagInfo::List::iterator tag = currentTagsList.begin();
                        tag != currentTagsList.end(); ++tag )
                    {
                        // find the tag with tag name according to tagHierarchy,
                        // and parent ID identical to the ID of the tag we found in
                        // the previous run.
                        if ((*tag).name == (*tagName) && (*tag).pid == parentTagID)
                        {
                            tagID = (*tag).id;
                            break;
                        }
                    }
                }

                if (tagID != 0)
                {
                    // tag already found in DB
                    parentTagID = tagID;
                    continue;
                }

                // Tag does not yet exist in DB, add it
                tagID = addTag(parentTagID, (*tagName), QString(), 0);

                if (tagID == -1)
                {
                    // Something is wrong in database. Abort.
                    break;
                }

                // append to our list of existing tags (for following keywords)
                TagInfo info;
                info.id   = tagID;
                info.pid  = parentTagID;
                info.name = (*tagName);
                currentTagsList.append(info);

                parentTagID      = tagID;
                parentTagExisted = false;
            }

            // add to result list
            tagIDs.append(tagID);
        }
    }

    return tagIDs;
}

int AlbumDB::getItemAlbum(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql ( QString ("SELECT dirid FROM Images "
                            "WHERE id=?;"),
                     imageID,
                     &values);

    if (!values.isEmpty())
        return values.first().toInt();
    else
        return 1;
}

QString AlbumDB::getItemName(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql ( QString ("SELECT name FROM Images "
                            "WHERE id=?;"),
                     imageID,
                     &values);

    if (!values.isEmpty())
        return values.first().toString();
    else
        return QString();
}

bool AlbumDB::setItemDate(qlonglong imageID,
                          const QDateTime& datetime)
{
    d->db->execSql ( QString ("UPDATE Images SET datetime=?"
                            "WHERE id=?;"),
                     datetime.toString(Qt::ISODate),
                     imageID );

    DatabaseAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageDate);

    return true;
}

bool AlbumDB::setItemDate(int albumID, const QString& name,
                          const QDateTime& datetime)
{
    /*
    d->db->execSql ( QString ("UPDATE Images SET datetime=?"
                       "WHERE dirid=? AND name=?;")
              .datetime.toString(Qt::ISODate,
                   QString::number(albumID),
                   (name)) );

    return true;
    */
    // easier because of attributes watch
    return setItemDate(getImageId(albumID, name), datetime);
}

void AlbumDB::setItemRating(qlonglong imageID, int rating)
{
    d->db->execSql ( QString ("REPLACE INTO ImageProperties "
                            "(imageid, property, value) "
                            "VALUES(?, ?, ?);"),
                     imageID,
                     QString("Rating"),
                     rating );

    DatabaseAccess::attributesWatch()
            ->sendImageFieldChanged(imageID, DatabaseAttributesWatch::ImageRating);
}

int AlbumDB::getItemRating(qlonglong imageID)
{
    QList<QVariant> values;

    d->db->execSql( QString("SELECT value FROM ImageProperties "
                            "WHERE imageid=? and property=?;"),
                    imageID,
                    QString("Rating"),
                    &values);

    if (!values.isEmpty())
        return values.first().toInt();
    else
        return 0;
}

#ifdef OLDCODE
QStringList AlbumDB::getItemURLsInAlbum(int albumID, ItemSortOrder sortOrder)
{
    QList<QVariant> values;

    QString basePath(DatabaseAccess::albumRoot());

    QString sqlString;
    switch(sortOrder)
    {
        case ByItemName:
            sqlString = QString("SELECT Albums.url||'/'||Images.name FROM Images, Albums "
                                 "WHERE Albums.id=? AND Albums.id=Images.dirid "
                                 "ORDER BY Images.name COLLATE NOCASE;");
            break;
        case ByItemPath:
            // Dont collate on the path - this is to maintain the same behaviour
            // that happens when sort order is "By Path"
            sqlString = QString("SELECT Albums.url||'/'||Images.name FROM Images, Albums "
                                 "WHERE Albums.id=? AND Albums.id=Images.dirid "
                                 "ORDER BY Albums.url,Images.name;");
            break;
        case ByItemDate:
            sqlString = QString("SELECT Albums.url||'/'||Images.name FROM Images, Albums "
                                 "WHERE Albums.id=? AND Albums.id=Images.dirid "
                                 "ORDER BY Images.datetime;");
            break;
        case ByItemRating:
            sqlString = QString("SELECT Albums.url||'/'||Images.name FROM Images, Albums, ImageProperties "
                                 "WHERE Albums.id=? AND Albums.id=Images.dirid "
                                 "AND Images.id = ImageProperties.imageid "
                                 "AND ImageProperties.property='Rating' "
                                 "ORDER BY ImageProperties.value DESC;");
            break;
        case NoItemSorting:
        default:
            sqlString = QString("SELECT Albums.url||'/'||Images.name FROM Images, Albums "
                                 "WHERE Albums.id=? AND Albums.id=Images.dirid;");
            break;
    }
    // all statements take one bound values
    d->db->execSql(sqlString, albumID, &values);

    QStringList urls;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end(); ++it)
        urls << basePath + it->toString();

    return urls;
}
#else
QStringList AlbumDB::getItemURLsInAlbum(int albumID, ItemSortOrder sortOrder)
{
    QList<QVariant> values;

    QString sqlString;
    switch(sortOrder)
    {
        case ByItemName:
            sqlString = QString("SELECT AlbumRoots.absolutePath||Albums.relativePath||'/'||Images.name "
                                 "FROM Images "
                                 "  LEFT OUTER JOIN Albums ON Albums.id=Images.dirid "
                                 "  LEFT OUTER JOIN AlbumRoots ON AlbumRoots.id=Albums.albumRoot "
                                 "WHERE Albums.id=? "
                                 "ORDER BY Images.name COLLATE NOCASE;");
            break;
        case ByItemPath:
            // Dont collate on the path - this is to maintain the same behaviour
            // that happens when sort order is "By Path"
            sqlString = QString("SELECT AlbumRoots.absolutePath||Albums.relativePath||'/'||Images.name "
                                 "FROM Images "
                                 "  LEFT OUTER JOIN Albums ON Albums.id=Images.dirid "
                                 "  LEFT OUTER JOIN AlbumRoots ON AlbumRoots.id=Albums.albumRoot "
                                 "WHERE Albums.id=? "
                                 "ORDER BY Albums.relativePath,Images.name;");
            break;
        case ByItemDate:
            sqlString = QString("SELECT AlbumRoots.absolutePath||Albums.relativePath||'/'||Images.name "
                                 "FROM Images "
                                 "  LEFT OUTER JOIN Albums ON Albums.id=Images.dirid "
                                 "  LEFT OUTER JOIN AlbumRoots ON AlbumRoots.id=Albums.albumRoot "
                                 "WHERE Albums.id=? "
                                 "ORDER BY Images.datetime;");
            break;
        case ByItemRating:
            sqlString = QString("SELECT AlbumRoots.absolutePath||Albums.relativePath||'/'||Images.name "
                                 "FROM Images "
                                 "  LEFT OUTER JOIN Albums ON Albums.id=Images.dirid "
                                 "  LEFT OUTER JOIN AlbumRoots ON AlbumRoots.id=Albums.albumRoot "
                                 "  LEFT OUTER JOIN ImageProperties.imageid=Images.id "
                                 "WHERE Albums.id=? "
                                 "AND ImageProperties.property='Rating' "
                                 "ORDER BY ImageProperties.value DESC;");
            break;
        case NoItemSorting:
        default:
            sqlString = QString("SELECT AlbumRoots.absolutePath||Albums.relativePath||'/'||Images.name "
                                 "FROM Images "
                                 "  LEFT OUTER JOIN Albums ON Albums.id=Images.dirid "
                                 "  LEFT OUTER JOIN AlbumRoots ON AlbumRoots.id=Albums.albumRoot "
                                 "WHERE Albums.id=?;");
            break;
    }
    // all statements take one bound values
    d->db->execSql(sqlString, albumID, &values);

    QStringList urls;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end(); ++it)
        urls << it->toString();

    return urls;
}
#endif

QList<qlonglong> AlbumDB::getItemIDsInAlbum(int albumID)
{
    QList<qlonglong> itemIDs;
    QList<QVariant> values;

    d->db->execSql( QString("SELECT id FROM Images WHERE dirid=?;"),
             albumID, &values );

    for (QList<QVariant>::iterator it = values.begin(); it != values.end(); ++it)
    {
        itemIDs << (*it).toLongLong();
    }

    return itemIDs;
}

#ifdef OLDCODE
QStringList AlbumDB::getItemURLsInTag(int tagID, bool recursive)
{
    QList<QVariant> values;

    QString basePath(DatabaseAccess::albumRoot());

    QString imagesIdClause;
    QList<QVariant> boundValues;
    if (recursive)
    {
        imagesIdClause = QString("SELECT imageid FROM ImageTags "
                                 " WHERE tagid=? "
                                 " OR tagid IN (SELECT id FROM TagsTree WHERE pid=?)");
        boundValues << tagID << tagID;
    }
    else
    {
        imagesIdClause = QString("SELECT imageid FROM ImageTags WHERE tagid=?");
        boundValues << tagID;
    }

    d->db->execSql( QString("SELECT Albums.url||'/'||Images.name FROM Images, Albums "
                            "WHERE Images.id IN (%1) "
                            "AND Albums.id=Images.dirid;")
                    .arg(imagesIdClause), boundValues, &values );

    QStringList urls;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end(); ++it)
        urls << basePath + it->toString();

    return urls;
}
#else
QStringList AlbumDB::getItemURLsInTag(int tagID, bool recursive)
{
    QList<QVariant> values;

    QString imagesIdClause;
    QList<QVariant> boundValues;
    if (recursive)
    {
        imagesIdClause = QString("SELECT imageid FROM ImageTags "
                                 " WHERE tagid=? "
                                 " OR tagid IN (SELECT id FROM TagsTree WHERE pid=?)");
        boundValues << tagID << tagID;
    }
    else
    {
        imagesIdClause = QString("SELECT imageid FROM ImageTags WHERE tagid=?");
        boundValues << tagID;
    }

    d->db->execSql( QString("SELECT AlbumRoots.absolutePath||Albums.relativePath||'/'||Images.name "
                            "FROM Images "
                            "  LEFT OUTER JOIN Albums ON Albums.id=Images.dirid "
                            "  LEFT OUTER JOIN AlbumRoots ON AlbumRoots.id=Albums.albumRoot "
                            "WHERE Images.id IN (%1);")
                    .arg(imagesIdClause), boundValues, &values );

    QStringList urls;
    for (QList<QVariant>::iterator it = values.begin(); it != values.end(); ++it)
        urls << it->toString();

    return urls;
}
#endif

QList<qlonglong> AlbumDB::getItemIDsInTag(int tagID, bool recursive)
{
    QList<qlonglong> itemIDs;
    QList<QVariant> values;

    if (recursive)
        d->db->execSql( QString("SELECT imageid FROM ImageTags "
                                " WHERE tagid=? "
                                " OR tagid IN (SELECT id FROM TagsTree WHERE pid=?);"),
                        tagID, tagID, &values );
    else
        d->db->execSql( QString("SELECT imageid FROM ImageTags WHERE tagid=?;"),
                 tagID, &values );

    for (QList<QVariant>::iterator it = values.begin(); it != values.end(); ++it)
    {
        itemIDs << (*it).toLongLong();
    }

    return itemIDs;
}

#ifdef OLDCODE
QString AlbumDB::getAlbumPath(int albumID)
{
    return getAlbumRelativePath(albumID);
}

QString AlbumDB::getAlbumRelativePath(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT url from Albums WHERE id=?"),
                    albumID, &values);
    if (!values.isEmpty())
        return values.first().toString();
    else
        return QString();
}
#else
QString AlbumDB::getAlbumPath(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT AlbumRoots.absolutePath||Albums.relativePath "
                            "FROM Albums, AlbumRoots WHERE Albums.id=? AND AlbumRoots.id=Albums.albumRoot; "),
                    albumID, &values);
    if (!values.isEmpty())
        return values.first().toString();
    else
        return QString();
}

QString AlbumDB::getAlbumRelativePath(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT relativePath from Albums WHERE id=?"),
                    albumID, &values);
    if (!values.isEmpty())
        return values.first().toString();
    else
        return QString();
}

int AlbumDB::getAlbumRootId(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT albumRoot FROM Albums WHERE id=?; "),
                    albumID, &values);
    if (!values.isEmpty())
        return values.first().toInt();
    else
        return -1;
}

#endif

QDate AlbumDB::getAlbumLowestDate(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT MIN(datetime) FROM Images "
                            "WHERE dirid=? GROUP BY dirid"),
                    albumID, &values );
    if (!values.isEmpty())
        return QDate::fromString( values.first().toString(), Qt::ISODate );
    else
        return QDate();
}

QDate AlbumDB::getAlbumHighestDate(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT MAX(datetime) FROM Images "
                            "WHERE dirid=? GROUP BY dirid"),
                    albumID , &values );
    if (!values.isEmpty())
        return QDate::fromString( values.first().toString(), Qt::ISODate );
    else
        return QDate();
}

QDate AlbumDB::getAlbumAverageDate(int albumID)
{
    QList<QVariant> values;
    d->db->execSql( QString("SELECT datetime FROM Images WHERE dirid=?"),
                    albumID , &values);

    int differenceInSecs = 0;
    int amountOfImages = 0;
    QDateTime baseDateTime;

    for (QList<QVariant>::iterator it = values.begin(); it != values.end(); ++it)
    {
        QDateTime itemDateTime = QDateTime::fromString( (*it).toString(), Qt::ISODate );
        if (itemDateTime.isValid())
        {
            ++amountOfImages;
            if ( baseDateTime.isNull() )
                baseDateTime=itemDateTime;
            else
                differenceInSecs += itemDateTime.secsTo( baseDateTime );
        }
    }

    if ( amountOfImages > 0 )
    {
        QDateTime averageDateTime;
        averageDateTime.setTime_t( baseDateTime.toTime_t() -
                                   (int)( differenceInSecs/amountOfImages ) );
        return ( averageDateTime.date() );
    }
    else
        return QDate();
}

void AlbumDB::deleteItem(int albumID, const QString& file)
{
    d->db->execSql( QString("DELETE FROM Images "
                            "WHERE dirid=? AND name=?;"),
                    albumID, file );
}

#ifdef OLDCODE
void AlbumDB::renameAlbum(int albumID, const QString& newUrl, bool renameSubalbums)
{
    QString oldUrl = getAlbumPath(albumID);

    // first delete any stale albums left behind
    d->db->execSql( QString("DELETE FROM Albums WHERE url = ?"),
                    newUrl );

    // now update the album url
    d->db->execSql( QString("UPDATE Albums SET relativePath = ? WHERE id = ?;"),
                    newUrl, albumID );

    if (renameSubalbums)
    {
        // now find the list of all subalbums which need to be updated
        QList<QVariant> values;
        d->db->execSql( QString("SELECT url FROM Albums WHERE url LIKE '?/%';"),
                        oldUrl, &values );

        // and update their url
        QString newChildURL;
        for (QList<QVariant>::iterator it = values.begin(); it != values.end(); ++it)
        {
            newChildURL = (*it).toString();
            newChildURL.replace(oldUrl, newUrl);
            d->db->execSql(QString("UPDATE Albums SET url=? WHERE url=?"),
                           newChildURL, (*it) );
        }
    }
}
#else
void AlbumDB::renameAlbum(int albumID, const QString& newRelativePath, bool renameSubalbums)
{
    int albumRoot  = getAlbumRootId(albumID);
    QString oldUrl = getAlbumRelativePath(albumID);

    if (oldUrl == newRelativePath)
        return;

    // first delete any stale albums left behind
    d->db->execSql( QString("DELETE FROM Albums WHERE relativePath=? AND albumRoot=?;"),
                    newRelativePath, albumRoot );

    // now update the album url
    d->db->execSql( QString("UPDATE Albums SET relativePath = ? WHERE id = ? AND albumRoot=?;"),
                    newRelativePath, albumID, albumRoot );

    if (renameSubalbums)
    {
        // now find the list of all subalbums which need to be updated
        QList<QVariant> values;
        d->db->execSql( QString("SELECT relativePath FROM Albums WHERE albumRoot=? AND relativePath LIKE '?/%';"),
                        albumRoot, oldUrl, &values );

        // and update their url
        QString newChildURL;
        for (QList<QVariant>::iterator it = values.begin(); it != values.end(); ++it)
        {
            newChildURL = (*it).toString();
            newChildURL.replace(oldUrl, newRelativePath);
            d->db->execSql(QString("UPDATE Albums SET url=? WHERE albumRoot=? AND url=?"),
                           newChildURL, albumRoot, (*it) );
        }
    }
}
#endif

void AlbumDB::setTagName(int tagID, const QString& name)
{
    d->db->execSql( QString("UPDATE Tags SET name=? WHERE id=?;"),
                    name, tagID );
}

void AlbumDB::moveItem(int srcAlbumID, const QString& srcName,
                       int dstAlbumID, const QString& dstName)
{

    // first delete any stale database entries if any
    deleteItem(dstAlbumID, dstName);

    d->db->execSql( QString("UPDATE Images SET dirid=?, name=? "
                            "WHERE dirid=? AND name=?;"),
                    dstAlbumID, dstName, srcAlbumID, srcName );
}

int AlbumDB::copyItem(int srcAlbumID, const QString& srcName,
                      int dstAlbumID, const QString& dstName)
{
    // check for src == dest
    if (srcAlbumID == dstAlbumID && srcName == dstName)
        return -1;

    // find id of src image
    QList<QVariant> values;
    d->db->execSql( QString("SELECT id FROM Images "
                            "WHERE dirid=? AND name=?;"),
                    srcAlbumID, srcName,
                    &values );

    if (values.isEmpty())
        return -1;

    int srcId = values.first().toInt();

    // first delete any stale database entries if any
    deleteItem(dstAlbumID, dstName);

    // copy entry in Images table
    d->db->execSql( QString("INSERT INTO Images (dirid, name, caption, datetime) "
                            "SELECT ?, ?, caption, datetime FROM Images "
                            "WHERE id=?;"),
                    dstAlbumID, dstName, srcId );

    int dstId = d->db->lastInsertedRow();

    // copy tags
    d->db->execSql( QString("INSERT INTO ImageTags (imageid, tagid) "
                            "SELECT ?, tagid FROM ImageTags "
                            "WHERE imageid=?;"),
                    dstId, srcId );

    // copy properties (rating)
    d->db->execSql( QString("INSERT INTO ImageProperties (imageid, property, value) "
                            "SELECT ?, property, value FROM ImageProperties "
                            "WHERE imageid=?;"),
                    dstId, srcId );

    return dstId;
}

bool AlbumDB::copyAlbumProperties(int srcAlbumID, int dstAlbumID)
{
    if (srcAlbumID == dstAlbumID)
        return true;

    QList<QVariant> values;
    d->db->execSql( QString("SELECT date, caption, collection, icon "
                            "FROM Albums WHERE id=?;"),
                    srcAlbumID,
                    &values );

    if (values.isEmpty())
    {
        DWarning() << " src album ID " << srcAlbumID << " does not exist" << endl;
        return false;
    }

    QList<QVariant> boundValues;
    boundValues << values[0] << values[1] << values[2] << values[3];
    boundValues << dstAlbumID;

    d->db->execSql( QString("UPDATE Albums SET date=?, caption=?, "
                            "collection=?, icon=? WHERE id=?"),
                    boundValues );
    return true;
}

}  // namespace Digikam
