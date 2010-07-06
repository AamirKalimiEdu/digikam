/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-31-01
 * Description : main digiKam interface implementation
 *
 * Copyright (C) 2007-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAMAPPPRIVATE_H
#define DIGIKAMAPPPRIVATE_H

// Qt includes

#include <QByteArray>
#include <QEventLoop>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QSignalMapper>
#include <QString>
#include <QTimer>
#include <QToolButton>

// KDE includes

#include <kconfig.h>
#include <kaction.h>
#include <kselectaction.h>
#include <kmenu.h>
#include <kstatusbar.h>

// LibKIPI includes

#include <libkipi/pluginloader.h>

// Local includes

#include "albummanager.h"
#include "albumsettings.h"
#include "cameralist.h"
#include "templatemanager.h"
#include "imagepluginloader.h"
#include "splashscreen.h"
#include "kipiinterface.h"
#include "dzoombar.h"
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
class CameraUI;
class SearchTextBar;
class AlbumIconViewFilter;

class DigikamAppPriv
{
public:

    DigikamAppPriv()
    {
        fullScreen                           = false;
        autoShowZoomToolTip                  = false;
        validIccPath                         = true;
        cameraMenu                           = 0;
        usbMediaMenu                         = 0;
        cardReaderMenu                       = 0;
        newAction                            = 0;
        moveSelectionToAlbumAction           = 0;
        deleteAction                         = 0;
        imageDeletePermanentlyAction         = 0;
        imageDeletePermanentlyDirectlyAction = 0;
        imageTrashDirectlyAction             = 0;
        albumSortAction                      = 0;
        recurseAlbumsAction                  = 0;
        recurseTagsAction                    = 0;
        backwardActionMenu                   = 0;
        forwardActionMenu                    = 0;
        addImagesAction                      = 0;
        propsEditAction                      = 0;
        addFoldersAction                     = 0;
        openInKonquiAction                   = 0;
        openInTerminalAction                 = 0;
        refreshAlbumAction                   = 0;
        writeAlbumMetadataAction             = 0;
        readAlbumMetadataAction              = 0;
        imageWriteMetadataAction             = 0;
        imageReadMetadataAction              = 0;
        newTagAction                         = 0;
        deleteTagAction                      = 0;
        editTagAction                        = 0;
        imagePreviewAction                   = 0;
        imageViewAction                      = 0;
        imageLightTableAction                = 0;
        imageAddLightTableAction             = 0;
        imageAddCurrentQueueAction           = 0;
        imageAddNewQueueAction               = 0;
        imageFindSimilarAction               = 0;
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
        imageGroupAction                     = 0;
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
        rawCameraListAction                  = 0;
        libsInfoAction                       = 0;
        dbStatAction                         = 0;
        kipiHelpAction                       = 0;
        donateMoneyAction                    = 0;
        addCameraSeparatorAction             = 0;
        themeMenuAction                      = 0;
        forwardSignalMapper                  = 0;
        backwardSignalMapper                 = 0;
        dcopIface                            = 0;
        imagePluginsLoader                   = 0;
        kipiInterface                        = 0;
        cameraList                           = 0;
        templateManager                      = 0;
        statusProgressBar                    = 0;
        statusNavigateBar                    = 0;
        zoomBar                              = 0;
        kipiPluginLoader                     = 0;
        view                                 = 0;
        splashScreen                         = 0;
        zoomTo100percents                    = 0;
        zoomFitToWindowAction                = 0;
        zoomPlusAction                       = 0;
        zoomMinusAction                      = 0;
        manualCameraActionGroup              = 0;
        solidCameraActionGroup               = 0;
        solidUsmActionGroup                  = 0;
        exifOrientationActionGroup           = 0;
        eventLoop                            = 0;
        albumIconViewFilter                  = 0;
        contributeAction                     = 0;
        showBarAction                        = 0;
        showMenuBarAction                    = 0;
        kipipluginsActionCollection          = 0;
        modelCollection                      = 0;
        scriptConsoleAction                  = 0;
    }

    bool                                fullScreen;
    bool                                autoShowZoomToolTip;
    bool                                validIccPath;

    // KIPI plugins support
    QList<QAction*>                     kipiFileActionsExport;
    QList<QAction*>                     kipiFileActionsImport;
    QList<QAction*>                     kipiImageActions;
    QList<QAction*>                     kipiToolsActions;
    QList<QAction*>                     kipiBatchActions;
    QList<QAction*>                     kipiAlbumActions;

    KActionMenu*                        cameraMenu;
    KActionMenu*                        usbMediaMenu;
    KActionMenu*                        cardReaderMenu;

    KSharedConfig::Ptr                  config;

    // KIPI actionCollection
    KActionCollection*                  kipipluginsActionCollection;

