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
 * Copyright (C) 2002-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QDataStream>
#include <QLabel>
#include <QStringList>
#include <QSignalMapper>
#include <QtDBus>

// KDE includes.

#include <khbox.h>
#include <kactioncollection.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>
#include <kshortcutsdialog.h>
#include <ktoggleaction.h>
#include <kedittoolbar.h>
#include <kiconloader.h>
#include <ktip.h>
#include <kdeversion.h>
#include <kapplication.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kwindowsystem.h>
#include <kactionmenu.h>
#include <kglobal.h>
#include <ktoolinvocation.h>
#include <ktoolbarpopupaction.h>
#include <ktogglefullscreenaction.h>
#include <ktoolbar.h>
#include <kfiledialog.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>
#include <solid/camera.h>
#include <solid/predicate.h>

#include "config-digikam.h"
#ifdef HAVE_MARBLEWIDGET
#include <marble/global.h>
#endif // HAVE_MARBLEWIDGET

// libKipi includes.

#include <libkipi/plugin.h>
#include <libkipi/interface.h>

// Libkdcraw includes.

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/dcrawbinary.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albumdb.h"
#include "albumthumbnailloader.h"
#include "albumiconviewfilter.h"
#include "albumselectdialog.h"
#include "cameratype.h"
#include "cameraui.h"
#include "componentsinfo.h"
#include "setup.h"
#include "setupplugins.h"
#include "setupeditor.h"
#include "setupicc.h"
#include "lighttablewindow.h"
#include "imagewindow.h"
#include "imageinfo.h"
#include "thumbnailsize.h"
#include "themeengine.h"
#include "scancontroller.h"
#include "loadingcache.h"
#include "loadingcacheinterface.h"
#include "imageattributeswatch.h"
#include "batchthumbsgenerator.h"
#include "batchalbumssyncmetadata.h"
#include "fingerprintsgenerator.h"
#include "rawcameradlg.h"
#include "dlogoaction.h"
#include "dio.h"
#include "digikamadaptor.h"
#include "digikamappprivate.h"
#include "digikamapp.h"
#include "digikamapp.moc"

using KIO::Job;
using KIO::UDSEntryList;
using KIO::UDSEntry;

namespace Digikam
{

DigikamApp* DigikamApp::m_instance = 0;

DigikamApp::DigikamApp()
          : KXmlGuiWindow(0)
{
    d = new DigikamAppPriv;
    m_instance = this;
    d->config  = KGlobal::config();

    setObjectName("Digikam");

    if(d->config->group("General Settings").readEntry("Show Splash", true) &&
       !kapp->isSessionRestored())
    {
        d->splashScreen = new SplashScreen();
        d->splashScreen->show();
    }

    if(d->splashScreen)
        d->splashScreen->message(i18n("Scan Albums"), Qt::AlignLeft, Qt::white);

    // collection scan
    if (d->config->group("General Settings").readEntry("Scan At Start", true) ||
        Digikam::DatabaseAccess().db()->getSetting("Scanned").isEmpty())
    {
        Digikam::ScanController::instance()->completeCollectionScan(d->splashScreen);
    }

    if(d->splashScreen)
        d->splashScreen->message(i18n("Initializing..."), Qt::AlignLeft, Qt::white);

    new DigikamAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/Digikam", this);

    // ensure creation
    AlbumSettings::instance();
    AlbumManager::instance();
    AlbumLister::instance();

    LoadingCacheInterface::initialize();

    d->cameraSolidMenu          = new KMenu;
    d->usbMediaMenu             = new KMenu;
    d->cardReaderMenu           = new KMenu;
    d->manuallyAddedCamerasMenu = new KMenu;

    d->cameraList = new CameraList(this, KStandardDirs::locateLocal("appdata", "cameras.xml"));

    connect(d->cameraList, SIGNAL(signalCameraAdded(CameraType *)),
            this, SLOT(slotCameraAdded(CameraType *)));

    connect(d->cameraList, SIGNAL(signalCameraRemoved(CameraType *)),
            this, SLOT(slotCameraRemoved(CameraType *)));

    setupView();
    setupStatusBar();
    setupAccelerators();
    setupActions();

    applyMainWindowSettings(d->config->group("General Settings"));

    // Check ICC profiles repository availability

    if(d->splashScreen)
        d->splashScreen->message(i18n("Checking ICC repository"), Qt::AlignLeft, Qt::white);

    d->validIccPath = SetupICC::iccRepositoryIsValid();

    // Check witch dcraw version available

    if(d->splashScreen)
        d->splashScreen->message(i18n("Checking dcraw version"), Qt::AlignLeft, Qt::white);

    KDcrawIface::DcrawBinary::instance()->checkSystem();

    // Read albums from database
    if(d->splashScreen)
        d->splashScreen->message(i18n("Reading database"), Qt::AlignLeft, Qt::white);

    AlbumManager::instance()->startScan();

    // Load KIPI Plugins.
    loadPlugins();

    // Load Themes
    populateThemes();

    setAutoSaveSettings("General Settings", true);
}

DigikamApp::~DigikamApp()
{
    ImageAttributesWatch::shutDown();

    // Close and delete image editor instance.

    if (ImageWindow::imagewindowCreated())
    {
        // Delete after close
        ImageWindow::imagewindow()->setAttribute(Qt::WA_DeleteOnClose, true);
        // pass ownership of object - needed by ImageWindow destructor
        d->imagePluginsLoader->setParent(ImageWindow::imagewindow());
        // close the window
        ImageWindow::imagewindow()->close();
    }

    // Close and delete light table instance.

    if (LightTableWindow::lightTableWindowCreated())
    {
        LightTableWindow::lightTableWindow()->setAttribute(Qt::WA_DeleteOnClose, true);
        LightTableWindow::lightTableWindow()->close();
    }

    if (d->view)
        delete d->view;

    d->albumIconViewFilter->saveSettings();
    AlbumSettings::instance()->setRecurseAlbums(d->recurseAlbumsAction->isChecked());
    AlbumSettings::instance()->setRecurseTags(d->recurseTagsAction->isChecked());
    AlbumSettings::instance()->setShowThumbbar(d->showBarAction->isChecked());
    AlbumSettings::instance()->saveSettings();

    ScanController::instance()->shutDown();
    AlbumLister::cleanUp();
    ImageAttributesWatch::cleanUp();
    LoadingCacheInterface::cleanUp();
    KDcrawIface::DcrawBinary::cleanUp();

    m_instance = 0;

    delete d;
}

DigikamApp* DigikamApp::getinstance()
{
    return m_instance;
}

void DigikamApp::show()
{
    // Remove Splashscreen.

    if(d->splashScreen)
    {
        d->splashScreen->finish(this);
        delete d->splashScreen;
        d->splashScreen = 0;
    }

    // Display application window.

    KMainWindow::show();

    // Report errors from ICC repository path.

    if(!d->validIccPath)
    {
        QString message = i18n("<qt><p>The ICC profiles path seems to be invalid.</p>"
                               "<p>If you want to set it now, select \"Yes\", otherwise "
                               "select \"No\". In this case, \"Color Management\" feature "
                               "will be disabled until you solve this issue</p></qt>");

        if (KMessageBox::warningYesNo(this, message) == KMessageBox::Yes)
        {
            if (!setup(true))
            {
                d->config->group("Color Management").writeEntry("EnableCM", false);
                d->config->sync();
            }
        }
        else
        {
            d->config->group("Color Management").writeEntry("EnableCM", false);
            d->config->sync();
        }
    }

    // Report errors from dcraw detection.

    KDcrawIface::DcrawBinary::instance()->checkReport();

    // Init album icon view zoom factor.
    slotThumbSizeChanged(AlbumSettings::instance()->getDefaultIconSize());
    slotZoomSliderChanged(AlbumSettings::instance()->getDefaultIconSize());
}

const QList<QAction*>& DigikamApp::menuImageActions()
{
    return d->kipiImageActions;
}

const QList<QAction*>& DigikamApp::menuBatchActions()
{
    return d->kipiBatchActions;
}

const QList<QAction*>& DigikamApp::menuAlbumActions()
{
    return d->kipiAlbumActions;
}

const QList<QAction*>& DigikamApp::menuExportActions()
{
    return d->kipiFileActionsExport;
}

void DigikamApp::autoDetect()
{
    // Called from main if command line option is set

    if(d->splashScreen)
        d->splashScreen->message(i18n("Auto-detect camera"), Qt::AlignLeft, Qt::white);

    QTimer::singleShot(0, this, SLOT(slotCameraAutoDetect()));
}

void DigikamApp::downloadFrom(const QString &cameraGuiPath)
{
    // Called from main if command line option is set

    if (!cameraGuiPath.isNull())
    {
        d->cameraGuiPath = cameraGuiPath;

        if(d->splashScreen)
            d->splashScreen->message(i18n("Opening Download Dialog"), Qt::AlignLeft, Qt::white);

        QTimer::singleShot(0, this, SLOT(slotOpenCameraUiFromPath()));
    }
}

bool DigikamApp::queryClose()
{
    if (ImageWindow::imagewindowCreated())
    {
        return ImageWindow::imagewindow()->queryClose();
    }
    else
        return true;
}

void DigikamApp::setupView()
{
    if(d->splashScreen)
        d->splashScreen->message(i18n("Initializing Main View"), Qt::AlignLeft, Qt::white);

    d->view = new DigikamView(this);
    setCentralWidget(d->view);
    d->view->applySettings();

    connect(d->view, SIGNAL(signalAlbumSelected(bool)),
            this, SLOT(slotAlbumSelected(bool)));

    connect(d->view, SIGNAL(signalTagSelected(bool)),
            this, SLOT(slotTagSelected(bool)));

    connect(d->view, SIGNAL(signalImageSelected(const ImageInfoList&, bool, bool, const KUrl::List&)),
            this, SLOT(slotImageSelected(const ImageInfoList&, bool, bool, const KUrl::List&)));
}

void DigikamApp::setupStatusBar()
{
    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    statusBar()->addWidget(d->statusProgressBar, 100);

    //------------------------------------------------------------------------------

    d->albumIconViewFilter = new AlbumIconViewFilter(statusBar());
    statusBar()->addWidget(d->albumIconViewFilter, 100);

    //------------------------------------------------------------------------------

    d->statusZoomBar = new StatusZoomBar(statusBar());
    statusBar()->addPermanentWidget(d->statusZoomBar, 1);

    //------------------------------------------------------------------------------

    d->statusNavigateBar = new StatusNavigateBar(statusBar());
    statusBar()->addPermanentWidget(d->statusNavigateBar, 1);

    //------------------------------------------------------------------------------

    connect(d->statusZoomBar, SIGNAL(signalZoomMinusClicked()),
            d->view, SLOT(slotZoomOut()));

    connect(d->statusZoomBar, SIGNAL(signalZoomPlusClicked()),
            d->view, SLOT(slotZoomIn()));

    connect(d->statusZoomBar, SIGNAL(signalZoomSliderChanged(int)),
            this, SLOT(slotZoomSliderChanged(int)));

    connect(d->view, SIGNAL(signalThumbSizeChanged(int)),
            this, SLOT(slotThumbSizeChanged(int)));

    connect(d->view, SIGNAL(signalZoomChanged(double, int)),
            this, SLOT(slotZoomChanged(double, int)));

    connect(d->view, SIGNAL(signalTogglePreview(bool)),
            this, SLOT(slotTogglePreview(bool)));

    connect(d->albumIconViewFilter, SIGNAL(signalResetTagFilters()),
            this, SIGNAL(signalResetTagFilters()));

    connect(d->statusNavigateBar, SIGNAL(signalFirstItem()),
            d->view, SLOT(slotFirstItem()));

    connect(d->statusNavigateBar, SIGNAL(signalNextItem()),
            d->view, SLOT(slotNextItem()));

    connect(d->statusNavigateBar, SIGNAL(signalPrevItem()),
            d->view, SLOT(slotPrevItem()));

    connect(d->statusNavigateBar, SIGNAL(signalLastItem()),
            d->view, SLOT(slotLastItem()));

    connect(d->statusProgressBar, SIGNAL(signalCancelButtonPressed()),
            this, SIGNAL(signalCancelButtonPressed()));
}

void DigikamApp::setupAccelerators()
{
    // Action are added by <MainWindow> tag in ui.rc XML file
    KAction *escapeAction = new KAction(i18n("Exit preview mode"), this);
    actionCollection()->addAction("exit_preview_mode", escapeAction);
    escapeAction->setShortcut( QKeySequence(Qt::Key_Escape) );
    connect(escapeAction, SIGNAL(triggered()), this, SIGNAL(signalEscapePressed()));

    KAction *nextImageAction = new KAction(i18n("Next Image"), this);
    actionCollection()->addAction("next_image", nextImageAction);
    nextImageAction->setShortcut(KShortcut(Qt::Key_Space, Qt::Key_Next));
    connect(nextImageAction, SIGNAL(triggered()), this, SIGNAL(signalNextItem()));

    KAction *previousImageAction = new KAction(i18n("Previous Image"), this);
    actionCollection()->addAction("previous_image", previousImageAction);
    previousImageAction->setShortcut(KShortcut(Qt::Key_Backspace, Qt::Key_Prior));
    connect(previousImageAction, SIGNAL(triggered()), this, SIGNAL(signalPrevItem()));

    KAction *altpreviousImageAction = new KAction(i18n("Previous Image"), this);
    actionCollection()->addAction("alt_previous_image_shift_space", altpreviousImageAction);
    altpreviousImageAction->setShortcut( KShortcut(Qt::SHIFT+Qt::Key_Space) );
    connect(altpreviousImageAction, SIGNAL(triggered()), this, SIGNAL(signalPrevItem()));

    KAction *firstImageAction = new KAction(i18n("First Image"), this);
    actionCollection()->addAction("first_image", firstImageAction);
    firstImageAction->setShortcut( QKeySequence(Qt::Key_Home) );
    connect(firstImageAction, SIGNAL(triggered()), this, SIGNAL(signalFirstItem()));

    KAction *lastImageAction = new KAction(i18n("Last Image"), this);
    actionCollection()->addAction("last_image", lastImageAction);
    lastImageAction->setShortcut( QKeySequence(Qt::Key_End) );
    connect(lastImageAction, SIGNAL(triggered()), this, SIGNAL(signalLastItem()));

    KAction *copyItemsAction = new KAction(i18n("Copy Album Items Selection"), this);
    actionCollection()->addAction("copy_album_selection", copyItemsAction);
    copyItemsAction->setShortcut( QKeySequence(Qt::CTRL+Qt::Key_C) );
    connect(copyItemsAction, SIGNAL(triggered()), this, SIGNAL(signalCopyAlbumItemsSelection()));

    KAction *pasteItemsAction = new KAction(i18n("Paste Album Items Selection"), this);
    actionCollection()->addAction("paste_album_selection", pasteItemsAction);
    pasteItemsAction->setShortcut( QKeySequence(Qt::CTRL+Qt::Key_V) );
    connect(pasteItemsAction, SIGNAL(triggered()), this, SIGNAL(signalPasteAlbumItemsSelection()));
}

void DigikamApp::setupActions()
{
    d->solidCameraActionGroup = new QActionGroup(this);
    connect(d->solidCameraActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenSolidCamera(QAction*)));

