/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-26
 * Description : Albums lister.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007 by Arnd Baecker <arnd dot baecker at web dot de>
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
#include <sys/time.h>
}

// C++ includes.

#include <cstdio>
#include <ctime>

// Qt includes.

#include <QDataStream>
#include <QFileInfo>
#include <QDir>
#include <QMap>
#include <QPair>
#include <QTimer>

// KDE includes.

#include <kdebug.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kio/job.h>
#include <kurl.h>

// Local includes.

#include "databaseaccess.h"
#include "databasewatch.h"
#include "imagelister.h"
#include "mimefilter.h"
#include "album.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "albumlister.h"
#include "albumlister.moc"

namespace Digikam
{

class AlbumListerPriv
{
public:

    AlbumListerPriv()
    {
        untaggedFilter = false;
        ratingFilter   = 0;
        filterTimer    = 0;
        refreshTimer   = 0;
        job            = 0;
        currAlbum      = 0;
        mimeTypeFilter = MimeFilter::AllFiles;
        ratingCond     = AlbumLister::GreaterEqualCondition;
        matchingCond   = AlbumLister::OrCondition;
        recurseAlbums  = false;
        recurseTags    = false;
    }

    bool                            untaggedFilter;

    int                             ratingFilter;
    int                             recurseAlbums;
    int                             recurseTags;

    QString                         textFilter;

    QMap<qlonglong, ImageInfo>      itemMap;
    QMap<QDateTime, bool>           dayFilter;
    QSet<int>                       invalidatedItems;

    QList<int>                      tagFilter;

    QTimer                         *filterTimer;
    QTimer                         *refreshTimer;

    KIO::TransferJob               *job;

    ImageInfoList                   itemList;

    QSet<qlonglong>                 itemListSet;

    Album                          *currAlbum;

    MimeFilter::TypeMimeFilter      mimeTypeFilter;

    AlbumLister::MatchingCondition  matchingCond;

    AlbumLister::RatingCondition    ratingCond;
};

AlbumLister* AlbumLister::m_instance = 0;

AlbumLister* AlbumLister::instance()
{
    if (!m_instance)
        new AlbumLister();

    return m_instance;
}

void AlbumLister::cleanUp()
{
    delete m_instance;
}

AlbumLister::AlbumLister()
{
    m_instance = this;

    d = new AlbumListerPriv;
    d->filterTimer = new QTimer(this);
    d->refreshTimer = new QTimer(this);
    d->refreshTimer->setSingleShot(true);

    connect(d->filterTimer, SIGNAL(timeout()),
            this, SLOT(slotFilterItems()));

    connect(d->refreshTimer, SIGNAL(timeout()),
            this, SLOT(slotNextRefresh()));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(collectionImageChange(const CollectionImageChangeset &)),
            this, SLOT(slotCollectionImageChange(const CollectionImageChangeset &)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(searchChange(const SearchChangeset &)),
            this, SLOT(slotSearchChange(const SearchChangeset &)));
}

AlbumLister::~AlbumLister()
{
    delete d;
    m_instance = 0;
}

void AlbumLister::openAlbum(Album *album)
{
    d->currAlbum = album;
    d->filterTimer->stop();
    emit signalClear();
    d->itemList.clear();
    d->itemListSet.clear();
    d->itemMap.clear();

    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }

    if (!album)
        return;

    startListJob(album->databaseUrl());
}

void AlbumLister::refresh()
{
    if (!d->currAlbum)
        return;

    d->filterTimer->stop();

    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }

    d->itemMap.clear();
    for (ImageInfoList::const_iterator it = d->itemList.begin(); it != d->itemList.end(); ++it)
    {
        d->itemMap.insert(it->id(), *it);
    }

    startListJob(d->currAlbum->databaseUrl());
}

void AlbumLister::slotNextRefresh()
{
    // Refresh, unless job is running, then postpone restart until job is finished
    // Rationale: Let the job run, don't stop it possibly several times
    if (d->job)
        d->refreshTimer->start(50);
    else
        refresh();
}

