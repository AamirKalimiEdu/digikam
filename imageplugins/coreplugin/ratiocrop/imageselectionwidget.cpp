/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-09
 * Description : image selection widget used by ratio crop tool.
 *
 * Copyright (C) 2007 by Jaromir Malenko <malenko at email.cz>
 * Copyright (C) 2008 by Roberto Castagnola <roberto dot castagnola at gmail dot com>
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#define OPACITY  0.7
#define RCOL     0xAA
#define GCOL     0xAA
#define BCOL     0xAA

#define MINRANGE 0

// Golden number (1+sqrt(5))/2
#define PHI      1.61803398874989479
// 1/PHI
#define INVPHI   0.61803398874989479

// C++ includes.

#include <iostream>
#include <cstdio>
#include <cmath>
#include <cstdlib>

// Qt includes.

#include <QRegion>
#include <QColor>
#include <QPainter>
#include <QBrush>
#include <QPixmap>
#include <QImage>
#include <QPen>
#include <QPoint>
#include <QTimer>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kglobal.h>

// Local includes.

#include "ddebug.h"
#include "imageiface.h"
#include "dimg.h"
#include "imageselectionwidget.h"
#include "imageselectionwidget.moc"

namespace DigikamImagesPluginCore
{

class ImageSelectionWidgetPriv
{
public:

    enum ResizingMode
    {
        ResizingNone = 0,
        ResizingTopLeft,
        ResizingTopRight,
        ResizingBottomLeft,
        ResizingBottomRight
    };

    ImageSelectionWidgetPriv()
    {
        currentResizing = ResizingNone;
        iface           = 0;
        pixmap          = 0;
        guideSize       = 1;
    }

    // Golden guide types.
    bool                 drawGoldenSection;
    bool                 drawGoldenSpiralSection;
    bool                 drawGoldenSpiral;
    bool                 drawGoldenTriangle;

    // Golden guide translations.
    bool                 flipHorGoldenGuide;
    bool                 flipVerGoldenGuide;

    bool                 moving;
    bool                 autoOrientation;

    int                  guideLinesType;
    int                  guideSize;

    int                  currentAspectRatioType;
    int                  currentResizing;
    int                  currentOrientation;

    float                currentWidthRatioValue;
    float                currentHeightRatioValue;

    QPoint               lastPos;

    QRect                rect;
    QRect                image;                   // Real image dimension.
    QRect                regionSelection;         // Real size image selection.
    QRect                localRegionSelection;    // Local size selection.

    // Draggable local region selection corners.
    QRect                localTopLeftCorner;
    QRect                localBottomLeftCorner;
    QRect                localTopRightCorner;
    QRect                localBottomRightCorner;

    QPixmap             *pixmap;

    QColor               guideColor;

    Digikam::DImg        preview;

