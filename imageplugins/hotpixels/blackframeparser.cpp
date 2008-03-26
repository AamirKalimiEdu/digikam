/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : black frames parser
 * 
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
 * 
 * Part of the algorithm for finding the hot pixels was based on
 * the code of jpegpixi, which was released under the GPL license,
 * and is Copyright (C) 2003, 2004 Martin Dickopp
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

// Denominator for relative quantities. 
#define DENOM (DENOM_SQRT * DENOM_SQRT)

// Square root of denominator for relative quantities. 
#define DENOM_SQRT 10000

// Convert relative to absolute numbers. Care must be taken not to overflow integers. 
#define REL_TO_ABS(n,m) \
    ((((n) / DENOM_SQRT) * (m) + ((n) % DENOM_SQRT) * (m) / DENOM_SQRT) / DENOM_SQRT)

// QT includes.

#include <QImage>
#include <QStringList>

// KDE includes.

#include <kapplication.h>
#include <kdeversion.h>
#include <kio/netaccess.h>
#include <kio/job.h>

// Local includes.

#include "blackframeparser.h"
#include "blackframeparser.moc"

namespace DigikamHotPixelsImagesPlugin
{

BlackFrameParser::BlackFrameParser(QObject *parent)
                : QObject(parent)
{
    m_imageLoaderThread = 0;
}

BlackFrameParser::~BlackFrameParser()
{
    delete m_imageLoaderThread;
}

void BlackFrameParser::parseHotPixels(const QString& file)
{
    parseBlackFrame(KUrl(file));
}

void BlackFrameParser::parseBlackFrame(const KUrl& url)
{
    KIO::NetAccess::download(url, m_localFile, kapp->activeWindow());

    if (!m_imageLoaderThread)
    {
        m_imageLoaderThread = new LoadSaveThread();

        connect(m_imageLoaderThread, SIGNAL(signalLoadingProgress(const LoadingDescription&, float)),
                this, SLOT(slotLoadingProgress(const LoadingDescription&, float)));

        connect(m_imageLoaderThread, SIGNAL(signalImageLoaded(const LoadingDescription&, const DImg&)),
                this, SLOT(slotLoadImageFromUrlComplete(const LoadingDescription&, const DImg&)));
    }

    LoadingDescription desc = LoadingDescription(m_localFile, KDcrawIface::RawDecodingSettings());
    m_imageLoaderThread->load(desc);
}

void BlackFrameParser::slotLoadingProgress(const LoadingDescription&, float v)
{
    emit signalLoadingProgress(v);
}

void BlackFrameParser::slotLoadImageFromUrlComplete(const LoadingDescription&, const DImg& img)
{
    DImg image(img);
    m_Image = image.copyQImage();
    blackFrameParsing();
    emit signalLoadingComplete();
}

void BlackFrameParser::parseBlackFrame(QImage& img)
{
    m_Image = img;
    blackFrameParsing();
}

// Parses black frames

void BlackFrameParser::blackFrameParsing()
{
    // Now find the hot pixels and store them in a list
    QList<HotPixel> hpList;
    
    for (int y=0 ; y < m_Image.height() ; ++y)
    {
        for (int x=0 ; x < m_Image.width() ; ++x)
        {
            //Get each point in the image
            QRgb pixrgb = m_Image.pixel(x,y);
            QColor color; 
            color.setRgb(pixrgb);
            
            // Find maximum component value.
            int       maxValue;
            int       threshold = DENOM/10;
            const int threshold_value = REL_TO_ABS(threshold,255);
            maxValue = (color.red()>color.blue()) ? color.red() : color.blue();
            if (color.green()>maxValue) maxValue = color.green();

            // If the component is bigger than the threshold, add the point
            if (maxValue > threshold_value)
            {
                HotPixel point;
                point.rect = QRect (x, y, 1, 1);
                //TODO:check this
                point.luminosity = ((2 * DENOM) / 255 ) * maxValue / 2;
    
                hpList.append(point);
            }
        }
    }
    
    //Now join points together into groups
    consolidatePixels (hpList);
    
    //And notify
    emit parsed(hpList);
}

// Consolidate adjacent points into larger points.

void BlackFrameParser::consolidatePixels (QList<HotPixel>& list)
{
    if (list.isEmpty()) 
        return;

    /* Consolidate horizontally.  */
    
    QList<HotPixel>::iterator it, prevPointIt;

    prevPointIt = list.begin();
    it          = list.begin();
    ++it;
    
    HotPixel tmp;
    HotPixel point;
    HotPixel point_below;
    QList<HotPixel>::iterator end(list.end()); 
    for (; it != end; ++it )
    {
        while (1)
        {
            point = (*it);
            tmp   = point;
    
            QList<HotPixel>::Iterator point_below_it;
            
            //find any intersecting hotpixels below tmp
            point_below_it = list.find (tmp); 

            if (point_below_it != list.end())
            {
                point_below =* point_below_it;
                validateAndConsolidate(&point, &point_below);
                
                point.rect.setX(qMin(point.x(), point_below.x()));
                point.rect.setWidth(qMax(point.x() + point.width(),
                                    point_below.x() + point_below.width()) - point.x());
                point.rect.setHeight(qMax(point.y() + point.height(),
                                     point_below.y() + point_below.height()) - point.y());
                *it = point;
                list.remove (point_below_it); //TODO: Check! this could remove it++?
            }
            else    
                break;
        }
    }
}

void BlackFrameParser::validateAndConsolidate (HotPixel *a, HotPixel *b)
{
    a->luminosity = qMax(a->luminosity, b->luminosity);
}

}  // NameSpace DigikamHotPixelsImagesPlugin