void AlbumLister::startListJob(const KUrl &url)
{
    d->job = ImageLister::startListJob(url);
    d->job->addMetaData("listAlbumsRecursively", d->recurseAlbums ? "true" : "false");
    d->job->addMetaData("listTagsRecursively", d->recurseTags ? "true" : "false");

    connect(d->job, SIGNAL(result(KJob*)),
            this, SLOT(slotResult(KJob*)));

    connect(d->job, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotData(KIO::Job*, const QByteArray&)));
}

void AlbumLister::setRecurseAlbums(bool recursive)
{
    d->recurseAlbums = recursive;
    refresh();
}

void AlbumLister::setRecurseTags(bool recursive)
{
    d->recurseTags = recursive;
    refresh();
}

void AlbumLister::setDayFilter(const QList<QDateTime>& days)
{
    d->dayFilter.clear();

    for (QList<QDateTime>::const_iterator it = days.begin(); it != days.end(); ++it)
        d->dayFilter.insert(*it, true);

    d->filterTimer->setSingleShot(true);
    d->filterTimer->start(100);
}

bool AlbumLister::tagFiltersIsActive()
{
    if (!d->tagFilter.isEmpty() || d->untaggedFilter)
        return true;

    return false;
}

void AlbumLister::setTagFilter(const QList<int>& tags, const MatchingCondition& matchingCond,
                               bool showUnTagged)
{
    d->tagFilter      = tags;
    d->matchingCond   = matchingCond;
    d->untaggedFilter = showUnTagged;
    d->filterTimer->setSingleShot(true);
    d->filterTimer->start(100);
}

void AlbumLister::setRatingFilter(int rating, const RatingCondition& ratingCond)
{
    d->ratingFilter = rating;
    d->ratingCond   = ratingCond;
    d->filterTimer->setSingleShot(true);
    d->filterTimer->start(100);
}

void AlbumLister::setMimeTypeFilter(int mimeTypeFilter)
{
    d->mimeTypeFilter = (MimeFilter::TypeMimeFilter)mimeTypeFilter;
    d->filterTimer->setSingleShot(true);
    d->filterTimer->start(100);
}

void AlbumLister::setTextFilter(const QString& text)
{
    d->textFilter = text;
    d->filterTimer->setSingleShot(true);
    d->filterTimer->start(100);
}

