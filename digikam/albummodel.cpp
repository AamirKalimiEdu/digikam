/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-22
 * Description : Qt Model for Albums
 *
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes.

#include <QStringList>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albummanager.h"
#include "albumthumbnailloader.h"
#include "albummodel.h"
#include "albummodel.moc"

namespace Digikam
{

class AlbumModelPriv
{
public:

    AlbumModelPriv()
    {
        rootAlbum = 0;
        type = Album::PHYSICAL;
        rootBehavior = AbstractAlbumModel::IncludeRootAlbum;
        addingAlbum = 0;
        removingAlbum = 0;
    }

    Album            *rootAlbum;
    Album::Type       type;
    AbstractAlbumModel::RootAlbumBehavior
                      rootBehavior;

    Album            *addingAlbum;
    Album            *removingAlbum;

    Album *findNthChild(Album *parent, int n)
    {
        // return the n-th of theh children of parent, or 0
        Album *a = parent->firstChild();
        for (int i=0; i<n; i++)
        {
            a = a->next();
            if (!a)
                return 0;
        }
        return a;
    }

    int findIndexAsChild(Album *child)
    {
        // return index of child in the list of children of its parent
        Album *parent = child->parent();
        if (!parent)
            return 0;
        Album *a = parent->firstChild();
        int i = 0;
        while (a != child)
        {
            a = a->next();
            if (!a)
                return -1;
            i++;
        }
        return i;
    }

    int numberOfChildren(Album *parent)
    {
        Album *a = parent->firstChild();
        int count = 0;
        while (a)
        {
            count++;
            a = a->next();
        }
        return count;
    }
};

AbstractAlbumModel::AbstractAlbumModel(Album::Type albumType, Album *rootAlbum, RootAlbumBehavior rootBehavior,
                                       QObject *parent)
    : QAbstractItemModel(parent)
{
    d = new AlbumModelPriv;

    d->type = albumType;
    d->rootAlbum = rootAlbum;
    d->rootBehavior = rootBehavior;

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAboutToBeAdded(Album*, Album*, Album*)),
            this, SLOT(slotAlbumAboutToBeAdded(Album*, Album*, Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAboutToBeDeleted(Album*)),
            this, SLOT(slotAlbumAboutToBeDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumHasBeenDeleted(void *)),
            this, SLOT(slotAlbumHasBeenDeleted(void *)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumIconChanged(Album*)),
            this, SLOT(slotAlbumIconChanged(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    //connect(AlbumManager::instance(), SIGNAL(signalPAlbumsDirty(const QMap<int, int>&)),
      //      this, SLOT(slotRefresh(const QMap<int, int>&)));
}

AbstractAlbumModel::~AbstractAlbumModel()
{
    delete d;
}

QVariant AbstractAlbumModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Album *a = static_cast<Album*>(index.internalPointer());

    switch (role)
    {
        case Qt::DisplayRole:
            return a->title();
        case Qt::ToolTipRole:
            return a->title();
        case Qt::DecorationRole:
            // reimplement in subclasses
            return decorationRole(a);
        case AlbumTypeRole:
            return a->type();
        case AlbumPointerRole:
            return a;
    }
    return QVariant();
}

QVariant AbstractAlbumModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)
    if (section == 0 && role == Qt::DisplayRole)
        return columnHeader();
    return QVariant();
}

int AbstractAlbumModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        Album *a = static_cast<Album*>(parent.internalPointer());
        return d->numberOfChildren(a);
    }
    else
    {
        if (!d->rootAlbum)
            return 0;

        if (d->rootBehavior == IncludeRootAlbum)
            return 1;
        else
            return d->numberOfChildren(d->rootAlbum);
    }
}

int AbstractAlbumModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

Qt::ItemFlags AbstractAlbumModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Album *a = static_cast<Album*>(index.internalPointer());
    return itemFlags(a);
}

bool AbstractAlbumModel::hasChildren(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        Album *a = static_cast<Album*>(parent.internalPointer());
        return a->firstChild();
    }
    else
    {
        if (!d->rootAlbum)
            return false;

        if (d->rootBehavior == IncludeRootAlbum)
            return 1;
        else
            return d->rootAlbum->firstChild();
    }
}

QModelIndex AbstractAlbumModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column != 0)
        return QModelIndex();

    if (parent.isValid())
    {
        Album *parentAlbum = static_cast<Album*>(parent.internalPointer());
        Album *a = d->findNthChild(parentAlbum, row);
        if (a)
            return createIndex(row, column, a);
    }
    else
    {
        if (!d->rootAlbum)
            return QModelIndex();

        if (d->rootBehavior == IncludeRootAlbum)
        {
            if (row == 0)
                return createIndex(0, 0, d->rootAlbum);
        }
        else
        {
            Album *a = d->findNthChild(d->rootAlbum, row);
            if (a)
                return createIndex(row, column, a);
        }
    }
    return QModelIndex();
}

QModelIndex AbstractAlbumModel::parent(const QModelIndex &index) const
{
    if (index.isValid())
    {
        Album *a = static_cast<Album*>(index.internalPointer());
        return indexForAlbum(a->parent());
    }
    return QModelIndex();
}

