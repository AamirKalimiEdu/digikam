/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : albums settings interface
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007 by Arnd Baecker <arnd dot baecker at web dot de>
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

#ifndef ALBUMSETTINGS_H
#define ALBUMSETTINGS_H

// Qt includes.

#include <QStringList>
#include <QString>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class AlbumSettingsPrivate;

class AlbumSettings 
{
public:

    enum AlbumSortOrder
    {
        ByFolder = 0,
        ByCollection,
        ByDate
    };

    enum ImageSortOrder
    {
        ByIName = 0,
        ByIPath,
        ByIDate,
        ByISize,
        ByIRating
    };

    enum ItemRightClickAction
    {
        ShowPreview = 0,
        StartEditor
    };

    void readSettings();
    void saveSettings();

    bool showToolTipsIsValid() const;

    void setDatabaseFilePath(const QString& path);
    QString getDatabaseFilePath() const;

    void setShowSplashScreen(bool val);
    bool getShowSplashScreen() const;

    void setScanAtStart(bool val);
    bool getScanAtStart() const;

    void setAlbumCollectionNames(const QStringList& list);
    QStringList getAlbumCollectionNames();

    bool addAlbumCollectionName(const QString& name);
    bool delAlbumCollectionName(const QString& name);

    void setAlbumSortOrder(const AlbumSortOrder order);
    AlbumSortOrder getAlbumSortOrder() const;

    void setImageSortOrder(const ImageSortOrder order);
    ImageSortOrder getImageSortOrder() const;

    void setItemRightClickAction(const ItemRightClickAction action);
    ItemRightClickAction getItemRightClickAction() const;

    QString getImageFileFilter() const;
    void addToImageFileFilter(const QString &extensions);

    QString getMovieFileFilter() const;

    QString getAudioFileFilter() const;

    QString getRawFileFilter() const;

    bool    addImageFileExtension(const QString& ext);
    QString getAllFileFilter() const;

    void setDefaultIconSize(int val);
    int  getDefaultIconSize() const;

    void setDefaultTreeIconSize(int val);
    int  getDefaultTreeIconSize() const;

    void setRatingFilterCond(int val);
    int  getRatingFilterCond() const;

    void setIconShowName(bool val);
    bool getIconShowName() const;

    void setIconShowSize(bool val);
    bool getIconShowSize() const;

    void setIconShowComments(bool val);
    bool getIconShowComments() const;

    void setIconShowResolution(bool val);
    bool getIconShowResolution() const;

    void setIconShowTags(bool val);
    bool getIconShowTags() const;

    void setIconShowDate(bool val);
    bool getIconShowDate() const;

    void setIconShowModDate(bool val);
    bool getIconShowModDate() const;

    void setIconShowRating(bool val);
    bool getIconShowRating() const;

    void setExifRotate(bool val);
    bool getExifRotate() const;

    void setExifSetOrientation(bool val);
    bool getExifSetOrientation() const;

    void setSaveTags(bool val);
    bool getSaveTags() const;

    void setSavePhotographerId(bool val);
    bool getSavePhotographerId() const;

    void setSaveCredits(bool val);
    bool getSaveCredits() const;

    void setWriteRawFiles(bool val);
    bool getWriteRawFiles() const;

    void setAuthor(const QString& author);
    QString getAuthor() const;

    void setAuthorTitle(const QString& authorTitle);
    QString getAuthorTitle() const;

    void setCredit(const QString& credit);
    QString getCredit() const;

    void setSource(const QString& source);
    QString getSource() const;

    void setCopyright(const QString& copyright);
    QString getCopyright() const;

    void setSaveComments(bool val);
    bool getSaveComments() const;

    void setSaveDateTime(bool val);
    bool getSaveDateTime() const;

    void setSaveRating(bool val);
    bool getSaveRating() const;

    void setShowToolTips(bool val);
    bool getShowToolTips() const;

    void setToolTipsShowFileName(bool val);
    bool getToolTipsShowFileName() const;

    void setToolTipsShowFileDate(bool val);
    bool getToolTipsShowFileDate() const;

    void setToolTipsShowFileSize(bool val);
    bool getToolTipsShowFileSize() const;

    void setToolTipsShowImageType(bool val);
    bool getToolTipsShowImageType() const;

    void setToolTipsShowImageDim(bool val);
    bool getToolTipsShowImageDim() const;

    void setToolTipsShowPhotoMake(bool val);
    bool getToolTipsShowPhotoMake() const;

    void setToolTipsShowPhotoDate(bool val);
    bool getToolTipsShowPhotoDate() const;

    void setToolTipsShowPhotoFocal(bool val);
    bool getToolTipsShowPhotoFocal() const;

    void setToolTipsShowPhotoExpo(bool val);
    bool getToolTipsShowPhotoExpo() const;

    void setToolTipsShowPhotoMode(bool val);
    bool getToolTipsShowPhotoMode() const;

    void setToolTipsShowPhotoFlash(bool val);
    bool getToolTipsShowPhotoFlash() const;

    void setToolTipsShowPhotoWB(bool val);
    bool getToolTipsShowPhotoWB() const;

    void setToolTipsShowAlbumName(bool val);
    bool getToolTipsShowAlbumName() const;

    void setToolTipsShowComments(bool val);
    bool getToolTipsShowComments() const;

    void setToolTipsShowTags(bool val);
    bool getToolTipsShowTags() const;

    void setToolTipsShowRating(bool val);
    bool getToolTipsShowRating() const;

    void    setCurrentTheme(const QString& theme);
    QString getCurrentTheme() const;

    void    setUseTrash(bool val);
    bool    getUseTrash() const;

    void    setShowTrashDeleteDialog(bool val);
    bool    getShowTrashDeleteDialog() const;

    void    setApplySidebarChangesDirectly(bool val);
    bool    getApplySidebarChangesDirectly() const;

    void setPreviewLoadFullImageSize(bool val);
    bool getPreviewLoadFullImageSize() const;

    void setShowFolderTreeViewItemsCount(bool val);
    bool getShowFolderTreeViewItemsCount() const;

    void setRecurseAlbums(bool val);
    bool getRecurseAlbums() const;

    void setRecurseTags(bool val);
    bool getRecurseTags() const;

    void setShowThumbbar(bool val);
    bool getShowThumbbar() const;

    static AlbumSettings *instance();

private:

    friend class AlbumSettingsCreator;
    AlbumSettings();
    ~AlbumSettings();

    void init();

    AlbumSettingsPrivate* const d;
};

}  // namespace Digikam

#endif  // ALBUMSETTINGS_H
