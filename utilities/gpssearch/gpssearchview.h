/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-30
 * Description : GPS search sidebar tab contents.
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#ifndef GPSSEARCHVIEW_H
#define GPSSEARCHVIEW_H

// Qt includes

#include <QWidget>

// local includes

#include "statesavingobject.h"
#include "worldmapwidget.h"

class QDragEnterEvent;
class QDropEvent;
class QPixmap;

namespace Digikam
{

class Album;
class SAlbum;
class ImageInfo;
class ImageInfoList;
class SearchTextBar;
class GPSSearchFolderView;
class GPSSearchViewPriv;
class SearchModel;
class SearchModificationHelper;

class GPSSearchView : public QWidget, public StateSavingObject
{
    Q_OBJECT

public:

    GPSSearchView(QWidget* parent, SearchModel* searchModel,
                  SearchModificationHelper* searchModificationHelper);
    ~GPSSearchView();

    void setActive(bool val);

    void changeAlbumFromHistory(SAlbum* album);

    virtual void setConfigGroup(KConfigGroup group);
    void doLoadState();
    void doSaveState();

public Q_SLOTS:
    void slotDigikamViewNoCurrentItem();
    void slotDigikamViewImageSelected(const ImageInfoList& selectedImage, bool hasPrevious, bool hasNext, const ImageInfoList& allImages);

private:


    bool checkName(QString& name);
    bool checkAlbum(const QString& name) const;

    void createNewGPSSearchAlbum(const QString& name);

private Q_SLOTS:

    void slotAlbumSelected(Album*);

    void slotSaveGPSSAlbum();
    void slotCheckNameEditGPSConditions();

    void slotSelectionChanged();

    void slotItemsInfo(const ImageInfoList&);

    void slotMapSelectedItems(const GPSInfoList& gpsList);
    void slotMapSoloItems(const GPSInfoList& gpsList);

Q_SIGNALS:
    void signalMapSelectedItems(const KUrl::List url);
    void signalMapSoloItems(const KUrl::List url, const QString& id);

private:

    GPSSearchViewPriv* const d;
};

}  // namespace Digikam

#endif /* FUZZYSEARCHVIEW_H */
