/* ============================================================
* File  : imageeffect_hotpixels.cpp
* Author: Unai Garro <ugarro at users dot sourceforge dot net>
*         Gilles Caulier <caulier dot gilles at free dot fr>
* Date  : 2005-07-05
* Description : a ListView to display black frames
*
* Copyright 2005 by Unai Garro and Gilles Caulier
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

#ifndef BLACKFRAMELISTVIEW_H
#define BLACKFRAMELISTVIEW_H

// Qt includes.

#include <qimage.h>
#include <qsize.h>
#include <qpoint.h>
#include <qvaluelist.h>

// KDE includes.

#include <klistview.h>
#include <kurl.h>

// Local includes.

#include "blackframeparser.h"
#include "hotpixel.h"

class BlackFrameListView;

class BlackFrameListViewItem : public QObject, KListViewItem
{
Q_OBJECT

public:

    BlackFrameListViewItem(BlackFrameListView* parent, KURL url);
    ~BlackFrameListViewItem(){};
    
    virtual QString text(int column)const;
    virtual void paintCell(QPainter* p, const QColorGroup& cg, int column, int width, int align);
    virtual int width (const QFontMetrics& fm, const QListView* lv, int c)const;

signals:

    void parsed(QValueList<HotPixel>);
            
private:

    // Data contained within each listview item
    QImage                mThumb;
    QImage                mImage;
    QSize                 mImageSize;
    QValueList <HotPixel> mHotPixels;
    BlackFrameParser      mParser;

private:    
        
    // Private methods
    QPixmap thumb(QSize size);
    
private slots:

    void slotParsed(QValueList<HotPixel>);        
};

/////////////////////////////////////////////////////////////////

class BlackFrameListView:public KListView
{
    Q_OBJECT

public:
    
    BlackFrameListView(QWidget* parent=0);
    ~BlackFrameListView(){};

signals:

    void blackFrameSelected(QValueList<HotPixel>);

private slots:

    void slotParsed(QValueList<HotPixel> hotPixels)
       {
       emit blackFrameSelected(hotPixels);
       };           
};

#endif  // BLACKFRAMELISTVIEW_H