    d->solidUsmActionGroup = new QActionGroup(this);
    connect(d->solidUsmActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenSolidUsmDevice(QAction*)));

    d->manualCameraActionGroup = new QActionGroup(this);
    connect(d->manualCameraActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenManualCamera(QAction*)));

    // -----------------------------------------------------------------

    d->themeMenuAction = new KSelectAction(i18n("&Themes"), this);
    connect(d->themeMenuAction, SIGNAL(triggered(const QString&)),
            this, SLOT(slotChangeTheme(const QString&)));
    actionCollection()->addAction("theme_menu", d->themeMenuAction);

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // -----------------------------------------------------------------

    d->backwardActionMenu = new KToolBarPopupAction(KIcon("go-previous"), i18n("&Back"), this);
    d->backwardActionMenu->setEnabled(false);
    d->backwardActionMenu->setShortcut(Qt::ALT+Qt::Key_Left);
    actionCollection()->addAction("album_back", d->backwardActionMenu);

    connect(d->backwardActionMenu->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowBackwardMenu()));

    // we are using a signal mapper to identify which of a bunch of actions was triggered
    d->backwardSignalMapper = new QSignalMapper(this);

    // connect mapper to view
    connect(d->backwardSignalMapper, SIGNAL(mapped(int)),
            d->view, SLOT(slotAlbumHistoryBack(int)));

    // connect action to mapper
    connect(d->backwardActionMenu, SIGNAL(triggered()), d->backwardSignalMapper, SLOT(map()));
    // inform mapper about number of steps
    d->backwardSignalMapper->setMapping(d->backwardActionMenu, 1);

    // -----------------------------------------------------------------

    d->forwardActionMenu = new KToolBarPopupAction(KIcon("go-next"), i18n("Forward"), this);
    d->forwardActionMenu->setEnabled(false);
    d->forwardActionMenu->setShortcut(Qt::ALT+Qt::Key_Right);
    actionCollection()->addAction("album_forward", d->forwardActionMenu);

    connect(d->forwardActionMenu->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowForwardMenu()));

    d->forwardSignalMapper = new QSignalMapper(this);

    connect(d->forwardSignalMapper, SIGNAL(mapped(int)),
            d->view, SLOT(slotAlbumHistoryForward(int)));

    connect(d->forwardActionMenu, SIGNAL(triggered()), d->forwardSignalMapper, SLOT(map()));
    d->forwardSignalMapper->setMapping(d->forwardActionMenu, 1);

    // -----------------------------------------------------------------

    d->newAction = new KAction(KIcon("albumfolder-new"), i18n("&New..."), this);
    d->newAction->setShortcut(KStandardShortcut::openNew());
    d->newAction->setWhatsThis(i18n("Creates a new empty Album in the collection."));
    connect(d->newAction, SIGNAL(triggered()), d->view, SLOT(slotNewAlbum()));
    actionCollection()->addAction("album_new", d->newAction);

    // -----------------------------------------------------------------

    d->deleteAction = new KAction(KIcon("user-trash"), i18n("Delete"), this);
    connect(d->deleteAction, SIGNAL(triggered()), d->view, SLOT(slotDeleteAlbum()));
    actionCollection()->addAction("album_delete", d->deleteAction);

    // -----------------------------------------------------------------

    d->propsEditAction = new KAction(KIcon("albumfolder-properties"), i18n("Properties..."), this);
    d->propsEditAction->setWhatsThis(i18n("Edit Album Properties and Collection information."));
    connect(d->propsEditAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumPropsEdit()));
    actionCollection()->addAction("album_propsEdit", d->propsEditAction);

    // -----------------------------------------------------------------

    d->refreshAlbumAction = new KAction(KIcon("view-refresh"), i18n("Refresh"), this);
    d->refreshAlbumAction->setShortcut(Qt::Key_F5);
    d->refreshAlbumAction->setWhatsThis(i18n("Refresh all album contents."));
    connect(d->refreshAlbumAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumRefresh()));
    actionCollection()->addAction("album_refresh", d->refreshAlbumAction);

    // -----------------------------------------------------------------

    d->syncAlbumMetadataAction = new KAction(KIcon("view-refresh"), i18n("Synchronize images with database"), this);
    d->syncAlbumMetadataAction->setWhatsThis(i18n("Updates all image metadata of the current "
                                                  "album with the contents of digiKam database "
                                                  "(image metadata will be over-written with data from "
                                                  "the database)."));
    connect(d->syncAlbumMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumSyncPicturesMetadata()));
    actionCollection()->addAction("album_syncmetadata", d->syncAlbumMetadataAction);

    // -----------------------------------------------------------------

    d->openInKonquiAction = new KAction(KIcon("konqueror"), i18n("Open in Konqueror"), this);
    connect(d->openInKonquiAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumOpenInKonqui()));
    actionCollection()->addAction("album_openinkonqui", d->openInKonquiAction);

    // -----------------------------------------------------------

    d->newTagAction = new KAction(KIcon("tag-new"), i18n("N&ew..."), this);
    connect(d->newTagAction, SIGNAL(triggered()), d->view, SLOT(slotNewTag()));
    actionCollection()->addAction("tag_new", d->newTagAction);

    // -----------------------------------------------------------

    d->editTagAction = new KAction(KIcon("tag-properties"), i18n("Properties..."), this);
    connect(d->editTagAction, SIGNAL(triggered()), d->view, SLOT(slotEditTag()));
    actionCollection()->addAction("tag_edit", d->editTagAction);

    // -----------------------------------------------------------

    d->deleteTagAction = new KAction(KIcon("tag-delete"), i18n("Delete"), this);
    connect(d->deleteTagAction, SIGNAL(triggered()), d->view, SLOT(slotDeleteTag()));
    actionCollection()->addAction("tag_delete", d->deleteTagAction);

    // -----------------------------------------------------------

    d->imagePreviewAction = new KToggleAction(KIcon("viewimage"), i18n("View..."), this);
    d->imagePreviewAction->setShortcut(Qt::Key_F3);
    connect(d->imagePreviewAction, SIGNAL(triggered()), d->view, SLOT(slotImagePreview()));
    actionCollection()->addAction("image_view", d->imagePreviewAction);

    // -----------------------------------------------------------

    d->imageViewAction = new KAction(KIcon("editimage"), i18n("Edit..."), this);
    d->imageViewAction->setShortcut(Qt::Key_F4);
    d->imageViewAction->setWhatsThis(i18n("Open the selected item in the image editor."));
    connect(d->imageViewAction, SIGNAL(triggered()), d->view, SLOT(slotImageEdit()));
    actionCollection()->addAction("image_edit", d->imageViewAction);

    // -----------------------------------------------------------

    d->imageLightTableAction = new KAction(KIcon("lighttable"), i18n("Place onto Light Table"), this);
    d->imageLightTableAction->setShortcut(Qt::CTRL+Qt::Key_L);
    d->imageLightTableAction->setWhatsThis(i18n("Place the selected items on the light table thumbbar."));
    connect(d->imageLightTableAction, SIGNAL(triggered()), d->view, SLOT(slotImageLightTable()));
    actionCollection()->addAction("image_lighttable", d->imageLightTableAction);

    // -----------------------------------------------------------

    d->imageAddLightTableAction = new KAction(KIcon("lighttableadd"), i18n("Add to Light Table"), this);
    d->imageAddLightTableAction->setShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_L);
    d->imageAddLightTableAction->setWhatsThis(i18n("Add selected items to the light table thumbbar."));
    connect(d->imageAddLightTableAction, SIGNAL(triggered()), d->view, SLOT(slotImageAddToLightTable()));
    actionCollection()->addAction("image_add_to_lighttable", d->imageAddLightTableAction);

    // -----------------------------------------------------------

    d->imageFindSimilarAction = new KAction(KIcon("tools-wizard"), i18n("Find Similar..."), this);
    d->imageFindSimilarAction->setWhatsThis(i18n("Find similar images using selected item as reference."));
    connect(d->imageFindSimilarAction, SIGNAL(triggered()), d->view, SLOT(slotImageFindSimilar()));
    actionCollection()->addAction("image_find_similar", d->imageFindSimilarAction);

    // -----------------------------------------------------------

    d->imageRenameAction = new KAction(KIcon("edit-rename"), i18n("Rename..."), this);
    d->imageRenameAction->setShortcut(Qt::Key_F2);
    d->imageRenameAction->setWhatsThis(i18n("Change the filename of the currently selected item."));
    connect(d->imageRenameAction, SIGNAL(triggered()), d->view, SLOT(slotImageRename()));
    actionCollection()->addAction("image_rename", d->imageRenameAction);

    // -----------------------------------------------------------

    // Pop up dialog to ask user whether to move to trash
    d->imageDeleteAction = new KAction(KIcon("user-trash"), i18n("Delete"), this);
    d->imageDeleteAction->setShortcut(Qt::Key_Delete);
    connect(d->imageDeleteAction, SIGNAL(triggered()), d->view, SLOT(slotImageDelete()));
    actionCollection()->addAction("image_delete", d->imageDeleteAction);

    // -----------------------------------------------------------

    // Pop up dialog to ask user whether to permanently delete
    d->imageDeletePermanentlyAction = new KAction(KIcon("edit-delete"), i18n("Delete permanently"), this);
    d->imageDeletePermanentlyAction->setShortcut(Qt::SHIFT+Qt::Key_Delete);
    connect(d->imageDeletePermanentlyAction, SIGNAL(triggered()), d->view, SLOT(slotImageDeletePermanently()));
    actionCollection()->addAction("image_delete_permanently", d->imageDeletePermanentlyAction);

    // -----------------------------------------------------------

    // These two actions are hidden, no menu entry, no toolbar entry, no shortcut.
    // Power users may add them.
    d->imageDeletePermanentlyDirectlyAction = new KAction(KIcon("edit-delete"),
                                              i18n("Delete permanently without confirmation"), this);
    connect(d->imageDeletePermanentlyDirectlyAction, SIGNAL(triggered()),
            d->view, SLOT(slotImageDeletePermanentlyDirectly()));
    actionCollection()->addAction("image_delete_permanently_directly", d->imageDeletePermanentlyDirectlyAction);

    // -----------------------------------------------------------

    d->imageTrashDirectlyAction = new KAction(KIcon("user-trash"),
                                  i18n("Move to trash without confirmation"), this);
    connect(d->imageTrashDirectlyAction, SIGNAL(triggered()),
            d->view, SLOT(slotImageTrashDirectly()));
    actionCollection()->addAction("image_trash_directly", d->imageTrashDirectlyAction);

    // -----------------------------------------------------------------

    d->albumSortAction = new KSelectAction(i18n("&Sort Albums"), this);
    d->albumSortAction->setWhatsThis(i18n("Sort Albums in tree-view."));
    connect(d->albumSortAction, SIGNAL(triggered(int)), d->view, SLOT(slotSortAlbums(int)));
    actionCollection()->addAction("album_sort", d->albumSortAction);

    // Use same list order as in albumsettings enum
    QStringList sortActionList;
    sortActionList.append(i18n("By Folder"));
    sortActionList.append(i18n("By Collection"));
    sortActionList.append(i18n("By Date"));
    d->albumSortAction->setItems(sortActionList);

    // -----------------------------------------------------------

    d->recurseAlbumsAction = new KToggleAction(i18n("Include Album Sub-Tree"), this);
    d->recurseAlbumsAction->setWhatsThis(i18n("Activate this option to recursively show all sub-albums below "
                                              "the current album."));
    connect(d->recurseAlbumsAction, SIGNAL(toggled(bool)), this, SLOT(slotRecurseAlbums(bool)));
    actionCollection()->addAction("albums_recursive", d->recurseAlbumsAction);

    d->recurseTagsAction = new KToggleAction(i18n("Include Tag Sub-Tree"), this);
    d->recurseTagsAction->setWhatsThis(i18n("Activate this option to show all images marked by the given tag "
                                            "and its all its sub-tags."));
    connect(d->recurseTagsAction, SIGNAL(toggled(bool)), this, SLOT(slotRecurseTags(bool)));
    actionCollection()->addAction("tags_recursive", d->recurseTagsAction);

    // -----------------------------------------------------------

    d->imageSortAction = new KSelectAction(i18n("&Sort Images"), this);
    d->imageSortAction->setWhatsThis(i18n("Sort Albums contents."));
    connect(d->imageSortAction, SIGNAL(triggered(int)), d->view, SLOT(slotSortImages(int)));
    actionCollection()->addAction("image_sort", d->imageSortAction);

    // Use same list order as in albumsettings enum
    QStringList sortImagesActionList;
    sortImagesActionList.append(i18n("By Name"));
    sortImagesActionList.append(i18n("By Path"));
    sortImagesActionList.append(i18n("By Date"));
    sortImagesActionList.append(i18n("By File Size"));
    sortImagesActionList.append(i18n("By Rating"));
    d->imageSortAction->setItems(sortImagesActionList);

    // -----------------------------------------------------------------

    QSignalMapper *exifOrientationMapper = new QSignalMapper(d->view);

    connect(exifOrientationMapper, SIGNAL(mapped(int)),
            d->view, SLOT(slotImageExifOrientation(int)));

    d->imageExifOrientationActionMenu = new KActionMenu(i18n("Adjust Exif orientation tag"), this);
    d->imageExifOrientationActionMenu->setDelayed(false);
    actionCollection()->addAction("image_set_exif_orientation", d->imageExifOrientationActionMenu);

    d->imageSetExifOrientation1Action =
        new KAction(i18n("Normal"), d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation2Action =
        new KAction(i18n("Flipped Horizontally"), d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation3Action =
        new KAction(i18n("Rotated upside down"), d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation4Action =
        new KAction(i18n("Flipped Vertically"), d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation5Action =
        new KAction(i18n("Rotated right / Horiz. Flipped"), d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation6Action =
        new KAction(i18n("Rotated right"), d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation7Action =
        new KAction(i18n("Rotated right / Vert. Flipped"), d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation8Action =
        new KAction(i18n("Rotated left"), d->imageExifOrientationActionMenu);

    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation1Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation2Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation3Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation4Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation5Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation6Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation7Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation8Action);

    connect(d->imageSetExifOrientation1Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation2Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation3Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation4Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation5Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation6Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation7Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation8Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    exifOrientationMapper->setMapping(d->imageSetExifOrientation1Action, 1);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation2Action, 2);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation3Action, 3);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation4Action, 4);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation5Action, 5);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation6Action, 6);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation7Action, 7);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation8Action, 8);

    // -----------------------------------------------------------------

    d->selectAllAction = new KAction(i18n("Select All"), this);
    d->selectAllAction->setShortcut(Qt::CTRL+Qt::Key_A);
    connect(d->selectAllAction, SIGNAL(triggered()), d->view, SLOT(slotSelectAll()));
    actionCollection()->addAction("selectAll", d->selectAllAction);

    // -----------------------------------------------------------------

    d->selectNoneAction = new KAction(i18n("Select None"), this);
    d->selectNoneAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_A);
    connect(d->selectNoneAction, SIGNAL(triggered()), d->view, SLOT(slotSelectNone()));
    actionCollection()->addAction("selectNone", d->selectNoneAction);

    // -----------------------------------------------------------------

    d->selectInvertAction = new KAction(i18n("Invert Selection"), this);
    d->selectInvertAction->setShortcut(Qt::CTRL+Qt::Key_Asterisk);
    connect(d->selectInvertAction, SIGNAL(triggered()), d->view, SLOT(slotSelectInvert()));
    actionCollection()->addAction("selectInvert", d->selectInvertAction);

    // -----------------------------------------------------------

    KStandardAction::keyBindings(this,       SLOT(slotEditKeys()),     actionCollection());
    KStandardAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStandardAction::preferences(this,       SLOT(slotSetup()),        actionCollection());

    // -----------------------------------------------------------

    d->zoomPlusAction = actionCollection()->addAction(KStandardAction::ZoomIn, "album_zoomin",
                                                      d->view, SLOT(slotZoomIn()));

    // -----------------------------------------------------------

    d->zoomMinusAction = actionCollection()->addAction(KStandardAction::ZoomOut, "album_zoomout",
                                                       d->view, SLOT(slotZoomOut()));

    // -----------------------------------------------------------

    d->zoomTo100percents = new KAction(KIcon("zoom-original"), i18n("Zoom to 1:1"), this);
    d->zoomTo100percents->setShortcut(Qt::ALT+Qt::CTRL+Qt::Key_0);       // NOTE: Photoshop 7 use ALT+CTRL+0
    connect(d->zoomTo100percents, SIGNAL(triggered()), d->view, SLOT(slotZoomTo100Percents()));
    actionCollection()->addAction("album_zoomto100percents", d->zoomTo100percents);

    // -----------------------------------------------------------

    d->zoomFitToWindowAction = new KAction(KIcon("zoom-fit-best"), i18n("Fit to &Window"), this);
    d->zoomFitToWindowAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_E); // NOTE: Gimp 2 use CTRL+SHIFT+E.
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), d->view, SLOT(slotFitToWindow()));
    actionCollection()->addAction("album_zoomfit2window", d->zoomFitToWindowAction);

    // -----------------------------------------------------------

    d->fullScreenAction = KStandardAction::fullScreen(this, SLOT(slotToggleFullScreen()), this, this);
    actionCollection()->addAction("full_screen", d->fullScreenAction);

    // -----------------------------------------------------------

    d->slideShowAction = new KActionMenu(KIcon("view-presentation"), i18n("Slide"), this);
    d->slideShowAction->setDelayed(false);
    actionCollection()->addAction("slideshow", d->slideShowAction);

    d->slideShowAllAction = new KAction(i18n("All"), this);
    d->slideShowAllAction->setShortcut(Qt::Key_F9);
    connect(d->slideShowAllAction, SIGNAL(triggered()), d->view, SLOT(slotSlideShowAll()));
    actionCollection()->addAction("slideshow_all", d->slideShowAllAction);
    d->slideShowAction->addAction(d->slideShowAllAction);

    d->slideShowSelectionAction = new KAction(i18n("Selection"), this);
    d->slideShowSelectionAction->setShortcut(Qt::ALT+Qt::Key_F9);
    connect(d->slideShowSelectionAction, SIGNAL(triggered()), d->view, SLOT(slotSlideShowSelection()));
    actionCollection()->addAction("slideshow_selected", d->slideShowSelectionAction);
    d->slideShowAction->addAction(d->slideShowSelectionAction);

    d->slideShowRecursiveAction = new KAction(i18n("With all sub-albums"), this);
    d->slideShowRecursiveAction->setShortcut(Qt::SHIFT+Qt::Key_F9);
    connect(d->slideShowRecursiveAction, SIGNAL(triggered()), d->view, SLOT(slotSlideShowRecursive()));
    actionCollection()->addAction("slideshow_recursive", d->slideShowRecursiveAction);
    d->slideShowAction->addAction(d->slideShowRecursiveAction);

    // -----------------------------------------------------------

    d->showBarAction = new KToggleAction(KIcon("view-choose"), i18n("Show Thumbnails"), this);
    d->showBarAction->setShortcut(Qt::CTRL+Qt::Key_T);
    connect(d->showBarAction, SIGNAL(triggered()), this, SLOT(slotToggleShowBar()));
    actionCollection()->addAction("showthumbs", d->showBarAction);

    // -----------------------------------------------------------

    d->quitAction = actionCollection()->addAction(KStandardAction::Quit, "app_exit",
                                                  this, SLOT(slotExit()));

    // -----------------------------------------------------------

    d->rawCameraListAction = new KAction(KIcon("kdcraw"), i18n("RAW camera supported"), this);
    connect(d->rawCameraListAction, SIGNAL(triggered()), this, SLOT(slotRawCameraList()));
    actionCollection()->addAction("help_rawcameralist", d->rawCameraListAction);

    // -----------------------------------------------------------

    d->libsInfoAction = new KAction(KIcon("help-about"), i18n("Components info"), this);
    connect(d->libsInfoAction, SIGNAL(triggered()), this, SLOT(slotComponentsInfo()));
    actionCollection()->addAction("help_librariesinfo", d->libsInfoAction);

    // -----------------------------------------------------------

    d->kipiHelpAction = new KAction(KIcon("kipi"), i18n("Kipi Plugins Handbook"), this);
    connect(d->kipiHelpAction, SIGNAL(triggered()), this, SLOT(slotShowKipiHelp()));
    actionCollection()->addAction("help_kipi", d->kipiHelpAction);

    // -----------------------------------------------------------

    d->tipAction = actionCollection()->addAction(KStandardAction::TipofDay, "help_tipofday",
                                                 this, SLOT(slotShowTip()));

    // -----------------------------------------------------------

    d->donateMoneyAction = new KAction(i18n("Donate..."), this);
    connect(d->donateMoneyAction, SIGNAL(triggered()), this, SLOT(slotDonateMoney()));
    actionCollection()->addAction("help_donatemoney", d->donateMoneyAction);

    d->contributeAction = new KAction(i18n("Contribute..."), this);
    connect(d->contributeAction, SIGNAL(triggered()), this, SLOT(slotContribute()));
    actionCollection()->addAction("help_contribute", d->contributeAction);

    // -- Logo on the right of tool bar --------------------------

    actionCollection()->addAction("logo_action", new DLogoAction(this));

    // -- Rating actions ---------------------------------------------------------------

    d->rating0Star = new KAction(i18n("Assign Rating \"No Stars\""), this);
    d->rating0Star->setShortcut(Qt::CTRL+Qt::Key_0);
    connect(d->rating0Star, SIGNAL(triggered()), d->view, SLOT(slotAssignRatingNoStar()));
    actionCollection()->addAction("ratenostar", d->rating0Star);

    d->rating1Star = new KAction(i18n("Assign Rating \"One Star\""), this);
    d->rating1Star->setShortcut(Qt::CTRL+Qt::Key_1);
    connect(d->rating1Star, SIGNAL(triggered()), d->view, SLOT(slotAssignRatingOneStar()));
    actionCollection()->addAction("rateonestar", d->rating1Star);

    d->rating2Star = new KAction(i18n("Assign Rating \"Two Stars\""), this);
    d->rating2Star->setShortcut(Qt::CTRL+Qt::Key_2);
    connect(d->rating2Star, SIGNAL(triggered()), d->view, SLOT(slotAssignRatingTwoStar()));
    actionCollection()->addAction("ratetwostar", d->rating2Star);

    d->rating3Star = new KAction(i18n("Assign Rating \"Three Stars\""), this);
    d->rating3Star->setShortcut(Qt::CTRL+Qt::Key_3);
    connect(d->rating3Star, SIGNAL(triggered()), d->view, SLOT(slotAssignRatingThreeStar()));
    actionCollection()->addAction("ratethreestar", d->rating3Star);

    d->rating4Star = new KAction(i18n("Assign Rating \"Four Stars\""), this);
    d->rating4Star->setShortcut(Qt::CTRL+Qt::Key_4);
    connect(d->rating4Star, SIGNAL(triggered()), d->view, SLOT(slotAssignRatingFourStar()));
    actionCollection()->addAction("ratefourstar", d->rating4Star);

    d->rating5Star = new KAction(i18n("Assign Rating \"Five Stars\""), this);
    d->rating5Star->setShortcut(Qt::CTRL+Qt::Key_5);
    connect(d->rating5Star, SIGNAL(triggered()), d->view, SLOT(slotAssignRatingFiveStar()));
    actionCollection()->addAction("ratefivestar", d->rating5Star);

    // -----------------------------------------------------------

    QAction* findAction = actionCollection()->addAction(KStandardAction::Find, "search_quick",
                                                        d->view, SLOT(slotNewKeywordSearch()));
    findAction->setText(i18n("Search..."));
    findAction->setIcon(BarIcon("system-search"));
    findAction->setShortcut(Qt::CTRL+Qt::Key_F);

    // -----------------------------------------------------------

    QAction* advFindAction = actionCollection()->addAction(KStandardAction::Find, "search_advanced",
                                                           d->view, SLOT(slotNewAdvancedSearch()));
    advFindAction->setText(i18n("Advanced Search..."));
    advFindAction->setShortcut(Qt::CTRL+Qt::ALT+Qt::Key_F);

    // -----------------------------------------------------------

    QAction* duplicatesAction = actionCollection()->addAction(KStandardAction::Find, "find_duplicates",
                                                              d->view, SLOT(slotNewDuplicatesSearch()));
    duplicatesAction->setText(i18n("Find Duplicates..."));
    duplicatesAction->setIcon(BarIcon("tools-wizard"));
    duplicatesAction->setShortcut(Qt::CTRL+Qt::Key_D);

    // -----------------------------------------------------------

    KAction *ltAction = new KAction(KIcon("lighttable"), i18n("Light Table"), this);
    ltAction->setShortcut(Qt::Key_L);
    connect(ltAction, SIGNAL(triggered()), d->view, SLOT(slotLightTable()));
    actionCollection()->addAction("light_table", ltAction);

    // -----------------------------------------------------------

    KAction *scanNewAction = new KAction(KIcon("view-refresh"), i18n("Scan for New Images"), this);
    connect(scanNewAction, SIGNAL(triggered()), this, SLOT(slotDatabaseRescan()));
    actionCollection()->addAction("database_rescan", scanNewAction);

    // -----------------------------------------------------------

    KAction *rebuildThumbsAction = new KAction(KIcon("run-build"), i18n("Rebuild all Thumbnails..."), this);
    connect(rebuildThumbsAction, SIGNAL(triggered()), this, SLOT(slotRebuildAllThumbs()));
    actionCollection()->addAction("thumbs_rebuild", rebuildThumbsAction);

    // -----------------------------------------------------------

    KAction *rebuildFingerPrintsAction = new KAction(KIcon("run-build"), i18n("Rebuild all FingerPrints..."), this);
    connect(rebuildFingerPrintsAction, SIGNAL(triggered()), this, SLOT(slotRebuildAllFingerPrints()));
    actionCollection()->addAction("fingerprints_rebuild", rebuildFingerPrintsAction);

    // -----------------------------------------------------------

    KAction *syncMetadataAction = new KAction(KIcon("run-build-file"),
                                              i18n("Update Metadata Database..."), this);
    connect(syncMetadataAction, SIGNAL(triggered()), this, SLOT(slotSyncAllPicturesMetadata()));
    actionCollection()->addAction("sync_metadata", syncMetadataAction);

    // -----------------------------------------------------------

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // Load Cameras -- do this before the createGUI so that the cameras
    // are plugged into the toolbar at startup
    if (d->splashScreen)
        d->splashScreen->message(i18n("Loading cameras"), Qt::AlignLeft, Qt::white);

    loadCameras();

    createGUI("digikamui.rc");

    // Initialize Actions ---------------------------------------

    d->deleteAction->setEnabled(false);
    d->addImagesAction->setEnabled(false);
    d->propsEditAction->setEnabled(false);
    d->openInKonquiAction->setEnabled(false);

    d->imageViewAction->setEnabled(false);
    d->imagePreviewAction->setEnabled(false);
    d->imageLightTableAction->setEnabled(false);
    d->imageAddLightTableAction->setEnabled(false);
    d->imageFindSimilarAction->setEnabled(false);
    d->imageRenameAction->setEnabled(false);
    d->imageDeleteAction->setEnabled(false);
    d->imageExifOrientationActionMenu->setEnabled(false);
    d->slideShowSelectionAction->setEnabled(false);

    d->albumSortAction->setCurrentItem((int)AlbumSettings::instance()->getAlbumSortOrder());
    d->imageSortAction->setCurrentItem((int)AlbumSettings::instance()->getImageSortOrder());
    d->recurseAlbumsAction->setChecked(AlbumSettings::instance()->getRecurseAlbums());
    d->recurseTagsAction->setChecked(AlbumSettings::instance()->getRecurseTags());
    d->showBarAction->setChecked(AlbumSettings::instance()->getShowThumbbar());

    // Setting the filter condition also updates the tooltip.
    // (So `setRating` is called first, as otherwise the filter value is not respected).
    d->albumIconViewFilter->readSettings();
}