    Digikam::ImageIface *iface;
};

ImageSelectionWidget::ImageSelectionWidget(int w, int h, QWidget *parent,
                                           int widthRatioValue, int heightRatioValue,
                                           int aspectRatioType, int orient, int guideLinesType)
                    : QWidget(parent)
{
    d = new ImageSelectionWidgetPriv;
    d->currentAspectRatioType  = aspectRatioType;
    d->currentWidthRatioValue  = widthRatioValue;
    d->currentHeightRatioValue = heightRatioValue;
    d->currentOrientation      = orient;
    d->guideLinesType          = guideLinesType;
    d->autoOrientation         = false;
    d->moving                  = true;
    reverseRatioValues();

    setMinimumSize(w, h);
    setMouseTracking(true);
    setAttribute(Qt::WA_DeleteOnClose);

    d->iface        = new Digikam::ImageIface(w, h);
    uchar *data     = d->iface->getPreviewImage();
    int width       = d->iface->previewWidth();
    int height      = d->iface->previewHeight();
    bool sixteenBit = d->iface->previewSixteenBit();
    bool hasAlpha   = d->iface->previewHasAlpha();
    d->preview      = Digikam::DImg(width, height, sixteenBit, hasAlpha, data);
    delete [] data;
    d->preview.convertToEightBit();
    d->pixmap  = new QPixmap(w, h);

    d->image = QRect(0, 0, d->iface->originalWidth(), d->iface->originalHeight());
    d->rect  = QRect(w/2-d->preview.width()/2, h/2-d->preview.height()/2,
                     d->preview.width(), d->preview.height());
    updatePixmap();
    setGoldenGuideTypes(true, false, false, false, false, false);
}

ImageSelectionWidget::~ImageSelectionWidget()
{
    delete d->iface;
    delete d->pixmap;
    delete d;
}

Digikam::ImageIface* ImageSelectionWidget::imageIface()
{
    return d->iface;
}

void ImageSelectionWidget::resizeEvent(QResizeEvent *e)
{
    delete d->pixmap;

    int w           = e->size().width();
    int h           = e->size().height();

    uchar *data     = d->iface->setPreviewImageSize(w, h);
    int width       = d->iface->previewWidth();
    int height      = d->iface->previewHeight();
    bool sixteenBit = d->iface->previewSixteenBit();
    bool hasAlpha   = d->iface->previewHasAlpha();
    d->preview      = Digikam::DImg(width, height, sixteenBit, hasAlpha, data);
    delete [] data;
    d->preview.convertToEightBit();

    d->pixmap = new QPixmap(w, h);

    d->rect = QRect(w/2-d->preview.width()/2, h/2-d->preview.height()/2,
                    d->preview.width(), d->preview.height());
    updatePixmap();
}

int ImageSelectionWidget::getOriginalImageWidth(void)
{
    return d->image.width();
}

int ImageSelectionWidget::getOriginalImageHeight(void)
{
    return d->image.height();
}

QRect ImageSelectionWidget::getRegionSelection(void)
{
    return d->regionSelection;
}

int ImageSelectionWidget::getMinWidthRange(void)
{
    return MINRANGE;
}

int ImageSelectionWidget::getMinHeightRange(void)
{
    return MINRANGE;
}

int ImageSelectionWidget::getMaxWidthRange(void)
{
    int maxW = d->image.width() - d->regionSelection.left();

    if (d->currentAspectRatioType != RATIONONE)
    {
        int t = d->currentWidthRatioValue > d->currentHeightRatioValue ? 1 : 0;
        int h = d->image.height() - d->regionSelection.top();
        int w = rint( ( h + t ) * d->currentWidthRatioValue /
                        d->currentHeightRatioValue ) - t;
        if ( w < maxW )
            maxW = w;
    }

    return maxW;
}

int ImageSelectionWidget::getMaxHeightRange(void)
{
    int maxH = d->image.height() - d->regionSelection.top();

    if (d->currentAspectRatioType != RATIONONE)
    {
        int t = d->currentHeightRatioValue > d->currentWidthRatioValue ? 1 : 0;
        int w = d->image.width() - d->regionSelection.left();
        int h = rint( ( w + t ) * d->currentHeightRatioValue /
                        d->currentWidthRatioValue ) - t;
        if ( h < maxH )
            maxH = h;
    }

    return maxH;
}

void ImageSelectionWidget::resetSelection(void)
{
    d->regionSelection.setWidth(d->image.width()/2);
    d->regionSelection.setHeight(d->image.height()/2);
    applyAspectRatio(false, false);

    setCenterSelection(CenterImage);
}

void ImageSelectionWidget::setCenterSelection(int centerType)
{
    if ( d->regionSelection.height() > d->image.height() )
    {
        d->regionSelection.setHeight(d->image.height());
        applyAspectRatio(true, false);
    }
    if ( d->regionSelection.width() > d->image.width() )
    {
        d->regionSelection.setWidth(d->image.width());
        applyAspectRatio(false, false);
    }

    switch (centerType)
    {
        case CenterWidth:
            d->regionSelection.moveLeft(
                   d->image.width()/2 - d->regionSelection.width()/2 );
            break;

        case CenterHeight:
            d->regionSelection.moveTop(
                   d->image.height()/2 - d->regionSelection.height()/2 );
            break;

        case CenterImage:
            d->regionSelection.moveTopLeft(QPoint(
                   d->image.width()/2 - d->regionSelection.width()/2,
                   d->image.height()/2 - d->regionSelection.height()/2 ));
            break;
    }

    updatePixmap();
    repaint();
    regionSelectionChanged();
}

void ImageSelectionWidget::maxAspectSelection(void)
{
    if ( d->currentAspectRatioType == RATIONONE )
    {
        d->regionSelection.setWidth(d->image.width());
        d->regionSelection.setHeight(d->image.height());
    }
    else if ( d->currentOrientation == Landscape )
    {
        d->regionSelection.setWidth(d->image.width());
        applyAspectRatio(false, false);
    }
    else                            // Portrait
    {
        d->regionSelection.setHeight(d->image.height());
        applyAspectRatio(true, false);
    }

    setCenterSelection(CenterImage);
}

void ImageSelectionWidget::setGoldenGuideTypes(bool drawGoldenSection,  bool drawGoldenSpiralSection,
                                               bool drawGoldenSpiral,   bool drawGoldenTriangle,
                                               bool flipHorGoldenGuide, bool flipVerGoldenGuide)
{
    d->drawGoldenSection       = drawGoldenSection;
    d->drawGoldenSpiralSection = drawGoldenSpiralSection;
    d->drawGoldenSpiral        = drawGoldenSpiral;
    d->drawGoldenTriangle      = drawGoldenTriangle;
    d->flipHorGoldenGuide      = flipHorGoldenGuide;
    d->flipVerGoldenGuide      = flipVerGoldenGuide;
}

void ImageSelectionWidget::slotGuideLines(int guideLinesType)
{
    d->guideLinesType = guideLinesType;
    updatePixmap();
    repaint();
}

void ImageSelectionWidget::slotChangeGuideColor(const QColor &color)
{
    d->guideColor = color;
    updatePixmap();
    repaint();
}

void ImageSelectionWidget::slotChangeGuideSize(int size)
{
    d->guideSize = size;
    updatePixmap();
    repaint();
}

void ImageSelectionWidget::setSelectionOrientation(int orient)
{
    d->currentOrientation = orient;
    reverseRatioValues();
    applyAspectRatio(true);
    emit signalSelectionOrientationChanged( d->currentOrientation );
}

void ImageSelectionWidget::setSelectionAspectRatioType(int aspectRatioType)
{
    d->currentAspectRatioType = aspectRatioType;

    switch(aspectRatioType)
    {
       case RATIO01X01:
          d->currentWidthRatioValue = 1.0;
          d->currentHeightRatioValue = 1.0;
          break;

       case RATIO03X04:
          d->currentWidthRatioValue = 4.0;
          d->currentHeightRatioValue = 3.0;
          break;

       case RATIO02x03:
          d->currentWidthRatioValue = 3.0;
          d->currentHeightRatioValue = 2.0;
          break;

       case RATIO05x07:
          d->currentWidthRatioValue = 7.0;
          d->currentHeightRatioValue = 5.0;
          break;

       case RATIO07x10:
          d->currentWidthRatioValue = 10.0;
          d->currentHeightRatioValue = 7.0;
          break;

       case RATIO04X05:
          d->currentWidthRatioValue = 5.0;
          d->currentHeightRatioValue = 4.0;
          break;

       case RATIOGOLDEN:
          d->currentWidthRatioValue = PHI;
          d->currentHeightRatioValue = 1.0;
          break;
    }

    reverseRatioValues();
    applyAspectRatio(false);
}

void ImageSelectionWidget::setSelectionAspectRatioValue(int widthRatioValue,
                                                        int heightRatioValue)
{
    int gdc = widthRatioValue;

    // Compute greatest common divisor using Euclidean algorithm

    for (int tmp, mod = heightRatioValue; mod != 0; mod = tmp % mod)
    {
        tmp = gdc;
        gdc = mod;
    }

    d->currentWidthRatioValue  = widthRatioValue / gdc;
    d->currentHeightRatioValue = heightRatioValue / gdc;
    d->currentAspectRatioType  = RATIOCUSTOM;

    // Fix orientation

    if ( d->autoOrientation )
    {
        if ( heightRatioValue > widthRatioValue &&
             d->currentOrientation == Landscape )
        {
            d->currentOrientation = Portrait;
            emit signalSelectionOrientationChanged( d->currentOrientation );
        }
        else if ( widthRatioValue > heightRatioValue &&
                  d->currentOrientation == Portrait )
        {
            d->currentOrientation = Landscape;
            emit signalSelectionOrientationChanged( d->currentOrientation );
        }
    }
    else
        reverseRatioValues();

    applyAspectRatio(false);
}

void ImageSelectionWidget::reverseRatioValues(void)
{
    if ( ( d->currentWidthRatioValue > d->currentHeightRatioValue &&
           d->currentOrientation == Portrait ) ||
         ( d->currentHeightRatioValue > d->currentWidthRatioValue &&
           d->currentOrientation == Landscape ) )
    {
        float tmp = d->currentWidthRatioValue;
        d->currentWidthRatioValue = d->currentHeightRatioValue;
        d->currentHeightRatioValue = tmp;
    }
}

void ImageSelectionWidget::setAutoOrientation(bool orientation)
{
    d->autoOrientation = orientation;
}

void ImageSelectionWidget::setSelectionX(int x)
{
    d->regionSelection.moveLeft(x);
    regionSelectionMoved();
}

void ImageSelectionWidget::setSelectionY(int y)
{
    d->regionSelection.moveTop(y);
    regionSelectionMoved();
}

void ImageSelectionWidget::setSelectionWidth(int w)
{
    d->regionSelection.setWidth(w);
    applyAspectRatio(false, true);

    regionSelectionChanged();
}

void ImageSelectionWidget::setSelectionHeight(int h)
{
    d->regionSelection.setHeight(h);
    applyAspectRatio(true, true);

    regionSelectionChanged();
}

QPoint ImageSelectionWidget::convertPoint(const QPoint pm, bool localToReal)
{
    return convertPoint(pm.x(), pm.y(), localToReal);
}

QPoint ImageSelectionWidget::convertPoint(int x, int y, bool localToReal)
{
    int pmX, pmY;

    if (localToReal)
    {
        pmX = ( x - d->rect.left() ) * (float)d->image.width() /
                    (float)d->preview.width();

        pmY = ( y - d->rect.top() ) * (float)d->image.height() /
                    (float)d->preview.height();
    }
    else
    {
        pmX = d->rect.left() + ( x * (float)d->preview.width() /
                                    (float)d->image.width() );

        pmY = d->rect.top() + ( y * (float)d->preview.height() /
                                    (float)d->image.height() );
    }

    return QPoint(pmX, pmY);
}

void ImageSelectionWidget::applyAspectRatio(bool WOrH, bool repaintWidget)
{
    // Save selection area for re-adjustment after changing width and height.
    QRect oldRegionSelection = d->regionSelection;

    if ( !WOrH )  // Width changed.
    {
        int w = d->regionSelection.width();

        switch(d->currentAspectRatioType)
        {
            case RATIONONE:
                break;

            default:
                d->regionSelection.setHeight(
                                rint( w * d->currentHeightRatioValue /
                                    d->currentWidthRatioValue ) );
                break;
        }
    }
    else      // Height changed.
    {
        int h = d->regionSelection.height();

        switch(d->currentAspectRatioType)
        {
            case RATIONONE:
                break;

            default:
                d->regionSelection.setWidth(
                                rint( h * d->currentWidthRatioValue /
                                    d->currentHeightRatioValue ) );
                break;
        }
    }

    // If we change selection size by a corner, re-adjust the oposite corner position.

    switch(d->currentResizing)
    {
        case ImageSelectionWidgetPriv::ResizingTopLeft:
            d->regionSelection.moveBottomRight( oldRegionSelection.bottomRight() );
            break;

        case ImageSelectionWidgetPriv::ResizingTopRight:
            d->regionSelection.moveBottomLeft( oldRegionSelection.bottomLeft() );
            break;

        case ImageSelectionWidgetPriv::ResizingBottomLeft:
            d->regionSelection.moveTopRight( oldRegionSelection.topRight() );
            break;

        case ImageSelectionWidgetPriv::ResizingBottomRight:
             d->regionSelection.moveTopLeft( oldRegionSelection.topLeft() );
             break;
    }

    if (repaintWidget)
    {
        updatePixmap();
        repaint();
    }
}

QPoint ImageSelectionWidget::computeAspectRatio( QPoint pm , int coef )
{
    QPoint point = pm;

    switch(d->currentAspectRatioType)
        {
        case RATIONONE:
          break;

        default:
            QPoint delta = pm - d->regionSelection.center();
            if ( d->currentOrientation == Landscape )
                point.setY( d->regionSelection.center().y() + coef *
                        rint( delta.x() * d->currentHeightRatioValue /
                                   d->currentWidthRatioValue ) );
            else                       // Portrait
                point.setX( d->regionSelection.center().x() + coef *
                        rint( delta.y() * d->currentWidthRatioValue /
                                   d->currentHeightRatioValue ) );
            break;
        }

    return point;
}

void ImageSelectionWidget::normalizeRegion(void)
{
    // Perform normalization of selection area.

    if (d->regionSelection.left() < d->image.left())
        d->regionSelection.moveLeft(d->image.left());

    if (d->regionSelection.top() < d->image.top())
        d->regionSelection.moveTop(d->image.top());

    if (d->regionSelection.right() > d->image.right())
        d->regionSelection.moveRight(d->image.right());

    if (d->regionSelection.bottom() > d->image.bottom())
        d->regionSelection.moveBottom(d->image.bottom());
}

void ImageSelectionWidget::regionSelectionMoved(void)
{
    normalizeRegion();

    updatePixmap();
    repaint();

    emit signalSelectionMoved( d->regionSelection );
}

void ImageSelectionWidget::regionSelectionChanged(void)
{
    if ( d->regionSelection.top() > d->image.bottom()   ||
         d->regionSelection.left() > d->image.right()   ||
         d->regionSelection.bottom() < d->image.top()-1 ||
         d->regionSelection.right() < d->image.left()-1 )
    {
        // Selection is completely out of image border
        d->regionSelection.setRect(d->image.left(), d->image.top(), 0, 0);
        updatePixmap();
        repaint();
    }
    else
    {
        if (d->regionSelection.left() < d->image.left())
        {
            d->regionSelection.setLeft(d->image.left());
            applyAspectRatio(false);
        }
        if (d->regionSelection.top() < d->image.top())
        {
            d->regionSelection.setTop(d->image.top());
            applyAspectRatio(true);
        }
        if (d->regionSelection.right() > d->image.right())
        {
            d->regionSelection.setRight(d->image.right());
            applyAspectRatio(false);
        }
        if (d->regionSelection.bottom() > d->image.bottom())
        {
            d->regionSelection.setBottom(d->image.bottom());
            applyAspectRatio(true);
        }
    }

    emit signalSelectionChanged( d->regionSelection );
}

void ImageSelectionWidget::updatePixmap(void)
{
    // Updated local selection region.

    d->localRegionSelection.setTopLeft(
            convertPoint(d->regionSelection.topLeft(), false));
    d->localRegionSelection.setBottomRight(
            convertPoint(d->regionSelection.bottomRight(), false));

    // Updated dragging corners region.

    d->localTopLeftCorner.setRect(d->localRegionSelection.left(),
                                  d->localRegionSelection.top(), 8, 8);
    d->localBottomLeftCorner.setRect(d->localRegionSelection.left(),
                                     d->localRegionSelection.bottom() - 7, 8, 8);
    d->localTopRightCorner.setRect(d->localRegionSelection.right() - 7,
                                   d->localRegionSelection.top(), 8, 8);
    d->localBottomRightCorner.setRect(d->localRegionSelection.right() - 7,
                                      d->localRegionSelection.bottom() - 7, 8, 8);

    // Drawing background and image.

    d->pixmap->fill(palette().color(QPalette::Background));

    if (d->preview.isNull())
        return;

    // Drawing region outside selection grayed.

    Digikam::DImg image = d->preview.copy();

    uchar* ptr = image.bits();
    uchar  r, g, b;

    for (int y=d->rect.top() ; y <= d->rect.bottom() ; y++)
    {
        for (int x=d->rect.left() ; x <= d->rect.right() ; x++)
        {
            if (! d->localRegionSelection.contains(x, y, true) )
            {
                b = ptr[0];
                g = ptr[1];
                r = ptr[2];

                r += (uchar)((RCOL - r) * OPACITY);
                g += (uchar)((GCOL - g) * OPACITY);
                b += (uchar)((BCOL - b) * OPACITY);

                ptr[0] = b;
                ptr[1] = g;
                ptr[2] = r;
            }

            ptr+=4;
        }
    }

    QPixmap pix = d->iface->convertToPixmap(image);
    QPainter p(d->pixmap);
    p.drawPixmap(d->rect.x(), d->rect.y(), pix);

    // Stop here if no selection to draw
    if ( d->regionSelection.isEmpty() )
        return;

    // Drawing selection borders.

    p.setPen(QPen(QColor(250, 250, 255), 1, Qt::SolidLine));
    p.drawRect(d->localRegionSelection);

    // Drawing selection corners.

    p.drawRect(d->localTopLeftCorner);
    p.drawRect(d->localBottomLeftCorner);
    p.drawRect(d->localTopRightCorner);
    p.drawRect(d->localBottomRightCorner);

    // Drawing guide lines.

    // Constraint drawing only on local selection region.
    // This is needed because arcs and incurved lines can draw
    // outside a little of local selection region.
    p.setClipping(true);
    p.setClipRect(d->localRegionSelection);

    switch (d->guideLinesType)
    {
       case RulesOfThirds:
       {
            int xThird = d->localRegionSelection.width()  / 3;
            int yThird = d->localRegionSelection.height() / 3;

            p.setPen(QPen(Qt::white, d->guideSize, Qt::SolidLine));
            p.drawLine( d->localRegionSelection.left() + xThird,   d->localRegionSelection.top(),
                        d->localRegionSelection.left() + xThird,   d->localRegionSelection.bottom() );
            p.drawLine( d->localRegionSelection.left() + 2*xThird, d->localRegionSelection.top(),
                        d->localRegionSelection.left() + 2*xThird, d->localRegionSelection.bottom() );

            p.drawLine( d->localRegionSelection.left(),  d->localRegionSelection.top() + yThird,
                        d->localRegionSelection.right(), d->localRegionSelection.top() + yThird );
            p.drawLine( d->localRegionSelection.left(),  d->localRegionSelection.top() + 2*yThird,
                        d->localRegionSelection.right(), d->localRegionSelection.top() + 2*yThird );

            p.setPen(QPen(d->guideColor, d->guideSize, Qt::DotLine));
            p.drawLine( d->localRegionSelection.left() + xThird,   d->localRegionSelection.top(),
                        d->localRegionSelection.left() + xThird,   d->localRegionSelection.bottom() );
            p.drawLine( d->localRegionSelection.left() + 2*xThird, d->localRegionSelection.top(),
                        d->localRegionSelection.left() + 2*xThird, d->localRegionSelection.bottom() );

            p.drawLine( d->localRegionSelection.left(),  d->localRegionSelection.top() + yThird,
                        d->localRegionSelection.right(), d->localRegionSelection.top() + yThird );
            p.drawLine( d->localRegionSelection.left(),  d->localRegionSelection.top() + 2*yThird,
                        d->localRegionSelection.right(), d->localRegionSelection.top() + 2*yThird );
            break;
       }

       case HarmoniousTriangles:
       {
            // Move coordinates to local center selection.
            p.translate(d->localRegionSelection.center().x(), d->localRegionSelection.center().y());

            // Flip horizontal.
            if (d->flipHorGoldenGuide)
                p.scale(-1, 1);

            // Flip verical.
            if (d->flipVerGoldenGuide)
                p.scale(1, -1);

            float w = (float)d->localRegionSelection.width();
            float h = (float)d->localRegionSelection.height();
            int dst = (int)((h*cos(atan(w/h)) / (cos(atan(h/w)))));

            p.setPen(QPen(Qt::white, d->guideSize, Qt::SolidLine));
            p.drawLine( -d->localRegionSelection.width()/2, -d->localRegionSelection.height()/2,
                         d->localRegionSelection.width()/2,  d->localRegionSelection.height()/2);

            p.drawLine( -d->localRegionSelection.width()/2 + dst, -d->localRegionSelection.height()/2,
                        -d->localRegionSelection.width()/2,        d->localRegionSelection.height()/2);

            p.drawLine( d->localRegionSelection.width()/2,       -d->localRegionSelection.height()/2,
                        d->localRegionSelection.width()/2 - dst,  d->localRegionSelection.height()/2);

            p.setPen(QPen(d->guideColor, d->guideSize, Qt::DotLine));
            p.drawLine( -d->localRegionSelection.width()/2, -d->localRegionSelection.height()/2,
                         d->localRegionSelection.width()/2,  d->localRegionSelection.height()/2);

            p.drawLine( -d->localRegionSelection.width()/2 + dst, -d->localRegionSelection.height()/2,
                        -d->localRegionSelection.width()/2,        d->localRegionSelection.height()/2);

            p.drawLine( d->localRegionSelection.width()/2,       -d->localRegionSelection.height()/2,
                        d->localRegionSelection.width()/2 - dst,  d->localRegionSelection.height()/2);
            break;
       }

       case GoldenMean:
       {
            // Move coordinates to local center selection.
            p.translate(d->localRegionSelection.center().x(), d->localRegionSelection.center().y());

            // Flip horizontal.
            if (d->flipHorGoldenGuide)
                p.scale(-1, 1);

            // Flip vertical.
            if (d->flipVerGoldenGuide)
                p.scale(1, -1);

            int w = d->localRegionSelection.width();
            int h = d->localRegionSelection.height();

            // lengths for the golden mean and half the sizes of the region:
            int w_g = (int)(w*INVPHI);
            int h_g = (int)(h*INVPHI);
            int w_2 = w/2;
            int h_2 = h/2;

            QRect R1(-w_2, -h_2, w_g, h);
            // w - 2*w_2 corrects for one-pixel difference
            // so that R2.right() is really at the right end of the region
            QRect R2(w_g-w_2, h_2-h_g, w-w_g+1-(w - 2*w_2), h_g);

            QRect R3((int)(w_2 - R2.width()*INVPHI), -h_2,
                     (int)(R2.width()*INVPHI), h - R2.height());
            QRect R4(R2.x(), R1.y(), R3.x() - R2.x(),
                     (int)(R3.height()*INVPHI));
            QRect R5(R4.x(), R4.bottom(), (int)(R4.width()*INVPHI),
                     R3.height() - R4.height());
            QRect R6(R5.x() + R5.width(), R5.bottom() - (int)(R5.height()*INVPHI),
                     R3.x() - R5.right(), (int)(R5.height()*INVPHI));
            QRect R7(R6.right() - (int)(R6.width()*INVPHI), R4.bottom(),
                     (int)(R6.width()*INVPHI), R5.height() - R6.height());

            p.setPen(QPen(Qt::white, d->guideSize, Qt::SolidLine));

            // Drawing Golden sections.
            if (d->drawGoldenSection)
            {
               // horizontal lines:
               p.drawLine( R1.left(), R2.top(),
                           R2.right(), R2.top());

               p.drawLine( R1.left(), R1.top() + R2.height(),
                           R2.right(), R1.top() + R2.height());

               // vertical lines:
               p.drawLine( R1.right(), R1.top(),
                           R1.right(), R1.bottom() );

               p.drawLine( R1.left()+R2.width(), R1.top(),
                           R1.left()+R2.width(), R1.bottom() );
            }

            // Drawing Golden triangle guides.
            if (d->drawGoldenTriangle)
            {
               p.drawLine( R1.left(),  R1.bottom(),
                           R2.right(), R1.top() );

               p.drawLine( R1.left(), R1.top(),
                           R2.right() - R1.width(), R1.bottom());

               p.drawLine( R1.left() + R1.width(), R1.top(),
                           R2.right(), R1.bottom() );
            }

            // Drawing Golden spiral sections.
            if (d->drawGoldenSpiralSection)
            {
               p.drawLine( R1.topRight(),   R1.bottomRight() );
               p.drawLine( R2.topLeft(),    R2.topRight() );
               p.drawLine( R3.topLeft(),    R3.bottomLeft() );
               p.drawLine( R4.bottomLeft(), R4.bottomRight() );
               p.drawLine( R5.topRight(),   R5.bottomRight() );
               p.drawLine( R6.topLeft(),    R6.topRight() );
               p.drawLine( R7.topLeft(),    R7.bottomLeft() );
            }

            // Drawing Golden Spiral.
            if (d->drawGoldenSpiral)
            {
               p.drawArc ( R1.left(),
                           R1.top() - R1.height(),
                           2*R1.width(), 2*R1.height(),
                           180*16, 90*16);

               p.drawArc ( R2.right() - 2*R2.width(),
                           R1.bottom() - 2*R2.height(),
                           2*R2.width(), 2*R2.height(),
                           270*16, 90*16);

               p.drawArc ( R2.right() - 2*R3.width(),
                           R3.top(),
                           2*R3.width(), 2*R3.height(),
                           0, 90*16);

               p.drawArc ( R4.left(),
                           R4.top(),
                           2*R4.width(), 2*R4.height(),
                           90*16, 90*16);

               p.drawArc ( R5.left(),
                           R5.top()-R5.height(),
                           2*R5.width(), 2*R5.height(),
                           180*16, 90*16);

               p.drawArc ( R6.left()-R6.width(),
                           R6.top()-R6.height(),
                           2*R6.width(), 2*R6.height(),
                           270*16, 90*16);

               p.drawArc ( R7.left()-R7.width(),
                           R7.top(),
                           2*R7.width(), 2*R7.height(),
                           0, 90*16);
            }

            p.setPen(QPen(d->guideColor, d->guideSize, Qt::DotLine));

            // Drawing Golden sections.
            if (d->drawGoldenSection)
            {
               // horizontal lines:
               p.drawLine( R1.left(), R2.top(),
                           R2.right(), R2.top());

               p.drawLine( R1.left(), R1.top() + R2.height(),
                           R2.right(), R1.top() + R2.height());

               // vertical lines:
               p.drawLine( R1.right(), R1.top(),
                           R1.right(), R1.bottom() );

               p.drawLine( R1.left()+R2.width(), R1.top(),
                           R1.left()+R2.width(), R1.bottom() );
            }

            // Drawing Golden triangle guides.
            if (d->drawGoldenTriangle)
            {
               p.drawLine( R1.left(),  R1.bottom(),
                           R2.right(), R1.top() );

               p.drawLine( R1.left(), R1.top(),
                           R2.right() - R1.width(), R1.bottom());

               p.drawLine( R1.left() + R1.width(), R1.top(),
                           R2.right(), R1.bottom() );
            }

            // Drawing Golden spiral sections.
            if (d->drawGoldenSpiralSection)
            {
               p.drawLine( R1.topRight(),   R1.bottomRight() );
               p.drawLine( R2.topLeft(),    R2.topRight() );
               p.drawLine( R3.topLeft(),    R3.bottomLeft() );
               p.drawLine( R4.bottomLeft(), R4.bottomRight() );
               p.drawLine( R5.topRight(),   R5.bottomRight() );
               p.drawLine( R6.topLeft(),    R6.topRight() );
               p.drawLine( R7.topLeft(),    R7.bottomLeft() );
            }

            // Drawing Golden Spiral.
            if (d->drawGoldenSpiral)
            {
               p.drawArc ( R1.left(),
                           R1.top() - R1.height(),
                           2*R1.width(), 2*R1.height(),
                           180*16, 90*16);

               p.drawArc ( R2.right() - 2*R2.width(),
                           R1.bottom() - 2*R2.height(),
                           2*R2.width(), 2*R2.height(),
                           270*16, 90*16);

               p.drawArc ( R2.right() - 2*R3.width(),
                           R3.top(),
                           2*R3.width(), 2*R3.height(),
                           0, 90*16);

               p.drawArc ( R4.left(),
                           R4.top(),
                           2*R4.width(), 2*R4.height(),
                           90*16, 90*16);

               p.drawArc ( R5.left(),
                           R5.top()-R5.height(),
                           2*R5.width(), 2*R5.height(),
                           180*16, 90*16);

               p.drawArc ( R6.left()-R6.width(),
                           R6.top()-R6.height(),
                           2*R6.width(), 2*R6.height(),
                           270*16, 90*16);

               p.drawArc ( R7.left()-R7.width(),
                           R7.top(),
                           2*R7.width(), 2*R7.height(),
                           0, 90*16);
            }

            break;
       }
    }

    p.setClipping(false);

    p.end();
}

void ImageSelectionWidget::paintEvent( QPaintEvent * )
{
    QPainter p(this);
    p.drawPixmap(0, 0, *d->pixmap);
    p.end();
}

QPoint ImageSelectionWidget::opposite(void)
{
    QPoint opp;

    switch(d->currentResizing)
    {
        case ImageSelectionWidgetPriv::ResizingTopLeft:
        default:
            opp = d->regionSelection.bottomRight();
            break;

        case ImageSelectionWidgetPriv::ResizingTopRight:
            opp = d->regionSelection.bottomLeft();
            break;

        case ImageSelectionWidgetPriv::ResizingBottomLeft:
            opp = d->regionSelection.topRight();
            break;

        case ImageSelectionWidgetPriv::ResizingBottomRight:
            opp = d->regionSelection.topLeft();
            break;
    }

    return opp;
}

float ImageSelectionWidget::distance(QPoint a, QPoint b)
{
    return sqrt(pow(a.x() - b.x(), 2) + pow(a.y() - b.y(), 2));
}

void ImageSelectionWidget::setCursorResizing()
{
    switch(d->currentResizing)
    {
        case ImageSelectionWidgetPriv::ResizingTopLeft:
            setCursor( Qt::SizeFDiagCursor );
            break;

        case ImageSelectionWidgetPriv::ResizingTopRight:
            setCursor( Qt::SizeBDiagCursor );
            break;

        case ImageSelectionWidgetPriv::ResizingBottomLeft:
            setCursor( Qt::SizeBDiagCursor );
            break;

        case ImageSelectionWidgetPriv::ResizingBottomRight:
            setCursor( Qt::SizeFDiagCursor );
            break;
    }
}

void ImageSelectionWidget::placeSelection(QPoint pm, bool symetric, QPoint center)
{
    // Place the corner at the mouse

    switch(d->currentResizing)
    {
        case ImageSelectionWidgetPriv::ResizingTopLeft:
            if ( ! symetric )
            {
                d->regionSelection.setTopLeft(pm);
            }
            else
            {
                // Place corner to the proper position
                d->regionSelection.setTopLeft(computeAspectRatio(pm));
                // Update oposite corner
                QPoint delta = d->regionSelection.topLeft() - center;
                d->regionSelection.setBottomRight(center - delta);
            }
            break;

        case ImageSelectionWidgetPriv::ResizingTopRight:
            if ( ! symetric )
            {
                d->regionSelection.setTopRight(pm);
            }
            else
            {
                d->regionSelection.setTopRight(computeAspectRatio(pm, -1));
                QPoint delta = d->regionSelection.topRight() - center;
                d->regionSelection.setBottomLeft(center - delta);
            }
            break;

        case ImageSelectionWidgetPriv::ResizingBottomLeft:
            if ( ! symetric )
            {
                d->regionSelection.setBottomLeft(pm);
            }
            else
            {
                d->regionSelection.setBottomLeft(computeAspectRatio(pm, -1));
                QPoint delta = d->regionSelection.bottomLeft() - center;
                d->regionSelection.setTopRight(center - delta);
            }
            break;

        case ImageSelectionWidgetPriv::ResizingBottomRight:
            if ( ! symetric )
            {
                d->regionSelection.setBottomRight(pm);
            }
            else
            {
                d->regionSelection.setBottomRight(computeAspectRatio(pm));
                QPoint delta = d->regionSelection.bottomRight() - center;
                d->regionSelection.setTopLeft(center - delta);
            }
            break;
    }

    // Set orientation

    if ( d->autoOrientation )
    {
        QPoint rel = pm - opposite();

        if ( abs(rel.x()) > abs(rel.y()) )
        {
            if ( d->currentOrientation == Portrait )
            {
                d->currentOrientation = Landscape;
                reverseRatioValues();
                emit signalSelectionOrientationChanged( d->currentOrientation );
            }
        }
        else
        {
            if ( d->currentOrientation == Landscape )
            {
                d->currentOrientation = Portrait;
                reverseRatioValues();
                emit signalSelectionOrientationChanged( d->currentOrientation );
            }
        }
    }

    // Repaint

    if ( ! symetric )
    {
        bool aspectFirst = d->currentOrientation == Portrait;
        applyAspectRatio(aspectFirst, false);
        applyAspectRatio(! aspectFirst);
    }
    else
    {
        updatePixmap();
        repaint();
    }

}

void ImageSelectionWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton )
    {
        QPoint pm = QPoint(e->x(), e->y());
        QPoint pmVirtual = convertPoint(pm);
        d->moving = false;

        if ( (e->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier )
        {
            bool symetric = (e->modifiers() & Qt::ControlModifier ) == Qt::ControlModifier;
            QPoint center = d->regionSelection.center();

            // Find the closest corner

            QPoint points[] = { d->regionSelection.topLeft(),
                                d->regionSelection.topRight(),
                                d->regionSelection.bottomLeft(),
                                d->regionSelection.bottomRight() };
            int resizings[] = { ImageSelectionWidgetPriv::ResizingTopLeft,
                                ImageSelectionWidgetPriv::ResizingTopRight,
                                ImageSelectionWidgetPriv::ResizingBottomLeft,
                                ImageSelectionWidgetPriv::ResizingBottomRight };
            float dist = -1;
            for (int i = 0 ; i < 4 ; i++)
            {
                QPoint point = points[i];
                float dist2 = distance(pmVirtual, point);
                if (dist2 < dist || d->currentResizing == ImageSelectionWidgetPriv::ResizingNone)
                {
                    dist = dist2;
                    d->currentResizing = resizings[i];
                }
            }

            setCursorResizing();

            placeSelection(pmVirtual, symetric, center);

        }
        else
        {
            if ( d->localTopLeftCorner.contains( pm ) )
                d->currentResizing = ImageSelectionWidgetPriv::ResizingTopLeft;
            else if ( d->localTopRightCorner.contains( pm ) )
                d->currentResizing = ImageSelectionWidgetPriv::ResizingTopRight;
            else if ( d->localBottomLeftCorner.contains( pm ) )
                d->currentResizing = ImageSelectionWidgetPriv::ResizingBottomLeft;
            else if ( d->localBottomRightCorner.contains( pm ) )
                d->currentResizing = ImageSelectionWidgetPriv::ResizingBottomRight;
            else
            {
                d->lastPos = pmVirtual;
                setCursor( Qt::SizeAllCursor );

                if (d->regionSelection.contains( pmVirtual ) )
                {
                    d->moving = true;
                }
                else
                {
                    d->regionSelection.moveCenter( pmVirtual );
                    normalizeRegion();
                    updatePixmap();
                    repaint();
                }
            }
        }
    }
}

