/* ============================================================
 * File  : thumbbar.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-11-22
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qpixmap.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qdict.h>
#include <qpoint.h>

#include <kfileitem.h>
#include <kio/previewjob.h>

#include <cmath>

#include "thumbbar.h"

class ThumbBarViewPriv
{
public:

    ThumbBarViewPriv() {
        firstItem = 0;
        lastItem  = 0;
        currItem  = 0;
        count     = 0;
        itemDict.setAutoDelete(false);
    }
    
    ThumbBarItem *firstItem;
    ThumbBarItem *lastItem;
    ThumbBarItem *currItem;
    int           count;

    QDict<ThumbBarItem> itemDict;
    
    bool          clearing;
    int           margin;
    int           tileSize;
    
    QTimer*       timer;
};

ThumbBarView::ThumbBarView(QWidget* parent)
    : QScrollView(parent)
{
    d = new ThumbBarViewPriv;
    d->margin   = 5;
    d->tileSize = 64;

    d->timer = new QTimer(this);
    connect(d->timer, SIGNAL(timeout()),
            SLOT(slotUpdate()));

    viewport()->setBackgroundMode(Qt::NoBackground);
    setHScrollBarMode(QScrollView::AlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setFixedWidth(d->tileSize + 2*d->margin
                  + verticalScrollBar()->sizeHint().width());
}

ThumbBarView::~ThumbBarView()
{
    clear(false);

    delete d->timer;
    d->timer = 0;
    
    delete d;
}

void ThumbBarView::clear(bool updateView)
{
    d->clearing = true;

    ThumbBarItem *item = d->firstItem;
    while (item)
    {
        ThumbBarItem *tmp = item->m_next;
        delete item;
        item = tmp;
    }

    d->firstItem = 0;
    d->lastItem  = 0;
    d->count     = 0;
    d->currItem  = 0;
    
    if (updateView)
        slotUpdate();

    d->clearing = false;
}

void ThumbBarView::triggerUpdate()
{
    d->timer->start(0, true);    
}

ThumbBarItem* ThumbBarView::currentItem() const
{
    return d->currItem;    
}

void ThumbBarView::setSelected(ThumbBarItem* item)
{
    if (d->currItem == item)
        return;
    
    d->currItem = item;
    if (d->currItem)
    {
        ensureVisible(0,item->m_pos);
        updateContents();
        emit signalURLSelected(item->url());
    }
}

void ThumbBarView::viewportPaintEvent(QPaintEvent* e)
{
    QRect er(e->rect());
    int cy = viewportToContents(er.topLeft()).y();
    
    QPixmap bgPix(contentsRect().width(), er.height());
    bgPix.fill(colorGroup().background());

    int ts = d->tileSize+2*d->margin;
    QPixmap tile(visibleWidth(), ts);

    int y1 = (cy/ts)*ts;
    int y2 = ((y1 + er.height())/ts +1)*ts;

    for (ThumbBarItem *item = d->firstItem; item; item = item->m_next)
    {
        if (y1 <= item->m_pos && item->m_pos <= y2)
        {
            if (item == d->currItem)
                tile.fill(colorGroup().highlight());
            else
                tile.fill(colorGroup().background());

            QPainter p(&tile);
            p.setPen(Qt::white);
            p.drawRect(0, 0, tile.width(), tile.height());
            p.end();
            
            if (item->m_pixmap)
            {
                int x = (tile.width() -item->m_pixmap->width())/2;
                int y = (tile.height()-item->m_pixmap->height())/2;
                bitBlt(&tile, x, y, item->m_pixmap);
            }
            
            bitBlt(&bgPix, 0, item->m_pos - cy, &tile);
        }
    }

    bitBlt(viewport(), 0, er.y(), &bgPix);
}

void ThumbBarView::contentsMousePressEvent(QMouseEvent* e)
{
    ThumbBarItem* barItem = 0;
    
    int y = e->pos().y();
    for (ThumbBarItem *item = d->firstItem; item; item = item->m_next)
    {
        if (y >= item->m_pos &&
            y <= (item->m_pos+d->tileSize+2*d->margin))
        {
            barItem = item;
            break;
        }
    }

    if (!barItem || barItem == d->currItem)
        return;

    d->currItem = barItem;
    viewport()->update();
    
    emit signalURLSelected(barItem->url());
}

void ThumbBarView::insertItem(ThumbBarItem* item)
{
    if (!item) return;

    if (!d->firstItem)
    {
        d->firstItem = item;
        d->lastItem  = item;
        item->m_prev = 0;
        item->m_next = 0;
    }
    else
    {
        d->lastItem->m_next = item;
        item->m_prev = d->lastItem;
        item->m_next = 0;
        d->lastItem = item;

    }

    if (!d->currItem)
    {
        d->currItem = item;
        emit signalURLSelected(item->url());
    }
    
    d->itemDict.insert(item->url().url(), item);
    
    d->count++;
    triggerUpdate();
}

void ThumbBarView::removeItem(ThumbBarItem* item)
{
    if (!item) return;

    d->count--;

    if (d->currItem == item)
    {
        d->currItem = 0;
        // emit signal
    }

    if (item == d->firstItem)
    {
	d->firstItem = d->firstItem->m_next;
	if (d->firstItem)
	    d->firstItem->m_prev = 0;
        else
            d->firstItem = d->lastItem = 0;
    }
    else if (item == d->lastItem)
    {
	d->lastItem = d->lastItem->m_prev;
	if ( d->lastItem )
	    d->lastItem->m_next = 0;
        else
            d->firstItem = d->lastItem = 0;
    }
    else
    {
	ThumbBarItem *i = item;
	if (i) {
	    if (i->m_prev )
		i->m_prev->m_next = i->m_next;
	    if ( i->m_next )
		i->m_next->m_prev = i->m_prev;
	}
    }

    d->itemDict.remove(item->url().url());
    
    if (!d->clearing)
    {
        triggerUpdate();
    }
}

void ThumbBarView::rearrangeItems()
{
    KURL::List urlList;

    int pos = 0;
    ThumbBarItem *item = d->firstItem;
    while (item)
    {
        item->m_pos = pos;
        pos += d->tileSize + 2*d->margin;
        if (!(item->m_pixmap))
            urlList.append(item->m_url);
        item = item->m_next;
    }

    resizeContents(width(), d->count*(d->tileSize+2*d->margin));
    
    if (!urlList.isEmpty())
    {
        KIO::PreviewJob* job = KIO::filePreview(urlList,
                                                d->tileSize);
        connect(job, SIGNAL(gotPreview(const KFileItem *, const QPixmap &)),
                SLOT(slotGotPreview(const KFileItem *, const QPixmap &)));
        connect(job, SIGNAL(failed(const KFileItem *)),
                SLOT(slotFailedPreview(const KFileItem *)));
    }
}

void ThumbBarView::slotUpdate()
{
    rearrangeItems();
    viewport()->update();
}

void ThumbBarView::slotGotPreview(const KFileItem *fileItem,
                                  const QPixmap& pix)
{
    ThumbBarItem* item = d->itemDict.find(fileItem->url().url());
    if (!item)
        return;

    item->m_pixmap = new QPixmap(pix);
    if (item->m_pos < height())
        viewport()->update();
}

void ThumbBarView::slotFailedPreview(const KFileItem* )
{
    
}

ThumbBarItem::ThumbBarItem(ThumbBarView* view,
                           const KURL& url)
    : m_view(view), m_url(url), m_next(0), m_prev(0),
      m_pixmap(0)
{
    m_view->insertItem(this);
}

ThumbBarItem::~ThumbBarItem()
{
    m_view->removeItem(this);
    if (m_pixmap)
        delete m_pixmap;
}

KURL ThumbBarItem::url() const
{
    return m_url;
}

ThumbBarItem* ThumbBarItem::next() const
{
    return m_next;
}

ThumbBarItem* ThumbBarItem::prev() const
{
    return m_prev;
}

#include "thumbbar.moc"