void DigikamApp::enableZoomPlusAction(bool val)
{
    d->zoomPlusAction->setEnabled(val);
    d->statusZoomBar->setEnableZoomPlus(val);
}

void DigikamApp::enableZoomMinusAction(bool val)
{
    d->zoomMinusAction->setEnabled(val);
    d->statusZoomBar->setEnableZoomMinus(val);
}

void DigikamApp::enableAlbumBackwardHistory(bool enable)
{
    d->backwardActionMenu->setEnabled(enable);
}

void DigikamApp::enableAlbumForwardHistory(bool enable)
{
    d->forwardActionMenu->setEnabled(enable);
}

void DigikamApp::slotAboutToShowBackwardMenu()
{
    d->backwardActionMenu->menu()->clear();
    QStringList titles;
    d->view->getBackwardHistory(titles);

    for (int i=0; i<titles.size(); i++)
    {
        QAction *action =
            d->backwardActionMenu->menu()->addAction(titles[i], d->backwardSignalMapper, SLOT(map()));
        d->backwardSignalMapper->setMapping(action, i + 1);
    }
}

void DigikamApp::slotAboutToShowForwardMenu()
{
    d->forwardActionMenu->menu()->clear();
    QStringList titles;
    d->view->getForwardHistory(titles);

    for (int i=0; i<titles.size(); i++)
    {
        QAction *action =
            d->forwardActionMenu->menu()->addAction(titles[i], d->forwardSignalMapper, SLOT(map()));
        d->forwardSignalMapper->setMapping(action, i + 1);
    }
}