void ImageSelectionWidget::mouseReleaseEvent ( QMouseEvent * )
{
    if ( d->currentResizing != ImageSelectionWidgetPriv::ResizingNone )
    {
        setCursor( Qt::ArrowCursor );
        regionSelectionChanged();
        d->currentResizing = ImageSelectionWidgetPriv::ResizingNone;
    }
    else if ( d->regionSelection.contains( d->lastPos ) )
    {
        setCursor( Qt::PointingHandCursor );
        regionSelectionMoved();
    }
    else
    {
        setCursor( Qt::ArrowCursor );
        regionSelectionMoved();
    }
}

void ImageSelectionWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( ( e->buttons() & Qt::LeftButton ) == Qt::LeftButton )
    {
        if ( d->moving )
        {
            setCursor( Qt::SizeAllCursor );
            QPoint newPos = convertPoint(e->x(), e->y());

            d->regionSelection.translate( newPos.x() - d->lastPos.x(),
                                          newPos.y() - d->lastPos.y() );

            d->lastPos = newPos;

            normalizeRegion();

            updatePixmap();
            repaint();
        }
        else
        {
            QPoint pmVirtual = convertPoint(e->x(), e->y());

            if ( d->currentResizing == ImageSelectionWidgetPriv::ResizingNone )
            {
                d->regionSelection.setTopLeft( pmVirtual );
                d->regionSelection.setBottomRight( pmVirtual );
                d->currentResizing = ImageSelectionWidgetPriv::ResizingTopLeft; // set to anything
            }

            QPoint center = d->regionSelection.center();
            bool symetric = (e->modifiers() & Qt::ControlModifier ) == Qt::ControlModifier;

            // Change resizing mode

            QPoint opp = symetric ? center : opposite();
            QPoint dir = pmVirtual - opp;

            if ( dir.x() > 0 && dir.y() > 0 && d->currentResizing != ImageSelectionWidgetPriv::ResizingBottomRight)
            {
                d->currentResizing = ImageSelectionWidgetPriv::ResizingBottomRight;
                d->regionSelection.setTopLeft( opp );
                setCursor( Qt::SizeFDiagCursor );
            }
            else if ( dir.x() > 0 && dir.y() < 0 && d->currentResizing != ImageSelectionWidgetPriv::ResizingTopRight)
            {
                d->currentResizing = ImageSelectionWidgetPriv::ResizingTopRight;
                d->regionSelection.setBottomLeft( opp );
                setCursor( Qt::SizeBDiagCursor );
            }
            else if ( dir.x() < 0 && dir.y() > 0 && d->currentResizing != ImageSelectionWidgetPriv::ResizingBottomLeft)
            {
                d->currentResizing = ImageSelectionWidgetPriv::ResizingBottomLeft;
                d->regionSelection.setTopRight( opp );
                setCursor( Qt::SizeBDiagCursor );
            }
            else if ( dir.x() < 0 && dir.y() < 0 && d->currentResizing != ImageSelectionWidgetPriv::ResizingTopLeft)
            {
                d->currentResizing = ImageSelectionWidgetPriv::ResizingTopLeft;
                d->regionSelection.setBottomRight( opp );
                setCursor( Qt::SizeFDiagCursor );
            }
            else
            {
                if ( dir.x() == 0 && dir.y() == 0 )
                    setCursor( Qt::SizeAllCursor );
                else if ( dir.x() == 0 )
                    setCursor( Qt::SizeHorCursor );
                else if ( dir.y() == 0 )
                    setCursor( Qt::SizeVerCursor );
            }

            placeSelection(pmVirtual, symetric, center);
        }
    }
    else
    {
        if ( d->localTopLeftCorner.contains( e->x(), e->y() ) ||
             d->localBottomRightCorner.contains( e->x(), e->y() ) )
            setCursor( Qt::SizeFDiagCursor );
        else if ( d->localTopRightCorner.contains( e->x(), e->y() ) ||
                  d->localBottomLeftCorner.contains( e->x(), e->y() ) )
            setCursor( Qt::SizeBDiagCursor );
        else if ( d->localRegionSelection.contains( e->x(), e->y() ) )
            setCursor( Qt::PointingHandCursor );
        else
            setCursor( Qt::ArrowCursor );
    }
}

}  // NameSpace DigikamImagesPluginCore
