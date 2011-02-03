/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-18-03
 * Description : image preview thumbs bar
 *
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPREVIEWBAR_H
#define IMAGEPREVIEWBAR_H

// Local includes

#include "thumbbar.h"
#include "imageinfo.h"
#include "digikam_export.h"

class QDragMoveEvent;
class QDropEvent;
class QMouseEvent;
class QPaintEvent;

namespace Digikam
{

class ImagePreviewBarItem;

class ImagePreviewBar : public ThumbBarView
{
    Q_OBJECT

public:

    explicit ImagePreviewBar(QWidget* parent, int orientation=Qt::Vertical,
                             bool exifRotate=true);
    virtual ~ImagePreviewBar();

    ThumbBarItem* ratingItem() const;

    void setSelectedItem(ImagePreviewBarItem* ltItem);

    ImageInfo     currentItemImageInfo() const;
    ImageInfoList itemsImageInfoList();

    ImagePreviewBarItem* findItemByInfo(const ImageInfo& info) const;
    ImagePreviewBarItem* findItemByPos(const QPoint& pos) const;

    /** Read tool tip settings from Album Settings instance */
    void readToolTipSettings();

    void applySettings();

    void ensureItemVisible(ThumbBarItem* item);
    void clear(bool updateView=true);
    void takeItem(ThumbBarItem* item);
    void removeItem(ThumbBarItem* item);

protected:

    QRect clickToRateRect(ImagePreviewBarItem* item);
    QPixmap ratingPixmap() const;
    void startDrag();

    /**
     * Hook method that can be implemented to provide custom item drawing.
     *
     * @param item the item to draw
     * @param p painter to use that is already prepared according to the item's
     *          role like being selected etc.
     * @param tile the pixmap to draw on
     */
    virtual void drawItem(ThumbBarItem* item, QPainter& p, QPixmap& tile);

    /**
     * Hook method that can be implemented to draw a custom message if there are
     * no images to display.
     *
     * @param pixmap pixmap to paint on
     */
    virtual void drawEmptyMessage(QPixmap& pixmap);

    virtual void rearrangeItems();
    virtual void contentsMouseMoveEvent(QMouseEvent*);
    virtual void viewportPaintEvent(QPaintEvent*);
    virtual void leaveEvent(QEvent*);
    virtual void focusOutEvent(QFocusEvent*);
    virtual void contentsWheelEvent(QWheelEvent*);
    virtual bool eventFilter(QObject* obj, QEvent* ev);

private Q_SLOTS:

    void slotImageRatingChanged(qlonglong);
    void slotEditRatingFromItem(int);

    void slotThemeChanged();

private:

    class ImagePreviewBarPriv;
    ImagePreviewBarPriv* const d;

    friend class ImagePreviewBarItem;
};

// -------------------------------------------------------------------------

class ImagePreviewBarItem : public ThumbBarItem
{
public:

    ImagePreviewBarItem(ImagePreviewBar* view, const ImageInfo& info);
    ~ImagePreviewBarItem();

    ImageInfo info();

    QRect clickToRateRect();

protected:

    ImageInfo m_info;

private:

    friend class ImagePreviewBar;
};

// -------------------------------------------------------------------------

class ImagePreviewBarToolTip : public ThumbBarToolTip
{

public:

    ImagePreviewBarToolTip(ThumbBarView* parent);
    ~ImagePreviewBarToolTip();

protected:

    virtual QString tipContents();
};

}  // namespace Digikam

#endif /* IMAGEPREVIEWBAR_H */
