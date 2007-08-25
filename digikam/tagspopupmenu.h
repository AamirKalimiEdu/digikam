/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-07
 * Description : a pop-up menu implementation to display a 
 *               hierarchical view of digiKam tags.
 * 
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#ifndef TAGSPOPUPMENU_H
#define TAGSPOPUPMENU_H

// Qt includes.

#include <QMenu>
#include <QList>

namespace Digikam
{

class TAlbum;
class TagsPopupMenuPriv;

class TagsPopupMenu : public QMenu
{
    Q_OBJECT

public:

    enum Mode
    {
        ASSIGN = 0,
        REMOVE
    };

    TagsPopupMenu(qlonglong selectedImageId, Mode mode);
    TagsPopupMenu(const QList<qlonglong>& selectedImageIDs, Mode mode);
    ~TagsPopupMenu();

signals:

    void signalTagActivated(int id);

private slots:

    void slotAboutToShow();
    void slotToggleTag(QAction *);
    void slotAddTag(QAction *);
    void slotTagThumbnail(Album *, const QPixmap&);

private:

    void         setup(Mode mode);
    void         clearPopup();
    QMenu*       buildSubMenu(int tagid);
    void         iterateAndBuildMenu(QMenu *menu, TAlbum *album);
    void         setAlbumIcon(QAction *action, TAlbum *album);

private:

    TagsPopupMenuPriv* d;
};

}  // namespace Digikam

#endif /* TAGSPOPUPMENU_H */