    // Album Actions
    KAction*                            newAction;
    KAction*                            moveSelectionToAlbumAction;
    KAction*                            deleteAction;
    KAction*                            imageDeletePermanentlyAction;
    KAction*                            imageDeletePermanentlyDirectlyAction;
    KAction*                            imageTrashDirectlyAction;
    KToolBarPopupAction*                backwardActionMenu;
    KToolBarPopupAction*                forwardActionMenu;

    KAction*                            addImagesAction;
    KAction*                            propsEditAction;
    KAction*                            addFoldersAction;
    KAction*                            openInKonquiAction;
    KAction*                            openInTerminalAction;
    KAction*                            refreshAlbumAction;
    KAction*                            writeAlbumMetadataAction;
    KAction*                            readAlbumMetadataAction;

    // Tag Actions
    KAction*                            newTagAction;
    KAction*                            deleteTagAction;
    KAction*                            editTagAction;

    // Image Actions
    KToggleAction*                      imagePreviewAction;
    KAction*                            imageLightTableAction;
    KAction*                            imageAddLightTableAction;
    KAction*                            imageAddCurrentQueueAction;
    KAction*                            imageAddNewQueueAction;
    KAction*                            imageViewAction;
    KAction*                            imageWriteMetadataAction;
    KAction*                            imageReadMetadataAction;
    KAction*                            imageFindSimilarAction;
    KToggleAction*                      imageSetExifOrientation1Action;
    KToggleAction*                      imageSetExifOrientation2Action;
    KToggleAction*                      imageSetExifOrientation3Action;
    KToggleAction*                      imageSetExifOrientation4Action;
    KToggleAction*                      imageSetExifOrientation5Action;
    KToggleAction*                      imageSetExifOrientation6Action;
    KToggleAction*                      imageSetExifOrientation7Action;
    KToggleAction*                      imageSetExifOrientation8Action;
    KAction*                            imageRenameAction;
    KAction*                            imageDeleteAction;
    KActionMenu*                        imageExifOrientationActionMenu;

    // Selection Actions
    KAction*                            selectAllAction;
    KAction*                            selectNoneAction;
    KAction*                            selectInvertAction;

    // View Actions
    QAction*                            fullScreenAction;
    KAction*                            zoomPlusAction;
    KAction*                            zoomMinusAction;
    KAction*                            zoomFitToWindowAction;
    KAction*                            zoomTo100percents;
    KSelectAction*                      imageSortAction;
    KSelectAction*                      imageSortOrderAction;
    KSelectAction*                      imageGroupAction;
    KSelectAction*                      albumSortAction;
    KToggleAction*                      recurseAlbumsAction;
    KToggleAction*                      recurseTagsAction;
    KToggleAction*                      showBarAction;
    KToggleAction*                      showMenuBarAction;

    KAction*                            rating0Star;
    KAction*                            rating1Star;
    KAction*                            rating2Star;
    KAction*                            rating3Star;
    KAction*                            rating4Star;
    KAction*                            rating5Star;

    // Tools Actions
    KActionMenu*                        slideShowAction;
    KAction*                            slideShowAllAction;
    KAction*                            slideShowSelectionAction;
    KAction*                            slideShowRecursiveAction;
    KAction*                            scriptConsoleAction;

    // Application Actions
    KAction*                            rawCameraListAction;
    KAction*                            libsInfoAction;
    KAction*                            dbStatAction;
    KAction*                            kipiHelpAction;
    KAction*                            donateMoneyAction;
    KAction*                            contributeAction;
    KSelectAction*                      themeMenuAction;
    QAction*                            addCameraSeparatorAction;
    QAction*                            quitAction;
    QAction*                            tipAction;

    QSignalMapper*                      backwardSignalMapper;
    QSignalMapper*                      forwardSignalMapper;
    QActionGroup*                       manualCameraActionGroup;
    QActionGroup*                       solidCameraActionGroup;
    QActionGroup*                       solidUsmActionGroup;
    QActionGroup*                       exifOrientationActionGroup;

    QMap<QString, QPointer<CameraUI> >  cameraUIMap;

    QEventLoop*                         eventLoop;
    QString                             solidErrorMessage;

    AlbumIconViewFilter*                albumIconViewFilter;
    SplashScreen*                       splashScreen;
    DCOPIface*                          dcopIface;
    ImagePluginLoader*                  imagePluginsLoader;
    KipiInterface*                      kipiInterface;
    DigikamView*                        view;
    CameraList*                         cameraList;
    TemplateManager*                    templateManager;
    DZoomBar*                           zoomBar;
    StatusProgressBar*                  statusProgressBar;
    StatusNavigateBar*                  statusNavigateBar;
    QString                             statusBarSelectionText;

    KIPI::PluginLoader*                 kipiPluginLoader;

    DigikamModelCollection*             modelCollection;
};

}  // namespace Digikam

#endif // ALBUMICONVIEW_H
