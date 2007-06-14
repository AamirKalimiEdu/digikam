/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2005-04-27
 * Descritpion : a folder view for date albums.
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

#ifndef DATEFOLDERVIEW_H
#define DATEFOLDERVIEW_H

// Qt includes.

#include <q3vbox.h>

namespace Digikam
{

class DateFolderViewPriv;
class DAlbum;

class DateFolderView : public Q3VBox
{
    Q_OBJECT
    
public:

    DateFolderView(QWidget* parent);
    ~DateFolderView();

    void setActive(bool val);

    void setSelected(Q3ListViewItem *item);
    
private slots:

    void slotAllDAlbumsLoaded();
    void slotAlbumAdded(Album* album);
    void slotAlbumDeleted(Album* album);
    void slotSelectionChanged();
    
private:

    /**
     * load the last view state from disk
     */
    void loadViewState();
    
    /**
     * writes the view state to disk
     */
    void saveViewState();
    
    
    DateFolderViewPriv* d;
};

}  // namespace Digikam

#endif /* DATEFOLDERVIEW_H */
