/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-31-01
 * Description : main digiKam interface implementation
 * 
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QList>
#include <QByteArray>
#include <QString>
#include <QMap>
#include <QSignalMapper>
#include <QToolButton>
#include <QTimer>

// KDE includes.

#include <kconfig.h>
#include <kaction.h>
#include <kselectaction.h>
#include <kmenu.h>
#include <kstatusbar.h>

// libKipi includes.

#include <libkipi/pluginloader.h>

// Local includes.

#include "albummanager.h"
#include "albumsettings.h"
#include "cameralist.h"
#include "imagepluginloader.h"
#include "splashscreen.h"
#include "kipiinterface.h"
#include "statuszoombar.h"
#include "statusprogressbar.h"
#include "statusnavigatebar.h"
#include "digikamview.h"

class KToolBarPopupAction;
class KToggleAction;
class KActionMenu;
class KSelectAction;

namespace Digikam
{

class DCOPIface;

class DigikamAppPriv
{
public:

    DigikamAppPriv()
    {
        fullScreen                           = false;
        validIccPath                         = true;
        usbMediaMenu                         = 0;
        cardReaderMenu                       = 0;
        newAction                            = 0;
        deleteAction                         = 0;
        imageDeletePermanentlyAction         = 0;
        imageDeletePermanentlyDirectlyAction = 0;
        imageTrashDirectlyAction             = 0;
        albumSortAction                      = 0;
        backwardActionMenu                   = 0;
        forwardActionMenu                    = 0;
        addImagesAction                      = 0;
        propsEditAction                      = 0;
        albumImportAction                    = 0;
        openInKonquiAction                   = 0;
        refreshAlbumAction                   = 0;
        syncAlbumMetadataAction              = 0;
        newTagAction                         = 0;
        deleteTagAction                      = 0;
        editTagAction                        = 0;
        imagePreviewAction                   = 0;
        imageViewAction                      = 0;
        imageLightTableAction                = 0;
        imageSetExifOrientation1Action       = 0;
        imageSetExifOrientation2Action       = 0;
        imageSetExifOrientation3Action       = 0;
        imageSetExifOrientation4Action       = 0;
        imageSetExifOrientation5Action       = 0;
        imageSetExifOrientation6Action       = 0;
        imageSetExifOrientation7Action       = 0;
        imageSetExifOrientation8Action       = 0;
        imageRenameAction                    = 0;
        imageDeleteAction                    = 0;
        imageSortAction                      = 0;
        imageExifOrientationActionMenu       = 0;
        selectAllAction                      = 0;
        selectNoneAction                     = 0;
        selectInvertAction                   = 0;
        fullScreenAction                     = 0;
        slideShowAction                      = 0;
        slideShowAllAction                   = 0;
        slideShowSelectionAction             = 0;
        slideShowRecursiveAction             = 0;
        rating0Star                          = 0;
        rating1Star                          = 0;
        rating2Star                          = 0;
        rating3Star                          = 0;
        rating4Star                          = 0;
        rating5Star                          = 0;
        quitAction                           = 0;
        tipAction                            = 0;
        kipiHelpAction                       = 0;
        donateMoneyAction                    = 0;
        cameraMenuAction                     = 0;
        themeMenuAction                      = 0;
        forwardSignalMapper                  = 0;
        backwardSignalMapper                 = 0;
        albumSettings                        = 0;
        albumManager                         = 0;
        dcopIface                            = 0;
        imagePluginsLoader                   = 0;
        kipiInterface                        = 0;
        cameraList                           = 0;
        statusProgressBar                    = 0;
        statusNavigateBar                    = 0;    
        statusZoomBar                        = 0;    
        kipiPluginLoader                     = 0;
        view                                 = 0;
        splashScreen                         = 0;
        zoomTo100percents                    = 0;
        zoomFitToWindowAction                = 0;
        zoomPlusAction                       = 0;
        zoomMinusAction                      = 0;
        cameraActionGroup                    = 0;
        solidActionGroup                     = 0;
        eventLoop                            = 0;
    }

