/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-27
 * Description : Fuzzy search folder view
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FUZZYSEARCHFOLDERVIEW_H
#define FUZZYSEARCHFOLDERVIEW_H

// Digikam includes.

#include "folderview.h"

class Q3ListViewItem;

namespace Digikam
{

class Album;
class SAlbum;
class FuzzySearchFolderItem;

class FuzzySearchFolderView : public FolderView
{
    Q_OBJECT

public:

    FuzzySearchFolderView(QWidget* parent);
    ~FuzzySearchFolderView();

    void searchDelete(SAlbum* album);
    static QString currentFuzzySketchSearchName();
    static QString currentFuzzyImageSearchName();

signals:

    void signalTextSearchFilterMatch(bool);
    void signalAlbumSelected(SAlbum*);
    void signalRenameAlbum(SAlbum*);

public slots:

    void slotTextSearchFilterChanged(const QString&);

private slots:

    void slotAlbumAdded(Album*);
    void slotAlbumDeleted(Album*);
    void slotAlbumRenamed(Album*);
    void slotAlbumCurrentChanged(Album*);
    void slotSelectionChanged();
    void slotContextMenu(Q3ListViewItem*, const QPoint&, int);

protected:

    void selectItem(int id);
};

}  // namespace Digikam

#endif /* FUZZYSEARCHFOLDERVIEW_H */
