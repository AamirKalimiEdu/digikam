/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-08-15
 * Description : a widget to draw stars rating
 *
 * Copyright (C) 2005 by Owen Hirst <n8rider@sbcglobal.net>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes.

#include "ratingwidget.h"
#include "ratingwidget.moc"

// C++ includes.

#include <cmath>

// Qt includes.

#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPolygon>

// Digikam includes.

#include "constants.h"
#include "themeengine.h"

namespace Digikam
{

class RatingWidgetPriv
{
public:

    RatingWidgetPriv()
    {
        rating = 0;

        // Pre-computed star polygon for a 15x15 pixmap.
        starPolygon << QPoint(0,  6);
        starPolygon << QPoint(5,  5);
        starPolygon << QPoint(7,  0);
        starPolygon << QPoint(9,  5);
        starPolygon << QPoint(14, 6);
        starPolygon << QPoint(10, 9);
        starPolygon << QPoint(11, 14);
        starPolygon << QPoint(7,  11);
        starPolygon << QPoint(3,  14);
        starPolygon << QPoint(4,  9);
    }

    int      rating;

    QPolygon starPolygon;

    QPixmap  selPixmap;      // Selected star.
    QPixmap  regPixmap;      // Regular star.
    QPixmap  disPixmap;      // Disable star.
};

RatingWidget::RatingWidget(QWidget* parent)
            : QWidget(parent)
{
    d = new RatingWidgetPriv;

    slotThemeChanged();

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

RatingWidget::~RatingWidget()
{
    delete d;
}

int RatingWidget::regPixmapWidth() const
{
    return d->regPixmap.width();
}

void RatingWidget::setRating(int val)
{
    if (val < RatingMin || val > RatingMax) return;

    d->rating = val;
    emit signalRatingChanged(d->rating);
    update();
}

int RatingWidget::rating() const
{
    return d->rating;
}

void RatingWidget::mouseMoveEvent(QMouseEvent* e)
{
    int pos = e->x() / d->regPixmap.width() +1;

    if (d->rating != pos)
    {
        if (pos > RatingMax)       // B.K.O.: # 151357
            pos = RatingMax;
        if (pos < RatingMin)
            pos = RatingMin;

        d->rating = pos;
        emit signalRatingChanged(d->rating);
        update();
    }
}

void RatingWidget::mousePressEvent(QMouseEvent* e)
{
    int pos = e->x() / d->regPixmap.width() +1;

    if (d->rating == pos)
    {
        d->rating--;
    }
    else
    {
        d->rating = pos;
    }

    emit signalRatingChanged(d->rating);

    update();
}

void RatingWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    // Widget is disable : drawing grayed frame.
    if (!isEnabled())
    {
        int x = 0;
        for (int i = 0; i < RatingMax; i++)
        {
            p.drawPixmap(x, 0, d->disPixmap);
            x += d->disPixmap.width();
        }
    }
    else
    {
        int x = 0;
        for (int i = 0; i < d->rating; i++)
        {
            p.drawPixmap(x, 0, d->selPixmap);
            x += d->selPixmap.width();
        }

        for (int i = d->rating; i < RatingMax; i++)
        {
            p.drawPixmap(x, 0, d->regPixmap);
            x += d->regPixmap.width();
        }
    }

    p.end();
}

QPixmap RatingWidget::starPixmapDisabled() const
{
    return d->disPixmap;
}

QPixmap RatingWidget::starPixmapFilled() const
{
    return d->selPixmap;
}

QPixmap RatingWidget::starPixmap() const
{
    return d->regPixmap;
}

void RatingWidget::regeneratePixmaps()
{
    slotThemeChanged();
}

void RatingWidget::slotThemeChanged()
{
    d->regPixmap = QPixmap(15, 15);
    d->regPixmap.fill(Qt::transparent);
    d->selPixmap = QPixmap(15, 15);
    d->selPixmap.fill(Qt::transparent);
    d->disPixmap = QPixmap(15, 15);
    d->disPixmap.fill(Qt::transparent);

    QPainter p1(&d->regPixmap);
    p1.setRenderHint(QPainter::Antialiasing, true);
    p1.setBrush(palette().color(QPalette::Active, backgroundRole()));
    p1.setPen(palette().color(QPalette::Active, foregroundRole()));
    p1.drawPolygon(d->starPolygon, Qt::WindingFill);
    p1.end();

    QPainter p2(&d->selPixmap);
    p2.setRenderHint(QPainter::Antialiasing, true);
    p2.setBrush(ThemeEngine::instance()->textSpecialRegColor());
    p2.setPen(palette().color(QPalette::Active, foregroundRole()));
    p2.drawPolygon(d->starPolygon, Qt::WindingFill);
    p2.end();

    QPainter p3(&d->disPixmap);
    p3.setRenderHint(QPainter::Antialiasing, true);
    p3.setBrush(palette().color(QPalette::Disabled, backgroundRole()));
    p3.setPen(palette().color(QPalette::Disabled, foregroundRole()));
    p3.drawPolygon(d->starPolygon, Qt::WindingFill);
    p3.end();

    setFixedSize(QSize(d->regPixmap.width()*RatingMax, d->regPixmap.height()));
    update();
}

}  // namespace Digikam