    bool                   fullScreen;
    bool                   validIccPath;
    
    // KIPI plugins support
    QList<QAction*>        kipiFileActionsExport;
    QList<QAction*>        kipiFileActionsImport;
    QList<QAction*>        kipiImageActions;
    QList<QAction*>        kipiToolsActions;
    QList<QAction*>        kipiBatchActions;
    QList<QAction*>        kipiAlbumActions;

    QString                cameraGuiPath;

    KMenu                 *usbMediaMenu;
    KMenu                 *cardReaderMenu;

    KSharedConfig::Ptr     config;    
    
    // Album Actions
    KAction               *newAction;
    KAction               *deleteAction;
    KAction               *imageDeletePermanentlyAction;
    KAction               *imageDeletePermanentlyDirectlyAction;
    KAction               *imageTrashDirectlyAction;
    KToolBarPopupAction   *backwardActionMenu;
    KToolBarPopupAction   *forwardActionMenu;

    KAction               *addImagesAction;
    KAction               *propsEditAction;
    KAction               *albumImportAction;
    KAction               *openInKonquiAction;
    KAction               *refreshAlbumAction;
    KAction               *syncAlbumMetadataAction;
    
    // Tag Actions
    KAction               *newTagAction;
    KAction               *deleteTagAction;
    KAction               *editTagAction;
    
    // Image Actions
    KToggleAction         *imagePreviewAction;
    KAction               *imageLightTableAction;
    KAction               *imageViewAction;
    KAction               *imageSetExifOrientation1Action;
    KAction               *imageSetExifOrientation2Action;
    KAction               *imageSetExifOrientation3Action;
    KAction               *imageSetExifOrientation4Action;
    KAction               *imageSetExifOrientation5Action;
    KAction               *imageSetExifOrientation6Action;
    KAction               *imageSetExifOrientation7Action;
    KAction               *imageSetExifOrientation8Action;
    KAction               *imageRenameAction;
    KAction               *imageDeleteAction;
    KActionMenu           *imageExifOrientationActionMenu;

    // Selection Actions
    KAction               *selectAllAction;
    KAction               *selectNoneAction;
    KAction               *selectInvertAction;

    // View Actions
    QAction               *fullScreenAction;
    QAction               *zoomPlusAction;
    QAction               *zoomMinusAction;
    KAction               *zoomFitToWindowAction;
    KAction               *zoomTo100percents;
    KActionMenu           *slideShowAction;
    KAction               *slideShowAllAction;
    KAction               *slideShowSelectionAction;
    KAction               *slideShowRecursiveAction;
    KSelectAction         *imageSortAction;
    KSelectAction         *albumSortAction;

    KAction               *rating0Star;
    KAction               *rating1Star;
    KAction               *rating2Star;
    KAction               *rating3Star;
    KAction               *rating4Star;
    KAction               *rating5Star;

    // Application Actions
    QAction               *quitAction;
    QAction               *tipAction;
    KAction               *kipiHelpAction;
    KAction               *donateMoneyAction;
    KActionMenu           *cameraMenuAction;
    KSelectAction         *themeMenuAction;

    QSignalMapper         *backwardSignalMapper;
    QSignalMapper         *forwardSignalMapper;
    QActionGroup          *cameraActionGroup;
    QActionGroup          *solidActionGroup;

    QEventLoop            *eventLoop;
    QString                solidErrorMessage;

    AlbumSettings         *albumSettings;
    AlbumManager          *albumManager;
    SplashScreen          *splashScreen;
    DCOPIface             *dcopIface;
    ImagePluginLoader     *imagePluginsLoader;
    DigikamKipiInterface  *kipiInterface;
    DigikamView           *view;
    CameraList            *cameraList;
    StatusZoomBar         *statusZoomBar;
    StatusProgressBar     *statusProgressBar;
    StatusNavigateBar     *statusNavigateBar;    

    KIPI::PluginLoader    *kipiPluginLoader;
};

}  // namespace Digikam