bool AlbumLister::matchesFilter(const ImageInfo &info, bool &foundText)
{
    if (d->dayFilter.isEmpty() && d->tagFilter.isEmpty() && d->textFilter.isEmpty() &&
        !d->untaggedFilter && d->ratingFilter==-1)
        return true;

    bool match = false;

    if (!d->tagFilter.isEmpty())
    {
        QList<int> tagIds = info.tagIds();
        QList<int>::iterator it;

        if (d->matchingCond == OrCondition)
        {
            for (it = d->tagFilter.begin(); it != d->tagFilter.end(); ++it)
            {
                if (tagIds.contains(*it))
                {
                    match = true;
                    break;
                }
            }
        }
        else
        {
            // AND matching condition...

            for (it = d->tagFilter.begin(); it != d->tagFilter.end(); ++it)
            {
                if (!tagIds.contains(*it))
                    break;
            }

            if (it == d->tagFilter.end())
                match = true;
        }

        match |= (d->untaggedFilter && tagIds.isEmpty());
    }
    else if (d->untaggedFilter)
    {
        match = info.tagIds().isEmpty();
    }
    else
    {
        match = true;
    }

    if (!d->dayFilter.isEmpty())
    {
        match &= d->dayFilter.contains(QDateTime(info.dateTime().date(), QTime()));
    }

    //-- Filter by rating ---------------------------------------------------------

    if (d->ratingFilter >= 0)
    {
        // for now we treat -1 (no rating) just like a rating of 0.
        int rating = info.rating();
        if (rating == -1)
            rating = 0;

        if (d->ratingCond == GreaterEqualCondition)
        {
            // If the rating is not >=, i.e it is <, then it does not match.
            if (rating < d->ratingFilter)
            {
                match = false;
            }
        }
        else if (d->ratingCond == EqualCondition)
        {
            // If the rating is not =, i.e it is !=, then it does not match.
            if (rating != d->ratingFilter)
            {
                match = false;
            }
        }
        else
        {
            // If the rating is not <=, i.e it is >, then it does not match.
            if (rating > d->ratingFilter)
            {
                match = false;
            }
        }
    }

    // -- Filter by mime type -----------------------------------------------------

    switch(d->mimeTypeFilter)
    {
        // info.format is a standardized string: Only one possibility per mime type
        case MimeFilter::ImageFiles:
        {
            // TODO: this filter should pull all supported mimetypes from digiKam settings
            if (info.format() != "JPG" && info.format() != "PNG" && info.format() != "TIFF" && info.format() != "RAW-DNG" && info.format() != "RAW")
                match = false;
            break;
        }
        case MimeFilter::JPGFiles:
        {
            if (info.format() != "JPG")
                match = false;
            break;
        }
        case MimeFilter::PNGFiles:
        {
            if (info.format() != "PNG")
                match = false;
            break;
        }
        case MimeFilter::TIFFiles:
        {
            if (info.format() != "TIFF")
                match = false;
            break;
        }
        case MimeFilter::DNGFiles:
        {
            if (info.format() != "RAW-DNG")
                match = false;
            break;
        }
        case MimeFilter::NoRAWFiles:
        {
            if (info.format().startsWith("RAW"))
                match = false;
            break;
        }
        case MimeFilter::RAWFiles:
        {
            if (!info.format().startsWith("RAW"))
                match = false;
            break;
        }
        case MimeFilter::MoviesFiles:
        {
            if (info.category() != DatabaseItem::Video)
                match = false;
            break;
        }
        case MimeFilter::AudioFiles:
        {
            if (info.category() != DatabaseItem::Audio)
                match = false;
            break;
        }
        default:        // All Files: do nothing...
            break;
    }

    //-- Filter by text -----------------------------------------------------------

    AlbumSettings *settings = AlbumSettings::instance();
    if ((settings->getIconShowName() || settings->getIconShowComments() || settings->getIconShowTags()) &&
        !d->textFilter.isEmpty())
    {
        foundText = false;
        if (settings->getIconShowName())
        {
            if (info.name().contains(d->textFilter))
            {
                foundText = true;
            }
        }
        if (settings->getIconShowComments())
        {
            if (info.comment().contains(d->textFilter))
                foundText = true;
        }
        if (settings->getIconShowTags())
        {
            QStringList tags = AlbumManager::instance()->tagNames(info.tagIds());
            for (QStringList::const_iterator it = tags.begin() ; it != tags.end() ; ++it)
            {
                if ((*it).contains(d->textFilter))
                    foundText = true;
            }
        }
        // check for folder names
        PAlbum* palbum = AlbumManager::instance()->findPAlbum(info.albumId());
        if ((palbum && palbum->title().contains(d->textFilter)))
        {
            foundText = true;
        }
        match &= foundText;
    }

    return match;
}

void AlbumLister::stop()
{
    d->currAlbum = 0;
    d->filterTimer->stop();
    emit signalClear();

    d->itemList.clear();
    d->itemListSet.clear();
    d->itemMap.clear();

    if (d->job)
    {
        d->job->kill();
        d->job = 0;
    }
}

void AlbumLister::invalidateItem(const ImageInfo &item)
{
    d->invalidatedItems << item.id();
}

void AlbumLister::slotFilterItems()
{
    if (d->job)
    {
        d->filterTimer->setSingleShot(true);
        d->filterTimer->start(100);
        return;
    }

    ImageInfoList newFilteredItemsList;
    ImageInfoList deleteFilteredItemsList;
    bool matchForText = false;
    bool match        = false;

    for (ImageInfoListIterator it = d->itemList.begin();
         it != d->itemList.end(); ++it)
    {
        bool foundText = false;
        if (matchesFilter(*it, foundText))
        {
            match = true;
            newFilteredItemsList.append(*it);
        }
        else
        {
            deleteFilteredItemsList.append(*it);
        }

        if (foundText)
            matchForText = true;
    }

    // This takes linear time - and deleting seems to take longer. Set wait cursor for large numbers.
    bool setCursor = (3*deleteFilteredItemsList.count() + newFilteredItemsList.count()) > 1500;
    if (setCursor)
        kapp->setOverrideCursor(Qt::WaitCursor);

    emit signalItemsTextFilterMatch(matchForText);
    emit signalItemsFilterMatch(match);

    if (!deleteFilteredItemsList.isEmpty())
    {
        foreach(const ImageInfo &info, deleteFilteredItemsList)
            emit signalDeleteFilteredItem(info);
    }
    if (!newFilteredItemsList.isEmpty())
    {
        emit signalNewFilteredItems(newFilteredItemsList);
    }

    if (setCursor)
        kapp->restoreOverrideCursor();
}