bool AbstractAlbumModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    //TODO
    return QAbstractItemModel::setData(index, value, role);
}

Qt::DropActions AbstractAlbumModel::supportedDropActions() const
{
    //TODO
    return QAbstractItemModel::supportedDropActions();
}

QStringList AbstractAlbumModel::mimeTypes() const
{
    //TODO
    return QAbstractItemModel::mimeTypes();
}

bool AbstractAlbumModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    //TODO
    return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
}

QMimeData *AbstractAlbumModel::mimeData(const QModelIndexList &indexes) const
{
    //TODO
    return QAbstractItemModel::mimeData(indexes);
}

QModelIndex AbstractAlbumModel::indexForAlbum(Album *a) const
{
    if (!a)
        return QModelIndex();

    // a is root album? Decide on root behavior
    if (a == d->rootAlbum)
    {
        if (d->rootBehavior == IncludeRootAlbum)
            // create top-level index
            return createIndex(0, 0, a);
        else
            // with this behavior, root album has no valid index
            return QModelIndex();
    }

    // Normal album. Get its row.
    int row = d->findIndexAsChild(a);
    return createIndex(row, 0, a);
}

Album *AbstractAlbumModel::albumForIndex(const QModelIndex &index) const
{
    return static_cast<Album*>(index.internalPointer());
}

Album *AbstractAlbumModel::rootAlbum() const
{
    return d->rootAlbum;
}

QVariant AbstractAlbumModel::decorationRole(Album *) const
{
    return QVariant();
}

QString AbstractAlbumModel::columnHeader() const
{
    return i18n("Album");
}

Qt::ItemFlags AbstractAlbumModel::itemFlags(Album *) const
{
    return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

bool AbstractAlbumModel::filterAlbum(Album *album) const
{
    return album && album->type() == d->type;
}

void AbstractAlbumModel::slotAlbumAboutToBeAdded(Album *album, Album *parent, Album *prev)
{
    if (!filterAlbum(album))
        return;

    // when the model is instantiated before albums are initialized
    if (album->isRoot() && !d->rootAlbum)
        d->rootAlbum = album;

    // start inserting operation
    int row = prev ? d->findIndexAsChild(prev) : 0;
    QModelIndex parentIndex = indexForAlbum(parent);
    beginInsertRows(parentIndex, row, row);

    // store album for slotAlbumAdded
    d->addingAlbum = album;
}

void AbstractAlbumModel::slotAlbumAdded(Album *album)
{
    if (d->addingAlbum == album)
    {
        d->addingAlbum = 0;
        endInsertRows();
    }
}

void AbstractAlbumModel::slotAlbumAboutToBeDeleted(Album *album)
{
    if (!filterAlbum(album))
        return;

    // begin removing operation
    int row = d->findIndexAsChild(album);
    QModelIndex parent = indexForAlbum(album->parent());
    beginRemoveRows(parent, row, row);

    // store album for slotAlbumHasBeenDeleted
    d->removingAlbum = album;
}

void AbstractAlbumModel::slotAlbumHasBeenDeleted(void *p)
{
    if (d->removingAlbum == p)
    {
        d->removingAlbum = 0;
        endRemoveRows();
    }
}

void AbstractAlbumModel::slotAlbumsCleared()
{
    reset();
}

void AbstractAlbumModel::slotAlbumIconChanged(Album* album)
{
    QModelIndex index = indexForAlbum(album);
    emit dataChanged(index, index);
}

void AbstractAlbumModel::slotAlbumRenamed(Album *album)
{
    QModelIndex index = indexForAlbum(album);
    emit dataChanged(index, index);
}

// ------------------------------------------------------------------

AbstractSpecificAlbumModel::AbstractSpecificAlbumModel(Album::Type albumType,
                                                       Album *rootAlbum,
                                                       RootAlbumBehavior rootBehavior,
                                                       QObject *parent)
    : AbstractAlbumModel(albumType, rootAlbum, rootBehavior, parent)
{
    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();

    connect(loader, SIGNAL(signalThumbnail(Album *, const QPixmap&)),
            this, SLOT(slotGotThumbnailFromIcon(Album *, const QPixmap&)));

    connect(loader, SIGNAL(signalFailed(Album *)),
            this, SLOT(slotThumbnailLost(Album *)));

    connect(loader, SIGNAL(signalReloadThumbnails()),
            this, SLOT(slotReloadThumbnails()));
}

QString AbstractSpecificAlbumModel::columnHeader() const
{
    return m_columnHeader;
}

void AbstractSpecificAlbumModel::slotGotThumbnailFromIcon(Album *album, const QPixmap&)
{
    // see decorationRole() method of subclasses

    if (!filterAlbum(album))
        return;

    QModelIndex index = indexForAlbum(album);
    emit dataChanged(index, index);
}

void AbstractSpecificAlbumModel::slotThumbnailLost(Album *)
{
    // ignore, use default thumbnail
}

void AbstractSpecificAlbumModel::slotReloadThumbnails()
{
    // emit dataChanged() for all albums
    emitDataChangedForChildren(rootAlbum());
}

void AbstractSpecificAlbumModel::emitDataChangedForChildren(Album *album)
{
    for (Album *child = album->firstChild(); child; child = child->next())
    {
        if (filterAlbum(child))
        {
            // recurse to children of children
            emitDataChangedForChildren(child);

            // emit signal for child
            QModelIndex index = indexForAlbum(child);
            emit dataChanged(index, index);
        }
    }
}

// ------------------------------------------------------------------

AbstractCheckableAlbumModel::AbstractCheckableAlbumModel(Album::Type albumType, Album *rootAlbum,
                                                         RootAlbumBehavior rootBehavior,
                                                         QObject *parent)
    : AbstractSpecificAlbumModel(albumType, rootAlbum, rootBehavior, parent)
{
    m_extraFlags = 0;
}

void AbstractCheckableAlbumModel::setCheckable(bool isCheckable)
{
    if (isCheckable)
        m_extraFlags |= Qt::ItemIsUserCheckable;
    else
    {
        m_extraFlags &= ~Qt::ItemIsUserCheckable;
        resetCheckedAlbums();
    }
}

bool AbstractCheckableAlbumModel::isCheckable() const
{
    return m_extraFlags & Qt::ItemIsUserCheckable;
}

void AbstractCheckableAlbumModel::setTristate(bool isTristate)
{
    if (isTristate)
        m_extraFlags |= Qt::ItemIsTristate;
    else
        m_extraFlags &= ~Qt::ItemIsTristate;
}

bool AbstractCheckableAlbumModel::isTristate() const
{
    return m_extraFlags & Qt::ItemIsTristate;
}

bool AbstractCheckableAlbumModel::isChecked(Album *album) const
{
    return m_checkedAlbums.value(album, Qt::Unchecked) == Qt::Checked;
}

Qt::CheckState AbstractCheckableAlbumModel::checkState(Album *album) const
{
    return m_checkedAlbums.value(album, Qt::Unchecked);
}

void AbstractCheckableAlbumModel::setChecked(Album *album, bool isChecked)
{
    setData(indexForAlbum(album), isChecked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
}

void AbstractCheckableAlbumModel::setCheckState(Album *album, Qt::CheckState state)
{
    setData(indexForAlbum(album), state, Qt::CheckStateRole);
}

QList<Album *> AbstractCheckableAlbumModel::checkedAlbums() const
{
    // return a list with all keys with value Qt::Checked
    return m_checkedAlbums.keys(Qt::Checked);
}

void AbstractCheckableAlbumModel::resetCheckedAlbums()
{
    QList<Album *> oldChecked = m_checkedAlbums.keys();
    m_checkedAlbums.clear();
    foreach(Album *album, oldChecked)
    {
        QModelIndex index = indexForAlbum(album);
        emit dataChanged(index, index);
        emit checkStateChanged(album, Qt::Unchecked);
    }
}

QVariant AbstractCheckableAlbumModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::CheckStateRole)
    {
        if (m_extraFlags & Qt::ItemIsUserCheckable)
        {
            // with Qt::Unchecked as default, albums not in the hash (initially all)
            // are simply regarded as unchecked
            return m_checkedAlbums.value(albumForIndex(index), Qt::Unchecked);
        }
    }

    return AbstractSpecificAlbumModel::data(index, role);
}