void DigikamApp::slotAlbumSelected(bool val)
{
    Album *album = AlbumManager::instance()->currentAlbum();

    if(album && !val)
    {
        // No PAlbum is selected
        d->deleteAction->setEnabled(false);
        d->addImagesAction->setEnabled(false);
        d->propsEditAction->setEnabled(false);
        d->openInKonquiAction->setEnabled(false);
        d->newAction->setEnabled(false);
        d->addFoldersAction->setEnabled(false);

    }
    else if(!album && !val)
    {
        // Groupitem selected (Collection/date)
        d->deleteAction->setEnabled(false);
        d->addImagesAction->setEnabled(false);
        d->propsEditAction->setEnabled(false);
        d->openInKonquiAction->setEnabled(false);
        d->newAction->setEnabled(false);
        d->addFoldersAction->setEnabled(false);

        foreach(QAction *action, d->kipiFileActionsExport)
        {
            action->setEnabled(false);
        }
    }
    else if(album && !album->isRoot() && album->type() == Album::PHYSICAL)
    {
        // Normal Album selected
        d->deleteAction->setEnabled(true);
        d->addImagesAction->setEnabled(true);
        d->propsEditAction->setEnabled(true);
        d->openInKonquiAction->setEnabled(true);
        d->newAction->setEnabled(true);
        d->addFoldersAction->setEnabled(true);

        foreach(QAction *action, d->kipiFileActionsExport)
        {
            action->setEnabled(true);
        }
    }
    else if(album && album->isRoot() && album->type() == Album::PHYSICAL)
    {
        // Root Album selected
        d->deleteAction->setEnabled(false);
        d->addImagesAction->setEnabled(false);
        d->propsEditAction->setEnabled(false);


        if(album->type() == Album::PHYSICAL)
        {
            d->newAction->setEnabled(true);
            d->openInKonquiAction->setEnabled(true);
            d->addFoldersAction->setEnabled(true);
        }
        else
        {
            d->newAction->setEnabled(false);
            d->openInKonquiAction->setEnabled(false);
            d->addFoldersAction->setEnabled(false);
        }

        foreach(QAction *action, d->kipiFileActionsExport)
        {
            action->setEnabled(true);
        }
    }
}

