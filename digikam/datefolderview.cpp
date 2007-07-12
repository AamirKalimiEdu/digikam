/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-27
 * Description : a view for date albums.
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

// Qt includes.

#include <Q3ListView>
#include <QDateTime>
#include <QFont>
#include <QPainter>
#include <QStyle>

// KDE includes.

#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kcalendarsystem.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "monthwidget.h"
#include "folderitem.h"
#include "folderview.h"
#include "datefolderview.h"
#include "datefolderview.moc"

namespace Digikam
{

class DateFolderViewPriv
{
public:

    FolderView*             listview;
    MonthWidget*            monthview;
    bool                    active;
    
    QString                 selected;
};

class DateFolderItem : public FolderItem
{
public:

    DateFolderItem(Q3ListView* parent, const QString& name)
        : FolderItem(parent, name, true), m_album(0)
    {
    }

    DateFolderItem(Q3ListViewItem* parent, const QString& name, DAlbum* album)
        : FolderItem(parent, name), m_album(album)
    {
    }

    int compare(Q3ListViewItem* i, int , bool ) const
    {
        if (!i)
            return 0;
        
        DateFolderItem* dItem = dynamic_cast<DateFolderItem*>(i);
        if (!dItem || !dItem->m_album)
        {
            return text(0).localeAwareCompare(i->text(0));
        } 

        if (m_album->date() == dItem->m_album->date())
            return 0;
        else if (m_album->date() > dItem->m_album->date())
            return 1;
        else
            return -1;
    }
    
    QString date() const
    {
        // If an album is set, return it's date, otherwise just the year
        return m_album ? m_album->date().toString() : text(0);
    }
    
    DAlbum* m_album;
};
    


DateFolderView::DateFolderView(QWidget* parent)
              : KVBox(parent)
{
    d = new DateFolderViewPriv;
    d->active    = false;
    d->listview  = new FolderView(this);
    d->monthview = new MonthWidget(this);

    d->listview->addColumn(i18n("My Dates"));
    d->listview->setResizeMode(Q3ListView::LastColumn);
    d->listview->setRootIsDecorated(true);

    connect(AlbumManager::componentData(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));
    connect(AlbumManager::componentData(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));
    connect(AlbumManager::componentData(), SIGNAL(signalAllDAlbumsLoaded()),
            this, SLOT(slotAllDAlbumsLoaded()));    
    connect(AlbumManager::componentData(), SIGNAL(signalAlbumsCleared()),
            d->listview, SLOT(clear()));

    connect(d->listview, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));
}

DateFolderView::~DateFolderView()
{
    saveViewState();
    delete d;
}

void DateFolderView::setActive(bool val)
{
    if (d->active == val)
        return;
    
    d->active = val;
    if (d->active)
    {
        slotSelectionChanged();
    }
    else
    {
        d->monthview->setActive(false);
    }
}

void DateFolderView::slotAllDAlbumsLoaded()
{
    disconnect(AlbumManager::componentData(), SIGNAL(signalAllDAlbumsLoaded()),
               this, SLOT(slotAllDAlbumsLoaded()));
    loadViewState();
}

void DateFolderView::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::DATE)
        return;

    DAlbum* album = (DAlbum*)a;
    
    QDate date = album->date();

    QString yr = QString::number(date.year());
    
    QString mo = KGlobal::locale()->calendar()->monthName(date, false);
    Q3ListViewItem* parent = d->listview->findItem(yr, 0);
    if (!parent)
    {
        parent = new DateFolderItem(d->listview, yr);
        parent->setPixmap(0, SmallIcon("date", AlbumSettings::componentData()->getDefaultTreeIconSize()));
    }

    DateFolderItem* item = new DateFolderItem(parent, mo, album);
    item->setPixmap(0, SmallIcon("date", AlbumSettings::componentData()->getDefaultTreeIconSize()));

    album->setExtraData(this, item);
}

void DateFolderView::slotAlbumDeleted(Album* a)
{
    if (!a || a->type() != Album::DATE)
        return;

    DAlbum* album = (DAlbum*)a;

    DateFolderItem* item = (DateFolderItem*) album->extraData(this);
    if (item)
    {
        delete item;
        album->removeExtraData(this);
    }
}

void DateFolderView::slotSelectionChanged()
{
    if (!d->active)
        return;
    
    Q3ListViewItem* selItem = 0;
    
    Q3ListViewItemIterator it( d->listview );
    while (it.current())
    {
        if (it.current()->isSelected())
        {
            selItem = it.current();
            break;
        }
        ++it;
    }

    d->monthview->setActive(false);
    
    if (!selItem)
    {
        AlbumManager::componentData()->setCurrentAlbum(0);
        return;
    }

    DateFolderItem* dateItem = dynamic_cast<DateFolderItem*>(selItem);
    
    if (!dateItem || !dateItem->m_album)
    {
        AlbumManager::componentData()->setCurrentAlbum(0);
        d->monthview->setActive(false);
    }
    else
    {
        AlbumManager::componentData()->setCurrentAlbum(dateItem->m_album);

        QDate date = dateItem->m_album->date();        
        d->monthview->setActive(true);
        d->monthview->setYearMonth(date.year(), date.month());
    }
}

void DateFolderView::loadViewState()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(objectName());
    
    QString selected;
    if(config->hasKey("LastSelectedItem"))
    {
        selected = group.readEntry("LastSelectedItem");
    }

    QStringList openFolders;
    if(config->hasKey("OpenFolders"))
    {
        openFolders = group.readEntry("OpenFolders",QStringList());
    }
    
    DateFolderItem *item;
    QString id;
    Q3ListViewItemIterator it(d->listview);
    for( ; it.current(); ++it)
    {        
        item = dynamic_cast<DateFolderItem*>(it.current());
        id = item->date();
        if(openFolders.contains(id))
            d->listview->setOpen(item, true);
        else
            d->listview->setOpen(item, false);
        
        if(id == selected)
            d->listview->setSelected(item, true);
    }    
}

void DateFolderView::saveViewState()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(objectName());
   
    DateFolderItem *item = dynamic_cast<DateFolderItem*>(d->listview->selectedItem());
    if(item)
        group.writeEntry("LastSelectedItem", item->date());
    
    QStringList openFolders;
    Q3ListViewItemIterator it(d->listview);
    item = dynamic_cast<DateFolderItem*>(d->listview->firstChild());
    while(item)
    {
        // Storing the years only, a month cannot be open
        if(item && d->listview->isOpen(item))
            openFolders.push_back(item->date());
        item = dynamic_cast<DateFolderItem*>(item->nextSibling());
    }
    group.writeEntry("OpenFolders", openFolders);
}

void DateFolderView::setSelected(Q3ListViewItem *item)
{
    if(!item)
        return;
    
    d->listview->setSelected(item, true);
    d->listview->ensureItemVisible(item);
}

}  // namespace Digikam
