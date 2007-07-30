/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation
 * 
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu> 
 * Copyright (C)      2006 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2002-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAMAPP_H
#define DIGIKAMAPP_H

// Qt includes.

#include <QList>
#include <QAction>
#include <QString>

// KDE includes.

#include <kxmlguiwindow.h>
#include <kio/global.h>
#include <kio/netaccess.h>

// Local includes.

#include "digikam_export.h"


namespace Digikam
{

class ImageInfo;
class ImageInfoList;
class CameraType;
class DigikamAppPriv;

class DIGIKAM_EXPORT DigikamApp : public KXmlGuiWindow
{
    Q_OBJECT

public:

    DigikamApp();
    ~DigikamApp();

    virtual void show();

    static DigikamApp* getinstance();

    // KIPI Actions collections access.
    const QList<QAction*>& menuImageActions();
    const QList<QAction*>& menuBatchActions();
    const QList<QAction*>& menuAlbumActions();

    QList<QAction*> menuImportActions();
    const QList<QAction*>& menuExportActions();

    void autoDetect();
    void downloadFrom(const QString &cameraGuiPath);
    void enableZoomPlusAction(bool val);
    void enableZoomMinusAction(bool val);
    void enableAlbumBackwardHistory(bool enable);
    void enableAlbumForwardHistory(bool enable);

signals:

    void signalEscapePressed();
    void signalNextItem();
    void signalPrevItem();
    void signalFirstItem();
    void signalLastItem();
    void signalCopyAlbumItemsSelection();
    void signalPasteAlbumItemsSelection();
    void signalCancelButtonPressed();

protected:

    bool queryClose();

protected slots:

    void slotCameraMediaMenuEntries( KIO::Job *, const KIO::UDSEntryList & );

private:

    bool setup(bool iccSetupPage=false);
    void setupView();
    void setupStatusBar();
    void setupActions();
    void setupAccelerators();
    void loadPlugins();
    void loadCameras();
    void populateThemes();

private slots:

    void slotAlbumSelected(bool val);
    void slotTagSelected(bool val);
    void slotImageSelected(const ImageInfoList&, bool, bool);
    void slotExit();
    void slotShowTip();
    void slotShowKipiHelp();
    void slotDonateMoney();

    void slotAboutToShowForwardMenu();
    void slotAboutToShowBackwardMenu();
            
    void slotSetup();
    void slotSetupCamera();
    void slotSetupChanged();

    void slotKipiPluginPlug();
    
    QString convertToLocalUrl( const QString& folder );
    void slotDownloadImages( const QString& folder );
    void slotDownloadImages();
    void slotCameraConnect();
    void slotCameraMediaMenu();
    void slotCameraAdded(CameraType *ctype);
    void slotCameraRemoved(CameraType *ctype);
    void slotCameraAutoDetect();
    void downloadImages(const QString& folder);
    void cameraAutoDetect();
    void slotEditKeys();
    void slotConfToolbars();
    void slotToggleFullScreen();

    void slotDatabaseRescan();
    void slotRebuildAllThumbs();
    void slotRebuildAllThumbsDone();
    void slotSyncAllPicturesMetadata();
    void slotSyncAllPicturesMetadataDone();
    
    void slotChangeTheme(const QString& theme);

    void slotProgressBarMode(int, const QString&);
    void slotProgressValue(int);

    void slotZoomSliderChanged(int);
    void slotThumbSizeChanged(int);
    void slotZoomChanged(double, int);
    void slotTooglePreview(bool);

private:

    DigikamAppPriv    *d;

    static DigikamApp *m_instance;
};

}  // namespace Digikam

#endif  // DIGIKAMAPP_H