void DigikamApp::slotTagSelected(bool val)
{
    Album *album = AlbumManager::instance()->currentAlbum();
    if (!album) return;

    if(!val)
    {
        d->deleteTagAction->setEnabled(false);
        d->editTagAction->setEnabled(false);
    }
    else if(!album->isRoot())
    {
        d->deleteTagAction->setEnabled(true);
        d->editTagAction->setEnabled(true);

        foreach(QAction *action, d->kipiFileActionsExport)
        {
            action->setEnabled(true);
        }
    }
    else
    {
        d->deleteTagAction->setEnabled(false);
        d->editTagAction->setEnabled(false);

        foreach(QAction *action, d->kipiFileActionsExport)
        {
            action->setEnabled(true);
        }
    }
}

void DigikamApp::slotImageSelected(const ImageInfoList& list, bool hasPrev, bool hasNext,
                                   const KUrl::List& listAll)
{
    ImageInfoList selection = list;

    KUrl::List all = listAll;
    int num_images = listAll.count();
    bool val       = selection.isEmpty() ? false : true;
    int index      = 1;
    QString text;

    d->imageViewAction->setEnabled(val);
    d->imagePreviewAction->setEnabled(val);
    d->imageLightTableAction->setEnabled(val);
    d->imageAddLightTableAction->setEnabled(val);
    d->imageFindSimilarAction->setEnabled(val);
    d->imageRenameAction->setEnabled(val);
    d->imageDeleteAction->setEnabled(val);
    d->imageExifOrientationActionMenu->setEnabled(val);
    d->slideShowSelectionAction->setEnabled(selection.count() != 0);

    switch (selection.count())
    {
        case 0:
            d->statusProgressBar->setText(i18n("No item selected"));
            break;
        case 1:
        {
            KUrl first = selection.first().fileUrl();

            for (KUrl::List::iterator it = all.begin();
                it != all.end(); ++it)
            {
                if ((*it) == first)
                    break;

                index++;
            }

            text = selection.first().fileUrl().fileName()
                                   + i18n(" (%1 of %2)", QString::number(index),
                                                         QString::number(num_images));
            d->statusProgressBar->setText(text);
            break;
        }
        default:
            d->statusProgressBar->setText(i18n("%1/%2 items selected", selection.count(),
                                                                       QString::number(num_images)));
            break;
    }

    d->statusNavigateBar->setNavigateBarState(hasPrev, hasNext);
}

void DigikamApp::slotProgressBarMode(int mode, const QString& text)
{
    d->statusProgressBar->progressBarMode(mode, text);
}

void DigikamApp::slotProgressValue(int count)
{
    d->statusProgressBar->setProgressValue(count);
}

void DigikamApp::slotExit()
{
    close();
}

void DigikamApp::downloadImages( const QString& folder )
{
    if (!folder.isNull())
    {
        // activate window when called by media menu and DCOP
        if (isMinimized())
            KWindowSystem::unminimizeWindow(winId());
        KWindowSystem::activateWindow(winId());

        d->cameraGuiPath = folder;
        QTimer::singleShot(0, this, SLOT(slotOpenCameraUiFromPath()));
    }
}

void DigikamApp::cameraAutoDetect()
{
    // activate window when called by media menu and DCOP
    if (isMinimized())
        KWindowSystem::unminimizeWindow(winId());
    KWindowSystem::activateWindow(winId());

    slotCameraAutoDetect();
}

void DigikamApp::loadCameras()
{
    d->cameraSolidMenu->menuAction()->setText(i18n("Digital Cameras"));
    d->cameraSolidMenu->menuAction()->setIcon(KIcon("camera-photo"));
    d->usbMediaMenu->menuAction()->setText(i18n("USB Storage Devices"));
    d->usbMediaMenu->menuAction()->setIcon(KIcon("drive-removable-media-usb"));
    d->cardReaderMenu->menuAction()->setText(i18n("Card Readers"));
    d->cardReaderMenu->menuAction()->setIcon(KIcon("media-flash-smart-media"));
    d->manuallyAddedCamerasMenu->menuAction()->setText(i18n("Cameras Added Manually"));
    d->manuallyAddedCamerasMenu->menuAction()->setIcon(KIcon("preferences-other"));

    actionCollection()->addAction("camera_solid", d->cameraSolidMenu->menuAction());
    actionCollection()->addAction("usb_media", d->usbMediaMenu->menuAction());
    actionCollection()->addAction("card_reader", d->cardReaderMenu->menuAction());
    actionCollection()->addAction("camera_addedmanually", d->manuallyAddedCamerasMenu->menuAction());

    KAction *cameraAction = new KAction(i18n("Add Camera..."), this);
    connect(cameraAction, SIGNAL(triggered()), this, SLOT(slotSetupCamera()));
    actionCollection()->addAction("camera_add", cameraAction);
    d->addCameraSeparatorAction = d->manuallyAddedCamerasMenu->addSeparator();
    d->manuallyAddedCamerasMenu->addAction(cameraAction);

    // -----------------------------------------------------------------

    d->addImagesAction = new KAction(KIcon("albumfolder-importimages"), i18n("Add Images..."), this);
    d->addImagesAction->setShortcut(Qt::CTRL+Qt::Key_I);
    d->addImagesAction->setWhatsThis(i18n("Adds new items to an Album."));
    connect(d->addImagesAction, SIGNAL(triggered()), this, SLOT(slotImportAddImages()));
    actionCollection()->addAction("import_addImages", d->addImagesAction);

    // -----------------------------------------------------------------

    d->addFoldersAction = new KAction(KIcon("albumfolder-importdir"), i18n("Add Folders..."), this);
    d->addFoldersAction->setWhatsThis(i18n("Adds new folders to Albums library."));
    connect(d->addFoldersAction, SIGNAL(triggered()), this, SLOT(slotImportAddFolders()));
    actionCollection()->addAction("import_addFolders", d->addFoldersAction);

    // -- fill manually added cameras ----------------------------------

    d->cameraList->load();

    // -- scan Solid devices -------------------------------------------

    fillSolidMenus();

    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(const QString &)),
            this, SLOT(slotSolidDeviceChanged(const QString &)));

    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(const QString &)),
            this, SLOT(slotSolidDeviceChanged(const QString &)));
}