Qt::ItemFlags AbstractCheckableAlbumModel::flags(const QModelIndex &index) const
{
    return AbstractSpecificAlbumModel::flags(index) | m_extraFlags;
}

bool AbstractCheckableAlbumModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole)
    {
        Qt::CheckState state = (Qt::CheckState)value.toInt();
        Album *album = albumForIndex(index);
        if (!album)
            return false;
        m_checkedAlbums.insert(album, state);
        emit dataChanged(index, index);
        emit checkStateChanged(album, state);
        return true;
    }
    else
        return AbstractSpecificAlbumModel::setData(index, value, role);
}

// ------------------------------------------------------------------

AlbumModel::AlbumModel(RootAlbumBehavior rootBehavior, QObject *parent)
    : AbstractCheckableAlbumModel(Album::PHYSICAL,
                                 AlbumManager::instance()->findPAlbum(0),
                                 rootBehavior, parent)
{
    m_columnHeader = i18n("My Albums");
}

QVariant AlbumModel::decorationRole(Album *album) const
{
    // asynchronous signals are handled by parent class
    return AlbumThumbnailLoader::instance()->getAlbumThumbnailDirectly(static_cast<PAlbum *>(album));
}

// ------------------------------------------------------------------

TagModel::TagModel(RootAlbumBehavior rootBehavior, QObject *parent)
    : AbstractCheckableAlbumModel(Album::TAG,
                                 AlbumManager::instance()->findTAlbum(0),
                                 rootBehavior, parent)
{
    m_columnHeader = i18n("My Tags");
}

QVariant TagModel::decorationRole(Album *album) const
{
    return AlbumThumbnailLoader::instance()->getTagThumbnailDirectly(static_cast<TAlbum *>(album), true);
}

} // namespace Digikam
