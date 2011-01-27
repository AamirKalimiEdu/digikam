/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-08-15
 * Description : a widget to draw stars rating
 *
 * Copyright (C) 2005 by Owen Hirst <n8rider@sbcglobal.net>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef RATINGWIDGET_H
#define RATINGWIDGET_H

// Qt includes

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT RatingWidget : public QWidget
{
    Q_OBJECT

public:

    RatingWidget(QWidget* parent);
    virtual ~RatingWidget();

    void setRating(int val);
    int  rating() const;

    void setTracking(bool tracking);
    bool hasTracking() const;

    void setFading(bool fading);
    bool hasFading() const;

    void startFading();
    void stopFading();
    void setVisibleImmediately();

    void setVisible(bool visible);
    int maximumVisibleWidth() const;

Q_SIGNALS:

    void signalRatingChanged(int);

protected:

    int regPixmapWidth() const;

    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void paintEvent(QPaintEvent*);

    QPixmap starPixmap() const;
    QPixmap starPixmapFilled() const;
    QPixmap starPixmapDisabled() const;

    void regeneratePixmaps();
    void setupTimeLine();
    void applyFading(QPixmap& pix);

protected Q_SLOTS:

    void setFadingValue(int value);

private Q_SLOTS:

    void slotThemeChanged();

private:

    class RatingWidgetPriv;
    RatingWidgetPriv* const d;
};

}  // namespace Digikam

#endif // RATINGWIDGET_H