void DigikamApp::slotCameraAdded(CameraType *ctype)
{
    if (!ctype) return;

    KAction *cAction = new KAction(KIcon("camera-photo"), ctype->title(), d->manualCameraActionGroup);
    cAction->setData(ctype->title());
    actionCollection()->addAction(ctype->title().toUtf8(), cAction);

    // insert before separator
    d->manuallyAddedCamerasMenu->insertAction(d->addCameraSeparatorAction, cAction);
    ctype->setAction(cAction);
}

void DigikamApp::slotCameraRemoved(CameraType *ctype)
{
    if (!ctype) return;

    KAction *cAction = ctype->action();

    if (cAction)
        d->manuallyAddedCamerasMenu->removeAction(cAction);
}

void DigikamApp::slotCameraAutoDetect()
{
    bool retry = false;

    CameraType* ctype = d->cameraList->autoDetect(retry);

    if (!ctype && retry)
    {
        QTimer::singleShot(0, this, SLOT(slotCameraAutoDetect()));
        return;
    }

    if (ctype && ctype->action())
    {
        ctype->action()->activate(QAction::Trigger);
    }
}

void DigikamApp::slotOpenCameraUiFromPath()
{
    if (d->cameraGuiPath.isNull())
        return;

    // the CameraUI will delete itself when it has finished
    CameraUI* cgui = new CameraUI(this,
                                  i18n("Images found in %1", d->cameraGuiPath),
                                  "directory browse", "Fixed", d->cameraGuiPath,
                                  QDateTime::currentDateTime());
    cgui->show();

    connect(cgui, SIGNAL(signalLastDestination(const KUrl&)),
            d->view, SLOT(slotSelectAlbum(const KUrl&)));

    connect(cgui, SIGNAL(signalAlbumSettingsChanged()),
            this, SLOT(slotSetupChanged()));
}

void DigikamApp::slotOpenManualCamera(QAction *action)
{
    CameraType* ctype = d->cameraList->find(action->data().toString());

    if (ctype)
    {
        // check not to open two dialogs for the same camera
        if (ctype->currentCameraUI() && !ctype->currentCameraUI()->isClosed())
        {
            // show and raise dialog
            if (ctype->currentCameraUI()->isMinimized())
                KWindowSystem::unminimizeWindow(ctype->currentCameraUI()->winId());
            KWindowSystem::activateWindow(ctype->currentCameraUI()->winId());
        }
        else
        {
            // the CameraUI will delete itself when it has finished
            CameraUI* cgui = new CameraUI(this, ctype->title(), ctype->model(),
                                          ctype->port(), ctype->path(), ctype->lastAccess());

            ctype->setCurrentCameraUI(cgui);

            cgui->show();

            connect(cgui, SIGNAL(signalLastDestination(const KUrl&)),
                    d->view, SLOT(slotSelectAlbum(const KUrl&)));

            connect(cgui, SIGNAL(signalAlbumSettingsChanged()),
                    this, SLOT(slotSetupChanged()));
        }
    }
}

void DigikamApp::slotOpenSolidCamera(QAction *action)
{
    QString udi = action->data().toString();

    // if there is already an open CameraUI for the device, show and raise it, and be done
    if (d->cameraUIMap.contains(udi))
    {
        CameraUI *ui = d->cameraUIMap.value(udi);
        if (ui && !ui->isClosed())
        {
            if (ui->isMinimized())
                KWindowSystem::unminimizeWindow(ui->winId());
            KWindowSystem::activateWindow(ui->winId());
            return;
        }
    }

    // recreate device from unambiguous UDI
    Solid::Device device(udi);

    if( device.isValid() )
    {
        Solid::Camera *camera = device.as<Solid::Camera>();
        QList<QVariant> list = camera->driverHandle("gphoto").toList();
        // all sanity checks have already been done when creating the action
        if (list.size() < 3)
            return;

        int vendorId = list[1].toInt();
        int productId = list[2].toInt();

        QString model, port;
        if (CameraList::findConnectedCamera(vendorId, productId, model, port))
        {
            DDebug() << "Found camera from ids " << vendorId << " " << productId << " camera is: " << model << " at " << port << endl;
            // the CameraUI will delete itself when it has finished
            CameraUI* cgui = new CameraUI(this, action->text(), model,
                                          port, "/", QDateTime());

            d->cameraUIMap[udi] = cgui;

            cgui->show();

            connect(cgui, SIGNAL(signalLastDestination(const KUrl&)),
                    d->view, SLOT(slotSelectAlbum(const KUrl&)));

            connect(cgui, SIGNAL(signalAlbumSettingsChanged()),
                    this, SLOT(slotSetupChanged()));
        }
        else
        {
            DError() << "Failed to detect camera with GPhoto2 from Solid information" << endl;
        }
    }
}

void DigikamApp::slotOpenSolidUsmDevice(QAction *action)
{
    QString udi = action->data().toString();

    // if there is already an open CameraUI for the device, show and raise it
    if (d->cameraUIMap.contains(udi))
    {
        CameraUI *ui = d->cameraUIMap.value(udi);
        if (ui && !ui->isClosed())
        {
            if (ui->isMinimized())
                KWindowSystem::unminimizeWindow(ui->winId());
            KWindowSystem::activateWindow(ui->winId());
            return;
        }
    }

    // recreate device from unambiguous UDI
    Solid::Device device(udi);

    if( device.isValid() )
    {
        Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
        if (!access)
            return;

        if (!access->isAccessible())
        {
            QApplication::setOverrideCursor(Qt::WaitCursor);

            if (!access->setup())
                return;

            d->eventLoop = new QEventLoop(this);
            connect(access, SIGNAL(setupDone(Solid::ErrorType, QVariant)),
                    this, SLOT(slotSolidSetupDone(Solid::ErrorType, QVariant)));

            int returnCode = d->eventLoop->exec(QEventLoop::ExcludeUserInputEvents);

            delete d->eventLoop;
            d->eventLoop = 0;
            QApplication::restoreOverrideCursor();

            if (returnCode == 1)
            {
                KMessageBox::error(this, d->solidErrorMessage);
                return;
            }
        }

        // Create Camera UI

        QString path = access->filePath();

        // the CameraUI will delete itself when it has finished
        CameraUI* cgui = new CameraUI(this,
                                      i18n("Images found in %1", path),
                                           "directory browse", "Fixed", path,
                                           QDateTime::currentDateTime());
        d->cameraUIMap[udi] = cgui;

        cgui->show();

        connect(cgui, SIGNAL(signalLastDestination(const KUrl&)),
                d->view, SLOT(slotSelectAlbum(const KUrl&)));

        connect(cgui, SIGNAL(signalAlbumSettingsChanged()),
                this, SLOT(slotSetupChanged()));
    }
}

void DigikamApp::slotSolidSetupDone(Solid::ErrorType errorType, QVariant errorData)
{
    if (!d->eventLoop)
        return;

    if (errorType == Solid::NoError)
        d->eventLoop->exit(0);
    else
    {
        d->solidErrorMessage = i18n("Cannot access the storage device.\n");
        d->solidErrorMessage += errorData.toString();
        d->eventLoop->exit(1);
    }
}

void DigikamApp::slotSolidDeviceChanged(const QString &udi)
{
    Solid::Device device(udi);

    if (device.isValid())
    {
        if (device.is<Solid::StorageAccess>() || device.is<Solid::Camera>())
            fillSolidMenus();
    }
}