void AlbumLister::slotResult(KJob* job)
{
    d->job = 0;

    if (job->error())
    {
        kWarning(50003) << "Failed to list url: " << job->errorString() << endl;
        d->itemMap.clear();
        d->invalidatedItems.clear();
        return;
    }

    for (QMap<qlonglong, ImageInfo>::iterator it = d->itemMap.begin();
         it != d->itemMap.end(); ++it)
    {
        emit signalDeleteItem(it.value());
        emit signalDeleteFilteredItem(it.value());
        d->itemList.removeAll(it.value());
        d->itemListSet.remove(it.key());
    }

    d->itemMap.clear();
    d->invalidatedItems.clear();

    emit signalCompleted();
}

void AlbumLister::slotData(KIO::Job*, const QByteArray& data)
{
    if (data.isEmpty())
        return;

    ImageInfoList newItemsList;
    ImageInfoList newFilteredItemsList;

    QByteArray tmp(data);
    QDataStream ds(&tmp, QIODevice::ReadOnly);

    while (!ds.atEnd())
    {
        bool foundText = false;
        ImageListerRecord record;
        ds >> record;

        if (d->itemMap.contains(record.imageID))
        {
            ImageInfo info = d->itemMap[record.imageID];
            d->itemMap.remove(record.imageID);

            if (d->invalidatedItems.contains(record.imageID))
            {
                emit signalDeleteItem(info);
                emit signalDeleteFilteredItem(info);
                d->itemList.removeAll(info);
                d->itemListSet.remove(info.id());
            }
            else
            {
                if (!matchesFilter(info, foundText))
                {
                    emit signalDeleteFilteredItem(info);
                }
                continue;
            }
        }

        ImageInfo info(record);

        if (matchesFilter(info, foundText))
            newFilteredItemsList.append(info);

        newItemsList.append(info);
        d->itemList.append(info);
        d->itemListSet.insert(info.id());
    }

    if (!newFilteredItemsList.isEmpty())
        emit signalNewFilteredItems(newFilteredItemsList);

    if (!newItemsList.isEmpty())
        emit signalNewItems(newItemsList);

    slotFilterItems();
}

void AlbumLister::slotCollectionImageChange(const CollectionImageChangeset &changeset)
{
    if (!d->currAlbum)
        return;

    bool doRefresh = false;

    switch (changeset.operation())
    {
        case CollectionImageChangeset::Added:
            switch(d->currAlbum->type())
            {
                case Album::PHYSICAL:
                    // that's easy: try if our album is affected
                    doRefresh = changeset.containsAlbum(d->currAlbum->id());
                    break;
                default:
                    // we cannot easily know if we are affected
                    doRefresh = true;
                    break;
            }

        case CollectionImageChangeset::Removed:
        case CollectionImageChangeset::RemovedAll:
            // is one of our images affected?
            foreach(qlonglong id, changeset.ids())
            {
                // if one matching image id is found, trigger a refresh
                if (d->itemListSet.contains(id))
                {
                    doRefresh = true;
                    break;
                }
            }
            break;

        default:
            break;
    }

    if (doRefresh)
    {
        // use timer: there may be several signals in a row
        if (!d->refreshTimer->isActive())
            d->refreshTimer->start(100);
    }
}

void AlbumLister::slotSearchChange(const SearchChangeset &changeset)
{
    if (!d->currAlbum)
        return;

    if (changeset.operation() != SearchChangeset::Changed)
        return;

    SAlbum *album = AlbumManager::instance()->findSAlbum(changeset.searchId());

    if (album && d->currAlbum == album)
    {
        if (!d->refreshTimer->isActive())
            d->refreshTimer->start(100);
    }
}

}  // namespace Digikam
