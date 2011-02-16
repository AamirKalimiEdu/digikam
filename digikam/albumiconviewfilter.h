/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-27
 * Description : a bar to filter album contents
 *
 * Copyright (C) 2007-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ALBUMICONVIEWFILTER_H
#define ALBUMICONVIEWFILTER_H

// Qt includes

#include <QString>
#include <QEvent>

// KDE includes

#include "khbox.h"

// Local includes

#include "searchtextbar.h"
#include "imagefiltersettings.h"

class QEvent;
class QObject;

namespace Digikam
{

class AlbumIconViewFilter : public KHBox
{
    Q_OBJECT

public:

    AlbumIconViewFilter(QWidget* parent);
    ~AlbumIconViewFilter();

public Q_SLOTS:

    void slotFilterMatches(bool);
    void slotFilterMatchesForText(bool);
    void slotFilterSettingsChanged(const ImageFilterSettings& settings);

Q_SIGNALS:

    void signalResetFilters();
    void mimeTypeFilterChanged(int);
    void textFilterChanged(const SearchTextSettings&);

private:

    bool eventFilter(QObject* object, QEvent* e);

private:

    class AlbumIconViewFilterPriv;
    AlbumIconViewFilterPriv* const d;
};

}  // namespace Digikam

#endif // ALBUMICONVIEWFILTER_H