void DigikamApp::fillSolidMenus()
{
    d->cameraSolidMenu->clear();
    d->usbMediaMenu->clear();
    d->cardReaderMenu->clear();

    QList<Solid::Device> cameraDevices = Solid::Device::listFromType(Solid::DeviceInterface::Camera);

    foreach(const Solid::Device &cameraDevice, cameraDevices)
    {
        // USM camera: will be handled below
        if (cameraDevice.is<Solid::StorageAccess>())
            continue;

        const Solid::Camera *camera = cameraDevice.as<Solid::Camera>();

        QStringList drivers = camera->supportedDrivers();

        DDebug() << "fillSolidMenus: Found Camera " << cameraDevice.vendor() + " " + cameraDevice.product() << " protocols " << camera->supportedProtocols() << " drivers " << camera->supportedDrivers("ptp") << endl;

        // We handle gphoto2 cameras in this loop
        if (! (camera->supportedDrivers().contains("gphoto") || camera->supportedProtocols().contains("ptp")) )
            continue;

        QVariant driverHandle = camera->driverHandle("gphoto");
        if (!driverHandle.canConvert(QVariant::List))
        {
            DWarning() << "Solid returns unsupported driver handle for gphoto2" << endl;
            continue;
        }
        QList<QVariant> driverHandleList = driverHandle.toList();
        if (driverHandleList.size() < 3 || driverHandleList[0].toString() != "usb"
            || !driverHandleList[1].canConvert(QVariant::Int)
            || !driverHandleList[2].canConvert(QVariant::Int)
           )
        {
            DWarning() << "Solid returns unsupported driver handle for gphoto2" << endl;
            continue;
        }

        QString label = cameraDevice.vendor() + " " + cameraDevice.product();
        QString iconName = cameraDevice.icon();
        if (iconName.isEmpty())
            iconName = "camera-photo";

        KAction *action = new KAction(label, d->solidCameraActionGroup);

        action->setIcon(KIcon(iconName));
        // set data to identify device in action slot slotSolidSetupDevice
        action->setData(cameraDevice.udi());

        d->cameraSolidMenu->addAction(action);
    }

    QList<Solid::Device> storageDevices = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);

    foreach(const Solid::Device &accessDevice, storageDevices)
    {
        // check for StorageAccess
        if (!accessDevice.is<Solid::StorageAccess>())
            continue;

        // check for StorageDrive
        Solid::Device driveDevice;
        for (Solid::Device currentDevice = accessDevice; currentDevice.isValid(); currentDevice = currentDevice.parent())
        {
            if (currentDevice.is<Solid::StorageDrive>())
            {
                driveDevice = currentDevice;
                break;
            }
        }
        if (!driveDevice.isValid())
            continue;

        const Solid::StorageDrive *drive = driveDevice.as<Solid::StorageDrive>();
        QString driveType;
        bool isHarddisk = false;
        switch (drive->driveType())
        {
            // skip these
            case Solid::StorageDrive::CdromDrive:
            case Solid::StorageDrive::Floppy:
            case Solid::StorageDrive::Tape:
            default:
                continue;
            // accept card readers
            case Solid::StorageDrive::CompactFlash:
                driveType = i18n("CompactFlash Card Reader");
                break;
            case Solid::StorageDrive::MemoryStick:
                driveType = i18n("Memory Stick Reader");
                break;
            case Solid::StorageDrive::SmartMedia:
                driveType = i18n("SmartMedia Card Reader");
                break;
            case Solid::StorageDrive::SdMmc:
                driveType = i18n("SD / MMC Card Reader");
                break;
            case Solid::StorageDrive::Xd:
                driveType = i18n("xD-Picture Card Reader");
                break;
            case Solid::StorageDrive::HardDisk:
                // We dont want to list HardDisk partitions, but USB Mass Storage devices.
                // Dont know what is the exact difference between removable and hotpluggable.
                if (drive->isRemovable() || drive->isHotpluggable())
                {
                    isHarddisk = true;
                    if (drive->bus() == Solid::StorageDrive::Usb)
                        driveType = "USB Disk ";
                    else
                        driveType = "Disk ";
                    break;
                }
                else
                    continue;
        }

        // check for StorageVolume
        Solid::Device volumeDevice;
        for (Solid::Device currentDevice = accessDevice; currentDevice.isValid(); currentDevice = currentDevice.parent())
        {
            if (currentDevice.is<Solid::StorageVolume>())
            {
                volumeDevice = currentDevice;
                break;
            }
        }
        if (!volumeDevice.isValid())
            continue;

        bool isCamera = accessDevice.is<Solid::Camera>();

        const Solid::StorageAccess *access = accessDevice.as<Solid::StorageAccess>();
        const Solid::StorageVolume *volume = volumeDevice.as<Solid::StorageVolume>();

        QString label;

        if (isCamera)
        {
            label = accessDevice.vendor() + " " + accessDevice.product();
        }
        else
        {
            label += driveType;

            if (!driveDevice.product().isEmpty())
                label += "\"" + driveDevice.product() + "\" ";
            else if (!volume->label().isEmpty())
                label += "\"" + volume->label() + "\" ";

            if (!access->filePath().isEmpty())
                label += "at " + access->filePath() + " ";

            if (volume->size())
            {
                qulonglong size = volume->size();
                if (size > 1024)
                {
                    size /= 1024;
                    if (size > 1024)
                    {
                        size /= 1024;
                        if (size > 1024)
                        {
                            size /= 1024;
                            label += "(" + QString::number(size) + " TB)";
                        }
                        else
                            label += "(" + QString::number(size) + " MB)";
                    }
                    else
                        label += "(" + QString::number(size) + " KB)";
                }
                else
                    label += "(" + QString::number(size) + " Bytes)";
            }
        }

        QString iconName;
        if (!driveDevice.icon().isEmpty())
            iconName = driveDevice.icon();
        else if (!accessDevice.icon().isEmpty())
            iconName = accessDevice.icon();
        else if (!volumeDevice.icon().isEmpty())
            iconName = volumeDevice.icon();

        KAction *action = new KAction(label, d->solidUsmActionGroup);
        if (!iconName.isEmpty())
            action->setIcon(KIcon(iconName));

        // set data to identify device in action slot slotSolidSetupDevice
        action->setData(accessDevice.udi());

        if (isCamera)
            d->cameraSolidMenu->addAction(action);
        if (isHarddisk)
            d->usbMediaMenu->addAction(action);
        else
            d->cardReaderMenu->addAction(action);
    }

    //TODO: Find best usable solution when no devices are connected: One entry, hide, or disable?
    /*
    // Add one entry telling that no device is available
    if (d->cameraSolidMenu->isEmpty())
    {
        QAction *action = d->cameraSolidMenu->addAction(i18n("No Camera Connected"));
        action->setEnabled(false);
    }
    if (d->usbMediaMenu->isEmpty())
    {
        QAction *action = d->usbMediaMenu->addAction(i18n("No Storage Devices Found"));
        action->setEnabled(false);
    }
    if (d->cardReaderMenu->isEmpty())
    {
        QAction *action = d->cardReaderMenu->addAction(i18n("No Card Readers Available"));
        action->setEnabled(false);
    }
    */
    /*
    // hide empty menus
    d->cameraSolidMenu->menuAction()->setVisible(!d->cameraSolidMenu->isEmpty());
    d->usbMediaMenu->menuAction()->setVisible(!d->usbMediaMenu->isEmpty());
    d->cardReaderMenu->menuAction()->setVisible(!d->cardReaderMenu->isEmpty());
    */
    // disable empty menus
    d->cameraSolidMenu->menuAction()->setEnabled(!d->cameraSolidMenu->isEmpty());
    d->usbMediaMenu->menuAction()->setEnabled(!d->usbMediaMenu->isEmpty());
    d->cardReaderMenu->menuAction()->setEnabled(!d->cardReaderMenu->isEmpty());
}


void DigikamApp::slotSetup()
{
    setup();
}

bool DigikamApp::setup(bool iccSetupPage)
{
    Setup setup(this, 0, iccSetupPage ? Setup::ICCPage : Setup::LastPageUsed);

    // To show the number of KIPI plugins in the setup dialog.

    KIPI::PluginLoader::PluginList list = d->kipiPluginLoader->pluginList();
    setup.kipiPluginsPage()->initPlugins((int)list.count());

    if (setup.exec() != QDialog::Accepted)
        return false;

    setup.kipiPluginsPage()->applyPlugins();

    slotSetupChanged();

    return true;
}

void DigikamApp::slotSetupCamera()
{
    Setup setup(this, 0, Setup::CameraPage);

    // For to show the number of KIPI plugins in the setup dialog.

    KIPI::PluginLoader::PluginList list = d->kipiPluginLoader->pluginList();
    setup.kipiPluginsPage()->initPlugins((int)list.count());

    if (setup.exec() != QDialog::Accepted)
        return;

    setup.kipiPluginsPage()->applyPlugins();

    slotSetupChanged();
}

void DigikamApp::slotSetupChanged()
{
    // raw loading options might have changed
    LoadingCacheInterface::cleanCache();

    // TODO: clear history when location changed
    //if(AlbumSettings::instance()->getAlbumLibraryPath() != AlbumManager::instance()->getLibraryPath())
      //  d->view->clearHistory();

    if (AlbumManager::instance()->setDatabase(AlbumSettings::instance()->getDatabaseFilePath(), false))
        AlbumManager::instance()->startScan();

    if(AlbumSettings::instance()->getShowFolderTreeViewItemsCount())
        AlbumManager::instance()->refresh();

    d->view->applySettings();
    d->albumIconViewFilter->readSettings();

    AlbumThumbnailLoader::instance()->setThumbnailSize(AlbumSettings::instance()->getDefaultTreeIconSize());

    if (ImageWindow::imagewindowCreated())
        ImageWindow::imagewindow()->applySettings();

    if (LightTableWindow::lightTableWindowCreated())
        LightTableWindow::lightTableWindow()->applySettings();

    d->config->sync();
}

void DigikamApp::slotEditKeys()
{
    KShortcutsDialog dialog(KShortcutsEditor::AllActions,
                            KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection( actionCollection(), i18n( "General" ) );


    KIPI::PluginLoader::PluginList list = d->kipiPluginLoader->pluginList();

    for( KIPI::PluginLoader::PluginList::Iterator it = list.begin() ; it != list.end() ; ++it )
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if ( plugin )
           dialog.addCollection(plugin->actionCollection(), (*it)->comment());
    }

    dialog.configure();
}

void DigikamApp::slotConfToolbars()
{
    saveMainWindowSettings(d->config->group("General Settings"));
    KEditToolBar *dlg = new KEditToolBar(actionCollection(), this);
    dlg->setResourceFile("digikamui.rc");

    if(dlg->exec())
    {
        createGUI("digikamui.rc");
        applyMainWindowSettings(d->config->group("General Settings"));
        plugActionList( QString::fromLatin1("file_actions_import"), d->kipiFileActionsImport );
        plugActionList( QString::fromLatin1("image_actions"), d->kipiImageActions );
        plugActionList( QString::fromLatin1("tool_actions"), d->kipiToolsActions );
        plugActionList( QString::fromLatin1("batch_actions"), d->kipiBatchActions );
        plugActionList( QString::fromLatin1("album_actions"), d->kipiAlbumActions );
        plugActionList( QString::fromLatin1("file_actions_export"), d->kipiFileActionsExport );
    }

    delete dlg;
}

void DigikamApp::slotToggleFullScreen()
{
    if (d->fullScreen)
    {
        setWindowState( windowState() & ~Qt::WindowFullScreen ); // reset

        menuBar()->show();
        statusBar()->show();

        QList<KToolBar *> toolbars = toolBars();
        foreach(KToolBar *toolbar, toolbars)
        {
            toolbar->show();
        }

        d->view->showSideBars();

        d->fullScreen = false;
    }
    else
    {
        KConfigGroup group         = d->config->group("ImageViewer Settings");
        bool fullScreenHideToolBar = group.readEntry("FullScreen Hide ToolBar", false);

        setWindowState( windowState() | Qt::WindowFullScreen ); // set

        menuBar()->hide();
        statusBar()->hide();

        if (fullScreenHideToolBar)
        {
            QList<KToolBar *> toolbars = toolBars();
            foreach(KToolBar *toolbar, toolbars)
            {
                toolbar->hide();
            }
        }

        d->view->hideSideBars();

        d->fullScreen = true;
    }
}

void DigikamApp::slotShowTip()
{
    QStringList tipsFiles;
    tipsFiles.append("digikam/tips");
    tipsFiles.append("kipi/tips");
    KTipDialog::showMultiTip(this, tipsFiles, true);
}

void DigikamApp::slotShowKipiHelp()
{
    KToolInvocation::invokeHelp( QString(), "kipi-plugins" );
}

void DigikamApp::slotRawCameraList()
{
    RawCameraDlg dlg(this);
    dlg.exec();
}

void DigikamApp::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void DigikamApp::loadPlugins()
{
    if(d->splashScreen)
        d->splashScreen->message(i18n("Loading Kipi Plugins"), Qt::AlignLeft, Qt::white);

    QStringList ignores;
    d->kipiInterface = new KipiInterface( this, "Digikam_KIPI_interface" );

    ignores.append( "HelloWorld" );
    ignores.append( "KameraKlient" );

    d->kipiPluginLoader = new KIPI::PluginLoader( ignores, d->kipiInterface );

    connect( d->kipiPluginLoader, SIGNAL( replug() ),
             this, SLOT( slotKipiPluginPlug() ) );

    d->kipiPluginLoader->loadPlugins();

    d->kipiInterface->slotCurrentAlbumChanged(AlbumManager::instance()->currentAlbum());

    // Setting the initial menu options after all plugins have been loaded
    d->view->slotAlbumSelected(AlbumManager::instance()->currentAlbum());

    d->imagePluginsLoader = new ImagePluginLoader(this, d->splashScreen);
}

void DigikamApp::slotKipiPluginPlug()
{
    unplugActionList(QString::fromLatin1("file_actions_export"));
    unplugActionList(QString::fromLatin1("file_actions_import"));
    unplugActionList(QString::fromLatin1("image_actions"));
    unplugActionList(QString::fromLatin1("tool_actions"));
    unplugActionList(QString::fromLatin1("batch_actions"));
    unplugActionList(QString::fromLatin1("album_actions"));

    d->kipiImageActions.clear();
    d->kipiFileActionsExport.clear();
    d->kipiFileActionsImport.clear();
    d->kipiToolsActions.clear();
    d->kipiBatchActions.clear();
    d->kipiAlbumActions.clear();

    KIPI::PluginLoader::PluginList list = d->kipiPluginLoader->pluginList();

    int cpt = 0;

    for( KIPI::PluginLoader::PluginList::Iterator it = list.begin() ; it != list.end() ; ++it )
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if ( !plugin || !(*it)->shouldLoad() )
            continue;

        ++cpt;

        //if(d->splashScreen)
          //  d->splashScreen->message(i18n("Loading: %1", (*it)->name()));

        plugin->setup( this );

        // Plugin category identification using KAction method based.

        QList<KAction*> actions = plugin->actions();

        for(QList<KAction*>::Iterator it2 = actions.begin(); it2 != actions.end(); ++it2)
        {
            if (plugin->category(*it2) == KIPI::ImagesPlugin)
            {
                d->kipiImageActions.append(*it2);
            }
            else if (plugin->category(*it2) == KIPI::ExportPlugin)
            {
                d->kipiFileActionsExport.append(*it2);
            }
            else if (plugin->category(*it2) == KIPI::ImportPlugin)
            {
                d->kipiFileActionsImport.append(*it2);
            }
            else if (plugin->category(*it2) == KIPI::ToolsPlugin)
            {
                d->kipiToolsActions.append(*it2);
            }
            else if (plugin->category(*it2) == KIPI::BatchPlugin)
            {
                d->kipiBatchActions.append(*it2);
            }
            else if (plugin->category(*it2) == KIPI::CollectionsPlugin)
            {
                d->kipiAlbumActions.append(*it2);
            }
            else
                DDebug() << "No menu found for a plugin!!!" << endl;
        }
    }

    // Create GUI menu in according with plugins.

    plugActionList(QString::fromLatin1("file_actions_export"), d->kipiFileActionsExport);
    plugActionList(QString::fromLatin1("file_actions_import"), d->kipiFileActionsImport);
    plugActionList(QString::fromLatin1("image_actions"),       d->kipiImageActions);
    plugActionList(QString::fromLatin1("tool_actions"),        d->kipiToolsActions);
    plugActionList(QString::fromLatin1("batch_actions"),       d->kipiBatchActions);
    plugActionList(QString::fromLatin1("album_actions"),       d->kipiAlbumActions);
}

void DigikamApp::populateThemes()
{
    if(d->splashScreen)
        d->splashScreen->message(i18n("Loading themes"), Qt::AlignLeft, Qt::white);

    ThemeEngine::instance()->scanThemes();
    d->themeMenuAction->setItems(ThemeEngine::instance()->themeNames());
    slotThemeChanged();
    ThemeEngine::instance()->slotChangeTheme(d->themeMenuAction->currentText());
}

void DigikamApp::slotChangeTheme(const QString& theme)
{
    // Theme menu entry is returned with keyboard accelerator. We remove it.
    QString name = theme;
    name.remove(QChar('&'));
    AlbumSettings::instance()->setCurrentTheme(name);
    ThemeEngine::instance()->slotChangeTheme(name);
}

void DigikamApp::slotThemeChanged()
{
    QStringList themes(ThemeEngine::instance()->themeNames());
    int index = themes.indexOf(AlbumSettings::instance()->getCurrentTheme());
    if (index == -1)
        index = themes.indexOf(i18n("Default"));

    d->themeMenuAction->setCurrentItem(index);
}

void DigikamApp::slotDatabaseRescan()
{
    ScanController::instance()->completeCollectionScan();

    d->view->refreshView();

    if (ImageWindow::imagewindowCreated())
        ImageWindow::imagewindow()->refreshView();

    if (LightTableWindow::lightTableWindowCreated())
        LightTableWindow::lightTableWindow()->refreshView();
}

void DigikamApp::slotRebuildAllThumbs()
{
    QString msg = i18n("Rebuilding all image thumbnails can take some time.\n"
                       "Do you want to continue?");
    int result = KMessageBox::warningContinueCancel(this, msg);
    if (result != KMessageBox::Continue)
        return;

    BatchThumbsGenerator *thumbsGenerator = new BatchThumbsGenerator(this);

    connect(thumbsGenerator, SIGNAL(signalRebuildAllThumbsDone()),
            this, SLOT(slotRebuildAllThumbsDone()));

    thumbsGenerator->exec();
}

void DigikamApp::slotRebuildAllThumbsDone()
{
    d->view->applySettings();
}

void DigikamApp::slotSyncAllPicturesMetadata()
{
    QString msg = i18n("Updating the metadata database can take some time. \nDo you want to continue?");
    int result = KMessageBox::warningContinueCancel(this, msg);
    if (result != KMessageBox::Continue)
        return;

    BatchAlbumsSyncMetadata *syncMetadata = new BatchAlbumsSyncMetadata(this);

    connect(syncMetadata, SIGNAL(signalComplete()),
            this, SLOT(slotSyncAllPicturesMetadataDone()));

    syncMetadata->exec();
}

void DigikamApp::slotSyncAllPicturesMetadataDone()
{
    d->view->applySettings();
}

void DigikamApp::slotRebuildAllFingerPrints()
{
    QString msg = i18n("Image finger-prints computation can take a while.\n"
                       "What do you want to do?\n"
                       "- Scan for changed or non-recorded items in database\n"
                       "- Rebuild all fingerprints (long)");
    int result = KMessageBox::questionYesNoCancel(this, msg,
                                                  i18n("Warning"),
                                                  KGuiItem(i18n("Scan")),
                                                  KGuiItem(i18n("Rebuild All")));

    if (result == KMessageBox::Cancel)
        return;

    runFingerPrintsGenerator(result == KMessageBox::Yes ? false : true);
}

void DigikamApp::runFingerPrintsGenerator(bool rebuildAll)
{
    FingerPrintsGenerator *fingerprintsGenerator = new FingerPrintsGenerator(this, rebuildAll);

    connect(fingerprintsGenerator, SIGNAL(signalRebuildAllFingerPrintsDone()),
            this, SLOT(slotRebuildAllFingerPrintsDone()));

    fingerprintsGenerator->exec();
}

void DigikamApp::slotRebuildAllFingerPrintsDone()
{
    d->config->group("General Settings").writeEntry("Finger Prints Generator First Run", true);
}

void DigikamApp::slotDonateMoney()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=donation");
}

void DigikamApp::slotContribute()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=contrib");
}

void DigikamApp::slotRecurseAlbums(bool checked)
{
    AlbumLister::instance()->setRecurseAlbums(checked);
}

void DigikamApp::slotRecurseTags(bool checked)
{
    AlbumLister::instance()->setRecurseTags(checked);
}

void DigikamApp::slotZoomSliderChanged(int size)
{
    d->view->setThumbSize(size);
}

void DigikamApp::slotThumbSizeChanged(int size)
{
    d->statusZoomBar->setZoomSliderValue(size);
    d->statusZoomBar->setZoomTrackerText(i18n("Size: %1", size));
}

void DigikamApp::slotZoomChanged(double zoom, int size)
{
    d->statusZoomBar->setZoomSliderValue(size);
    d->statusZoomBar->setZoomTrackerText(i18n("zoom: %1%", (int)(zoom*100.0)));
}

void DigikamApp::slotTogglePreview(bool t)
{
    // NOTE: if 't' is true, we are in Preview Mode, else we are in AlbumView Mode

    // This is require if ESC is pressed to go out of Preview Mode.
    // imagePreviewAction is handled by F3 key only.
    d->imagePreviewAction->setChecked(t);

    // Here, we will toggle some menu actions depending of current Mode.

    // Select menu.
    d->selectAllAction->setEnabled(!t);
    d->selectNoneAction->setEnabled(!t);
    d->selectInvertAction->setEnabled(!t);

    // View menu
    d->albumSortAction->setEnabled(!t);
    d->imageSortAction->setEnabled(!t);
    d->zoomTo100percents->setEnabled(t);
    d->zoomFitToWindowAction->setEnabled(t);
    d->showBarAction->setEnabled(t);
}

void DigikamApp::slotImportAddImages()
{
    QString path = KFileDialog::getExistingDirectory(KGlobalSettings::documentPath(), this,
                                i18n("Select folder to parse"));

    if(path.isEmpty())
        return;

    // The folder contents will be parsed by Camera interface in "Directory Browse" mode.
    downloadFrom(path);
}

void DigikamApp::slotImportAddFolders()
{
    KFileDialog dlg(KUrl(), "inode/directory", this);
    dlg.setCaption(i18n("Select folders to import"));
    dlg.setMode(KFile::Directory |  KFile::Files);
    if(dlg.exec() != QDialog::Accepted)
        return;

    KUrl::List urls = dlg.selectedUrls();
    if(urls.empty()) return;

    Album *album = AlbumManager::instance()->currentAlbum();
    if (album && album->type() != Album::PHYSICAL)
        album = 0;

    QString header(i18n("<p>Please select the destination album from the digiKam library to "
                        "import folders.</p>"));

    album = AlbumSelectDialog::selectAlbum(this, (PAlbum*)album, header);
    if (!album) return;

    PAlbum *pAlbum = dynamic_cast<PAlbum*>(album);
    if (!pAlbum) return;

    KUrl albumUrl;
    albumUrl.setPath(pAlbum->folderPath());

    KIO::Job* job = DIO::copy(urls, albumUrl);

    connect(job, SIGNAL(result(KJob *)),
            this, SLOT(slotDIOResult(KJob *)));
}

void DigikamApp::slotDIOResult(KJob* kjob)
{
    KIO::Job *job = static_cast<KIO::Job*>(kjob);
    if (job->error())
    {
        job->ui()->setWindow(this);
        job->ui()->showErrorMessage();
    }
}

void DigikamApp::slotToggleShowBar()
{
    d->view->toggleShowBar(d->showBarAction->isChecked());
}

}  // namespace Digikam
