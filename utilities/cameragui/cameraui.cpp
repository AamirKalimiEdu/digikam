/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-16
 * Description : Camera interface
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <QMenu>
#include <QGroupBox>
#include <QPushButton>
#include <QToolButton>
#include <QSplitter>
#include <QPixmap>
#include <QComboBox>
#include <QToolBox>
#include <QFrame>
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QCloseEvent>
#include <QGridLayout>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>

// KDE includes.

#include <ktoolinvocation.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kimageio.h>
#include <kaboutdata.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kmenubar.h>
#include <kstandarddirs.h>
#include <khelpmenu.h>
#include <kcalendarsystem.h>
#include <kurllabel.h>
#include <kinputdialog.h>
#include <kvbox.h>
#include <kactioncollection.h>
#include <kstatusbar.h>
#include <kshortcutsdialog.h>
#include <kedittoolbar.h>
#include <ktoolbar.h>
#include <kio/global.h>

#include "config-digikam.h"
#ifdef HAVE_MARBLEWIDGET
#include <marble/global.h>
#endif // HAVE_MARBLEWIDGET

// libKipi includes.

#include <libkipi/plugin.h>
#include <libkipi/interface.h>

// Libkexiv2 includes.

#include <libkexiv2/kexiv2.h>

// Libkdcraw includes.

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/dcrawbinary.h>

// C Ansi includes.

extern "C"
{
#include <gphoto2-version.h>
#include <png.h>
#include <tiffvers.h>
#include <jpeglib.h>
}

// Local includes.

#include "ddebug.h"
#include "statuszoombar.h"
#include "statusprogressbar.h"
#include "statusnavigatebar.h"
#include "dlogoaction.h"
#include "thumbnailsize.h"
#include "kdatetimeedit.h"
#include "sidebar.h"
#include "themeengine.h"
#include "setup.h"
#include "downloadsettingscontainer.h"
#include "imagepropertiessidebarcamgui.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "album.h"
#include "albumselectdialog.h"
#include "renamecustomizer.h"
#include "animwidget.h"
#include "freespacewidget.h"
#include "collectionscanner.h"
#include "collectionmanager.h"
#include "greycstorationiface.h"
#include "rawcameradlg.h"
#include "libsinfodlg.h"
#include "capturedlg.h"
#include "camerafolderdialog.h"
#include "camerainfodialog.h"
#include "cameraiconview.h"
#include "cameraiconitem.h"
#include "cameracontroller.h"
#include "cameralist.h"
#include "cameratype.h"
#include "camerauiprivate.h"
#include "cameraui.h"
#include "cameraui.moc"

namespace Digikam
{

CameraUI::CameraUI(QWidget* parent, const QString& cameraTitle, 
                   const QString& model, const QString& port,
                   const QString& path, const QDateTime lastAccess)
        : KXmlGuiWindow(parent)
{
    d = new CameraUIPriv;
    d->lastAccess  = lastAccess;
    d->cameraTitle = cameraTitle;
    setCaption(cameraTitle);

    // -------------------------------------------------------------------

    setupUserArea();
    setupStatusBar();
    setupActions();
    setupAccelerators();

    // -- Make signals/slots connections ---------------------------------

    setupConnections();

    // -- Read settings --------------------------------------------------

    readSettings();
    setAutoSaveSettings("Camera Settings", true);


    // -- Init. camera controller ----------------------------------------

    setupCameraController(model, port, path);

    d->view->setFocus();
    QTimer::singleShot(0, d->controller, SLOT(slotConnect()));
}

CameraUI::~CameraUI()
{
    delete d->rightSidebar;
    delete d->controller;
    delete d;
}

void CameraUI::setupUserArea()
{
    KHBox* widget   = new KHBox(this);
    d->splitter     = new QSplitter(widget);
    d->view         = new CameraIconView(this, d->splitter);
    d->rightSidebar = new ImagePropertiesSideBarCamGui(widget, d->splitter, KMultiTabBar::Right, true);
    d->rightSidebar->setObjectName("CameraGui Sidebar Right");
    d->splitter->setFrameStyle( QFrame::NoFrame );
    d->splitter->setFrameShadow( QFrame::Plain );
    d->splitter->setFrameShape( QFrame::NoFrame );
    d->splitter->setOpaqueResize(false);
    d->splitter->setStretchFactor(0, 10);      // set iconview default size to max.

    // -------------------------------------------------------------------------

    d->advBox            = new QToolBox(d->rightSidebar);
    d->renameCustomizer  = new RenameCustomizer(d->advBox, d->cameraTitle);
    d->view->setRenameCustomizer(d->renameCustomizer);

    d->advBox->setWhatsThis( i18n("<p>Set how digiKam will rename files as they are downloaded."));

    d->advBox->insertItem(CameraUIPriv::RENAMEFILEPAGE, d->renameCustomizer, 
                          SmallIcon("insert-image"), i18n("File Renaming Options"));

    // -- Albums Auto-creation options -----------------------------------------

    QWidget* albumBox      = new QWidget(d->advBox);
    QVBoxLayout* albumVlay = new QVBoxLayout(albumBox);
    d->autoAlbumExtCheck   = new QCheckBox(i18n("Extension-based sub-albums"), albumBox);
    d->autoAlbumDateCheck  = new QCheckBox(i18n("Date-based sub-albums"), albumBox);
    KHBox *hbox1           = new KHBox(albumBox);
    d->folderDateLabel     = new QLabel(i18n("Date format:"), hbox1);
    d->folderDateFormat    = new QComboBox(hbox1);
    d->folderDateFormat->insertItem(CameraUIPriv::IsoDateFormat,   i18n("ISO"));
    d->folderDateFormat->insertItem(CameraUIPriv::TextDateFormat,  i18n("Full Text"));
    d->folderDateFormat->insertItem(CameraUIPriv::LocalDateFormat, i18n("Local Settings"));

    albumVlay->addWidget(d->autoAlbumExtCheck);
    albumVlay->addWidget(d->autoAlbumDateCheck);
    albumVlay->addWidget(hbox1);
    albumVlay->addStretch();
    albumVlay->setMargin(KDialog::spacingHint());
    albumVlay->setSpacing(KDialog::spacingHint());

    albumBox->setWhatsThis( i18n("<p>Set how digiKam creates albums automatically when downloading."));
    d->autoAlbumExtCheck->setWhatsThis( i18n("<p>Enable this option if you want to download your "
                     "pictures into automatically created file extension-based sub-albums of the destination "
                     "album. This way, you can separate JPEG and RAW files as they are downloaded from your camera."));
    d->autoAlbumDateCheck->setWhatsThis( i18n("<p>Enable this option if you want to "
                     "download your pictures into automatically created file date-based sub-albums "
                     "of the destination album."));
    d->folderDateFormat->setWhatsThis( i18n("<p>Select your preferred date format used to "
                     "create new albums. The options available are:<p>"
                     "<b>ISO</b>: the date format is in accordance with ISO 8601 "
                     "(YYYY-MM-DD). E.g.: <i>2006-08-24</i><p>"
                     "<b>Full Text</b>: the date format is in a user-readable string. "
                     "E.g.: <i>Thu Aug 24 2006</i><p>"
                     "<b>Local Settings</b>: the date format depending on KDE control panel settings.<p>"));

    d->advBox->insertItem(CameraUIPriv::AUTOALBUMPAGE, albumBox, SmallIcon("folder-new"), 
                          i18n("Auto-creation of Albums"));

    // -- On the Fly options ---------------------------------------------------

    QWidget* onFlyBox      = new QWidget(d->advBox);
    QVBoxLayout* onFlyVlay = new QVBoxLayout(onFlyBox);
    d->setPhotographerId   = new QCheckBox(i18n("Set default photographer identity"), onFlyBox);
    d->setCredits          = new QCheckBox(i18n("Set default credit and copyright"), onFlyBox);
    d->fixDateTimeCheck    = new QCheckBox(i18n("Fix internal date && time"), onFlyBox);
    d->dateTimeEdit        = new KDateTimeEdit(onFlyBox, "datepicker");
    d->autoRotateCheck     = new QCheckBox(i18n("Auto-rotate/flip image"), onFlyBox);
    d->convertJpegCheck    = new QCheckBox(i18n("Convert to lossless file format"), onFlyBox);
    KHBox *hbox2           = new KHBox(onFlyBox);
    d->formatLabel         = new QLabel(i18n("New image format:"), hbox2);
    d->losslessFormat      = new QComboBox(hbox2);
    d->losslessFormat->insertItem(0, "PNG");

    onFlyVlay->addWidget(d->setPhotographerId);
    onFlyVlay->addWidget(d->setCredits);
    onFlyVlay->addWidget(d->fixDateTimeCheck);
    onFlyVlay->addWidget(d->dateTimeEdit);
    onFlyVlay->addWidget(d->autoRotateCheck);
    onFlyVlay->addWidget(d->convertJpegCheck);
    onFlyVlay->addWidget(hbox2);
    onFlyVlay->addStretch();
    onFlyVlay->setMargin(KDialog::spacingHint());
    onFlyVlay->setSpacing(KDialog::spacingHint());

    onFlyBox->setWhatsThis( i18n("<p>Set here all options to fix/transform JPEG files automatically "
                     "as they are downloaded."));
    d->autoRotateCheck->setWhatsThis( i18n("<p>Enable this option if you want images automatically "
                     "rotated or flipped using EXIF information provided by the camera."));
    d->setPhotographerId->setWhatsThis( i18n("<p>Enable this option to store the default "
                     "photographer identity in the XMP and IPTC tags using digiKam's metadata settings."));
    d->setCredits->setWhatsThis( i18n("<p>Enable this option to store the default credit "
                     "and copyright information in the XMP and IPTC tags using digiKam's metadata settings."));
    d->fixDateTimeCheck->setWhatsThis( i18n("<p>Enable this option to set date and time metadata "
                     "tags to the right values if your camera does not set "
                     "these tags correctly when pictures are taken. The values will "
                     "be saved in the DateTimeDigitized and DateTimeCreated EXIF, XMP, and IPTC tags."));
    d->convertJpegCheck->setWhatsThis( i18n("<p>Enable this option to automatically convert "
                     "all JPEG files to a lossless image format. <b>Note:</b> Image conversion can take a "
                     "while on a slow computer."));
    d->losslessFormat->setWhatsThis( i18n("<p>Select your preferred lossless image file format to "
                     "convert to.  <b>Note:</b> All metadata will be preserved during the conversion."));

    d->advBox->insertItem(CameraUIPriv::ONFLYPAGE, onFlyBox, SmallIcon("system-run"), 
                          i18n("On the Fly Operations (JPEG only)"));

    d->rightSidebar->appendTab(d->advBox, SmallIcon("configure"), i18n("Settings"));
    d->rightSidebar->loadViewState();

    // -------------------------------------------------------------------------

    setCentralWidget(widget);
}

void CameraUI::setupActions()
{
    // -- File menu ----------------------------------------------------

    d->cameraCancelAction = new KAction(KIcon("process-stop"), i18n("Cancel"), this);
    connect(d->cameraCancelAction, SIGNAL(triggered()), this, SLOT(slotCancelButton()));
    actionCollection()->addAction("cameraui_cancelprocess", d->cameraCancelAction);
    d->cameraCancelAction->setEnabled(false);

    // -----------------------------------------------------------------

    d->cameraInfoAction = new KAction(KIcon("camera-photo"), i18n("Information"), this);
    connect(d->cameraInfoAction, SIGNAL(triggered()), this, SLOT(slotInformations()));
    actionCollection()->addAction("cameraui_info", d->cameraInfoAction);

    // -----------------------------------------------------------------

    d->cameraCaptureAction = new KAction(KIcon("camera"), i18n("Capture"), this);
    connect(d->cameraCaptureAction, SIGNAL(triggered()), this, SLOT(slotCapture()));
    actionCollection()->addAction("cameraui_capture", d->cameraCaptureAction);

    // -----------------------------------------------------------------

    actionCollection()->addAction(KStandardAction::Close, "cameraui_close", 
                                  this, SLOT(close()));

    // -- Edit menu ----------------------------------------------------

    d->selectAllAction = new KAction(i18n("Select All"), this);
    d->selectAllAction->setShortcut(Qt::CTRL+Qt::Key_A);
    connect(d->selectAllAction, SIGNAL(triggered()), d->view, SLOT(slotSelectAll()));
    actionCollection()->addAction("cameraui_selectall", d->selectAllAction);

    // -----------------------------------------------------------------

    d->selectNoneAction = new KAction(i18n("Select None"), this);
    d->selectNoneAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_A);
    connect(d->selectNoneAction, SIGNAL(triggered()), d->view, SLOT(slotSelectNone()));
    actionCollection()->addAction("cameraui_selectnone", d->selectNoneAction);

    // -----------------------------------------------------------------

    d->selectInvertAction = new KAction(i18n("Invert Selection"), this);
    d->selectInvertAction->setShortcut(Qt::CTRL+Qt::Key_Asterisk);
    connect(d->selectInvertAction, SIGNAL(triggered()), d->view, SLOT(slotSelectInvert()));
    actionCollection()->addAction("cameraui_selectinvert", d->selectInvertAction);

    // -----------------------------------------------------------

    d->selectNewItemsAction = new KAction(KIcon("document-new"), i18n("Select New Items"), this);
    connect(d->selectNewItemsAction, SIGNAL(triggered()), d->view, SLOT(slotSelectInvert()));
    actionCollection()->addAction("cameraui_selectnewitems", d->selectNewItemsAction);

    // -- Image menu ---------------------------------------------

    d->imageViewAction = new KAction(KIcon("editimage"), i18n("View"), this);
    connect(d->imageViewAction, SIGNAL(triggered()), this, SLOT(slotFileView()));
    actionCollection()->addAction("cameraui_imageview", d->imageViewAction);
    d->imageViewAction->setEnabled(false);

    // -----------------------------------------------------------------

    d->downloadSelectedAction = new KAction(KIcon("computer"), i18n("Download Selected"), this);
    connect(d->downloadSelectedAction, SIGNAL(triggered()), this, SLOT(slotDownloadSelected()));
    actionCollection()->addAction("cameraui_imagedownloadselected", d->downloadSelectedAction);
    d->downloadSelectedAction->setEnabled(false);

    // -----------------------------------------------------------------

    d->downloadAllAction = new KAction(i18n("Download All"), this);
    connect(d->downloadAllAction, SIGNAL(triggered()), this, SLOT(slotDownloadAll()));
    actionCollection()->addAction("cameraui_imagedownloadall", d->downloadAllAction);

    // -----------------------------------------------------------------

    d->downloadDelSelectedAction = new KAction(i18n("Download/Delete Selected"), this);
    connect(d->downloadDelSelectedAction, SIGNAL(triggered()), this, SLOT(slotDownloadAndDeleteSelected()));
    actionCollection()->addAction("cameraui_imagedownloaddeleteselected", d->downloadDelSelectedAction);
    d->downloadDelSelectedAction->setEnabled(false);

    // -------------------------------------------------------------------------

    d->downloadDelAllAction = new KAction(i18n("Download/Delete All"), this);
    connect(d->downloadDelAllAction, SIGNAL(triggered()), this, SLOT(slotDownloadAndDeleteAll()));
    actionCollection()->addAction("cameraui_imagedownloaddeleteall", d->downloadDelAllAction);

    // -------------------------------------------------------------------------

    d->uploadAction = new KAction(KIcon("media-flash-smart-media"), i18n("Upload..."), this);
    connect(d->uploadAction, SIGNAL(triggered()), this, SLOT(slotUpload()));
    actionCollection()->addAction("cameraui_imageupload", d->uploadAction);

    // -------------------------------------------------------------------------

    d->lockAction = new KAction(KIcon("document-decrypt"), i18n("Toggle Lock"), this);
    connect(d->lockAction, SIGNAL(triggered()), this, SLOT(slotToggleLock()));
    actionCollection()->addAction("cameraui_imagelock", d->lockAction);

    // -------------------------------------------------------------------------

    d->deleteSelectedAction = new KAction(KIcon("edit-delete"), i18n("Delete Selected"), this);
    connect(d->deleteSelectedAction, SIGNAL(triggered()), this, SLOT(slotDeleteSelected()));
    actionCollection()->addAction("cameraui_imagedeleteselected", d->deleteSelectedAction);
    d->deleteSelectedAction->setEnabled(false);

    // -------------------------------------------------------------------------

    d->deleteAllAction = new KAction(i18n("Delete All"), this);
    connect(d->deleteAllAction, SIGNAL(triggered()), this, SLOT(slotDeleteAll()));
    actionCollection()->addAction("cameraui_imagedeleteall", d->deleteAllAction);

    // -- Standard 'View' menu actions ---------------------------------------------

    d->increaseThumbsAction = actionCollection()->addAction(KStandardAction::ZoomIn, "cameraui_zoomplus", 
                                                            this, SLOT(slotIncreaseThumbSize()));
    d->increaseThumbsAction->setEnabled(false);

    d->decreaseThumbsAction = actionCollection()->addAction(KStandardAction::ZoomOut, "cameraui_zoomminus", 
                                                            this, SLOT(slotDecreaseThumbSize()));
    d->decreaseThumbsAction->setEnabled(false);

    d->fullScreenAction = actionCollection()->addAction(KStandardAction::FullScreen,
                          "cameraui_fullscreen", this, SLOT(slotToggleFullScreen()));

    // -- Standard 'Configure' menu actions ----------------------------------------

    KStandardAction::keyBindings(this, SLOT(slotEditKeys()),           actionCollection());
    KStandardAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStandardAction::preferences(this, SLOT(slotSetup()),              actionCollection());

    // ---------------------------------------------------------------------------------

    d->themeMenuAction = new KSelectAction(i18n("&Themes"), this);
    connect(d->themeMenuAction, SIGNAL(triggered(const QString&)), 
            this, SLOT(slotChangeTheme(const QString&)));
    actionCollection()->addAction("theme_menu", d->themeMenuAction);

    d->themeMenuAction->setItems(ThemeEngine::instance()->themeNames());
    slotThemeChanged();

    // -- Standard 'Help' menu actions ---------------------------------------------

    d->donateMoneyAction = new KAction(i18n("Donate..."), this);
    connect(d->donateMoneyAction, SIGNAL(triggered()), this, SLOT(slotDonateMoney()));
    actionCollection()->addAction("cameraui_donatemoney", d->donateMoneyAction);

    d->contributeAction = new KAction(i18n("Contribute..."), this);
    connect(d->contributeAction, SIGNAL(triggered()), this, SLOT(slotContribute()));
    actionCollection()->addAction("cameraui_contribute", d->contributeAction);

    d->rawCameraListAction = new KAction(KIcon("kdcraw"), i18n("RAW camera supported"), this);
    connect(d->rawCameraListAction, SIGNAL(triggered()), this, SLOT(slotRawCameraList()));
    actionCollection()->addAction("cameraui_rawcameralist", d->rawCameraListAction);

    d->libsInfoAction = new KAction(KIcon("help-about"), i18n("Components info"), this);
    connect(d->libsInfoAction, SIGNAL(triggered()), this, SLOT(slotComponentsInfo()));
    actionCollection()->addAction("cameraui_librariesinfo", d->libsInfoAction);

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // -- Keyboard-only actions added to <MainWindow> ------------------------------

    KAction *exitFullscreenAction = new KAction(i18n("Exit Fullscreen mode"), this);
    actionCollection()->addAction("cameraui_exitfullscreen", exitFullscreenAction);
    exitFullscreenAction->setShortcut( QKeySequence(Qt::Key_Escape) );
    connect(exitFullscreenAction, SIGNAL(triggered()), this, SLOT(slotEscapePressed()));

    KAction *altBackwardAction = new KAction(i18n("Previous Image"), this);
    actionCollection()->addAction("cameraui_backward_shift_space", altBackwardAction);
    altBackwardAction->setShortcut( KShortcut(Qt::SHIFT+Qt::Key_Space) );
    connect(altBackwardAction, SIGNAL(triggered()), this, SLOT(slotPrevItem()));

    // ---------------------------------------------------------------------------------

    DLogoAction *logoAction = new DLogoAction(this);
    actionCollection()->addAction("logo_action", logoAction);

    d->anim                   = new AnimWidget(this, 22);
    QWidgetAction *animAction = new QWidgetAction(this);
    animAction->setDefaultWidget(d->anim);
    actionCollection()->addAction("anim_action", animAction);

    createGUI("cameraui.rc");
}

void CameraUI::setupConnections()
{
    connect(d->autoAlbumDateCheck, SIGNAL(toggled(bool)),
            d->folderDateFormat, SLOT(setEnabled(bool)));

    connect(d->autoAlbumDateCheck, SIGNAL(toggled(bool)),
            d->folderDateLabel, SLOT(setEnabled(bool)));

    connect(d->convertJpegCheck, SIGNAL(toggled(bool)),
            d->losslessFormat, SLOT(setEnabled(bool)));

    connect(d->convertJpegCheck, SIGNAL(toggled(bool)),
            d->formatLabel, SLOT(setEnabled(bool)));

    connect(d->convertJpegCheck, SIGNAL(toggled(bool)),
            d->view, SLOT(slotDownloadNameChanged()));

    connect(d->fixDateTimeCheck, SIGNAL(toggled(bool)),
            d->dateTimeEdit, SLOT(setEnabled(bool)));

    // -------------------------------------------------------------------------

    connect(d->view, SIGNAL(signalSelected(CameraIconViewItem*, bool)),
            this, SLOT(slotItemsSelected(CameraIconViewItem*, bool)));

    connect(d->view, SIGNAL(signalFileView(CameraIconViewItem*)),
            this, SLOT(slotFileView(CameraIconViewItem*)));

    connect(d->view, SIGNAL(signalUpload(const KUrl::List&)),
            this, SLOT(slotUploadItems(const KUrl::List&)));

    connect(d->view, SIGNAL(signalDownload()),
            this, SLOT(slotDownloadSelected()));

    connect(d->view, SIGNAL(signalDownloadAndDelete()),
            this, SLOT(slotDownloadAndDeleteSelected()));

    connect(d->view, SIGNAL(signalDelete()),
            this, SLOT(slotDeleteSelected()));

    connect(d->view, SIGNAL(signalToggleLock()),
            this, SLOT(slotToggleLock()));

    connect(d->view, SIGNAL(signalNewSelection(bool)),
            this, SLOT(slotNewSelection(bool)));

    connect(d->view, SIGNAL(signalThumbSizeChanged(int)),
            this, SLOT(slotThumbSizeChanged(int)));

    // -------------------------------------------------------------------------

    connect(d->statusNavigateBar, SIGNAL(signalFirstItem()),
            this, SLOT(slotFirstItem()));

    connect(d->statusNavigateBar, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->statusNavigateBar, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->statusNavigateBar, SIGNAL(signalLastItem()),
            this, SLOT(slotLastItem()));

    // -------------------------------------------------------------------------

    connect(d->statusZoomBar, SIGNAL(signalZoomMinusClicked()),
           this, SLOT(slotDecreaseThumbSize()));

    connect(d->statusZoomBar, SIGNAL(signalZoomPlusClicked()),
           this, SLOT(slotIncreaseThumbSize()));

    connect(d->statusZoomBar, SIGNAL(signalZoomSliderChanged(int)),
           this, SLOT(slotZoomSliderChanged(int)));
}

void CameraUI::setupStatusBar()
{
    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    d->statusProgressBar->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addWidget(d->statusProgressBar, 100);

    //------------------------------------------------------------------------------

    d->albumLibraryFreeSpace = new FreeSpaceWidget(statusBar(), 100);
    d->albumLibraryFreeSpace->setMode(FreeSpaceWidget::AlbumLibrary);
    d->albumLibraryFreeSpace->setPath(AlbumSettings::instance()->getAlbumLibraryPath());
    statusBar()->addWidget(d->albumLibraryFreeSpace, 1);

    //------------------------------------------------------------------------------

    d->cameraFreeSpace = new FreeSpaceWidget(statusBar(), 100);
    statusBar()->addWidget(d->cameraFreeSpace, 1);

    //------------------------------------------------------------------------------

    d->statusZoomBar = new StatusZoomBar(statusBar());
    d->statusZoomBar->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addPermanentWidget(d->statusZoomBar, 1);

    //------------------------------------------------------------------------------

    d->statusNavigateBar = new StatusNavigateBar(statusBar());
    d->statusNavigateBar->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addPermanentWidget(d->statusNavigateBar, 1);
}

void CameraUI::setupCameraController(const QString& model, const QString& port, const QString& path)
{
    d->controller = new CameraController(this, d->cameraTitle, model, port, path);

    if (d->controller->cameraDriverType() == DKCamera::GPhotoDriver)
    { 
        d->cameraFreeSpace->setMode(FreeSpaceWidget::GPhotoCamera);
        connect(d->controller, SIGNAL(signalFreeSpace(unsigned long, unsigned long)),
                this, SLOT(slotCameraFreeSpaceInfo(unsigned long, unsigned long)));
    }
    else
    {
        d->cameraFreeSpace->setMode(FreeSpaceWidget::UMSCamera);
        d->cameraFreeSpace->setPath(d->controller->cameraPath());
    }

    connect(d->controller, SIGNAL(signalConnected(bool)),
            this, SLOT(slotConnected(bool)));

    connect(d->controller, SIGNAL(signalInfoMsg(const QString&)),
            d->statusProgressBar, SLOT(setProgressText(const QString&)));

    connect(d->controller, SIGNAL(signalErrorMsg(const QString&)),
            this, SLOT(slotErrorMsg(const QString&)));

    connect(d->controller, SIGNAL(signalCameraInformations(const QString&, const QString&, const QString&)),
            this, SLOT(slotCameraInformations(const QString&, const QString&, const QString&)));

    connect(d->controller, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(d->controller, SIGNAL(signalFolderList(const QStringList&)),
            this, SLOT(slotFolderList(const QStringList&)));

    connect(d->controller, SIGNAL(signalFileList(const GPItemInfoList&)),
            this, SLOT(slotFileList(const GPItemInfoList&)));

    connect(d->controller, SIGNAL(signalThumbnail(const QString&, const QString&, const QImage&)),
            this, SLOT(slotThumbnail(const QString&, const QString&, const QImage&)));

    connect(d->controller, SIGNAL(signalDownloaded(const QString&, const QString&, int)),
            this, SLOT(slotDownloaded(const QString&, const QString&, int)));

    connect(d->controller, SIGNAL(signalSkipped(const QString&, const QString&)),
            this, SLOT(slotSkipped(const QString&, const QString&)));

    connect(d->controller, SIGNAL(signalDeleted(const QString&, const QString&, bool)),
            this, SLOT(slotDeleted(const QString&, const QString&, bool)));

    connect(d->controller, SIGNAL(signalLocked(const QString&, const QString&, bool)),
            this, SLOT(slotLocked(const QString&, const QString&, bool)));

    connect(d->controller, SIGNAL(signalExifFromFile(const QString&, const QString&)),
            this, SLOT(slotExifFromFile(const QString&, const QString&)));

    connect(d->controller, SIGNAL(signalExifData(const QByteArray&)),
            this, SLOT(slotExifFromData(const QByteArray&)));

    connect(d->controller, SIGNAL(signalUploaded(const GPItemInfo&)),
            this, SLOT(slotUploaded(const GPItemInfo&)));
}

void CameraUI::setupAccelerators()
{
}

void CameraUI::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Camera Settings");

    d->advBox->setCurrentIndex(group.readEntry("Settings Tab", (int)CameraUIPriv::RENAMEFILEPAGE));
    d->autoRotateCheck->setChecked(group.readEntry("AutoRotate", true));
    d->autoAlbumDateCheck->setChecked(group.readEntry("AutoAlbumDate", false));
    d->autoAlbumExtCheck->setChecked(group.readEntry("AutoAlbumExt", false));
    d->fixDateTimeCheck->setChecked(group.readEntry("FixDateTime", false));
    d->setPhotographerId->setChecked(group.readEntry("SetPhotographerId", false));
    d->setCredits->setChecked(group.readEntry("SetCredits", false));
    d->convertJpegCheck->setChecked(group.readEntry("ConvertJpeg", false));
    d->losslessFormat->setCurrentIndex(group.readEntry("LossLessFormat", 0));   // PNG by default
    d->folderDateFormat->setCurrentIndex(group.readEntry("FolderDateFormat", (int)CameraUIPriv::IsoDateFormat));

    d->view->setThumbnailSize(group.readEntry("ThumbnailSize", (int)ThumbnailSize::Large));

    if (group.hasKey("Splitter State")) 
    {
        QByteArray state;
        state = group.readEntry("Splitter State", state);
        d->splitter->restoreState(QByteArray::fromBase64(state));
    }

    d->dateTimeEdit->setEnabled(d->fixDateTimeCheck->isChecked());
    d->losslessFormat->setEnabled(convertLosslessJpegFiles());
    d->formatLabel->setEnabled(convertLosslessJpegFiles());
    d->folderDateFormat->setEnabled(d->autoAlbumDateCheck->isChecked());
    d->folderDateLabel->setEnabled(d->autoAlbumDateCheck->isChecked());
}

void CameraUI::saveSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Camera Settings");

    group.writeEntry("Settings Tab", d->advBox->currentIndex());
    group.writeEntry("AutoRotate", d->autoRotateCheck->isChecked());
    group.writeEntry("AutoAlbumDate", d->autoAlbumDateCheck->isChecked());
    group.writeEntry("AutoAlbumExt", d->autoAlbumExtCheck->isChecked());
    group.writeEntry("FixDateTime", d->fixDateTimeCheck->isChecked());
    group.writeEntry("SetPhotographerId", d->setPhotographerId->isChecked());
    group.writeEntry("SetCredits", d->setCredits->isChecked());
    group.writeEntry("ConvertJpeg", convertLosslessJpegFiles());
    group.writeEntry("LossLessFormat", d->losslessFormat->currentIndex());
    group.writeEntry("ThumbnailSize", d->view->thumbnailSize());
    group.writeEntry("Splitter State", d->splitter->saveState().toBase64());
    group.writeEntry("FolderDateFormat", d->folderDateFormat->currentIndex());
    config->sync();
}

void CameraUI::slotProcessUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

bool CameraUI::isBusy() const
{
    return d->busy;
}

bool CameraUI::isClosed() const
{
    return d->closed;
}

bool CameraUI::autoRotateJpegFiles() const
{
    return d->autoRotateCheck->isChecked();
}

bool CameraUI::convertLosslessJpegFiles() const
{
    return d->convertJpegCheck->isChecked();
}

QString CameraUI::losslessFormat()
{
    return d->losslessFormat->currentText();
}

QString CameraUI::cameraTitle() const
{
    return d->cameraTitle;
}

void CameraUI::slotCancelButton()
{
    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode, 
                                          i18n("Cancelling current operation, please wait..."));
    d->controller->slotCancel();
    d->currentlyDeleting.clear();
    refreshFreeSpace();
}

void CameraUI::refreshFreeSpace()
{
    if (d->controller->cameraDriverType() == DKCamera::GPhotoDriver)
        d->controller->getFreeSpace();
    else
        d->cameraFreeSpace->refresh();
}

void CameraUI::closeEvent(QCloseEvent* e)
{
    if (dialogClosed())
        e->accept();
    else
        e->ignore();
}

void CameraUI::slotClose()
{
/*FIXME    if (dialogClosed())
        reject();*/
}

bool CameraUI::dialogClosed()
{
    if (d->closed)
        return true;

    if (isBusy())
    {
        if (KMessageBox::questionYesNo(this,
                                       i18n("Do you want to close the dialog "
                                            "and cancel the current operation?"))
            == KMessageBox::No)
            return false;
    }

    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode,
                                          i18n("Disconnecting from camera, please wait..."));

    if (isBusy())
    {
        d->controller->slotCancel();
        // will be read in slotBusy later and finishDialog
        // will be called only when everything is finished
        d->closed = true;
    }
    else
    {
        d->closed = true;
        finishDialog();
    }

    return true;
}

void CameraUI::finishDialog()
{
    // Look if an item have been downloaded to computer during camera gui session.
    // If yes, update the lastAccess date property of camera in digiKam camera list.

    if (d->view->itemsDownloaded() > 0)
    {
        CameraList* clist = CameraList::defaultList();
        if (clist) 
            clist->changeCameraAccessTime(d->cameraTitle, QDateTime::QDateTime::currentDateTime());
    }

    // When a directory is created, a watch is put on it to spot new files
    // but it can occur that the file is copied there before the watch is
    // completely setup. That is why as an extra safeguard run CollectionScanner
    // over the folders we used. Bug: 119201

    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode,
                                          i18n("Scanning for new files, please wait..."));
    CollectionScanner scanner;
    for (QStringList::iterator it = d->foldersToScan.begin();
         it != d->foldersToScan.end(); ++it)
    {
        //DDebug() << "Scanning " << (*it) << endl;
        scanner.partialScan(*it);
    }

    // Never call finalScan after deleteLater() - ScanLib will call processEvent(),
    // and the delete event may be executed!
    deleteLater();

    if(!d->lastDestURL.isEmpty())
        emit signalLastDestination(d->lastDestURL);

    saveSettings();
}

void CameraUI::slotBusy(bool val)
{
    if (!val)   // Camera is available for actions.
    {
        if (!d->busy)
            return;

        d->busy = false;
        d->cameraCancelAction->setEnabled(false);

        d->advBox->setEnabled(true);
        // B.K.O #127614: The Focus need to be restored in custom prefix widget.
        // commenting this out again: If we do not disable, no need to restore focus
        // d->renameCustomizer->restoreFocus();

        d->uploadAction->setEnabled(d->controller->cameraUploadSupport());
        d->downloadSelectedAction->setEnabled(true);
        d->downloadDelSelectedAction->setEnabled(d->controller->cameraDeleteSupport());
        d->downloadAllAction->setEnabled(true);
        d->downloadDelAllAction->setEnabled(d->controller->cameraDeleteSupport());
        d->deleteSelectedAction->setEnabled(d->controller->cameraDeleteSupport());
        d->deleteAllAction->setEnabled(d->controller->cameraDeleteSupport());
        d->selectAllAction->setEnabled(true);
        d->selectNoneAction->setEnabled(true);
        d->selectInvertAction->setEnabled(true);
        d->selectNewItemsAction->setEnabled(true);
        d->lockAction->setEnabled(true);
        d->cameraInfoAction->setEnabled(true);
        d->cameraCaptureAction->setEnabled(d->controller->cameraCaptureImageSupport());

        d->anim->stop();
        d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode, i18n("Ready"));

        // like WDestructiveClose, but after camera controller operation has safely finished
        if (d->closed)
        {
            finishDialog();
        }
    }
    else    // Camera is busy.
    {
        if (d->busy)
            return;

        if (!d->anim->running())
            d->anim->start();

        d->busy = true;
        d->cameraCancelAction->setEnabled(true);
        d->statusProgressBar->progressBarMode(StatusProgressBar::ProgressBarMode);

        // Settings tab is disabled in slotDownload, selectively when downloading
        // Fast dis/enabling would create the impression of flicker, e.g. when retrieving EXIF from camera
        //d->advBox->setEnabled(false);

        d->uploadAction->setEnabled(false);
        d->downloadSelectedAction->setEnabled(false);
        d->downloadDelSelectedAction->setEnabled(false);
        d->downloadAllAction->setEnabled(false);
        d->downloadDelAllAction->setEnabled(false);
        d->deleteSelectedAction->setEnabled(false);
        d->deleteAllAction->setEnabled(false);
        d->selectAllAction->setEnabled(false);
        d->selectNoneAction->setEnabled(false);
        d->selectInvertAction->setEnabled(false);
        d->selectNewItemsAction->setEnabled(false);
        d->lockAction->setEnabled(false);
        d->cameraInfoAction->setEnabled(false);
        d->cameraCaptureAction->setEnabled(false);
    }
}

void CameraUI::slotIncreaseThumbSize()
{
    int thumbSize = d->view->thumbnailSize() + ThumbnailSize::Step;
    d->view->setThumbnailSize(thumbSize);
}

void CameraUI::slotDecreaseThumbSize()
{
    int thumbSize = d->view->thumbnailSize() - ThumbnailSize::Step;
    d->view->setThumbnailSize(thumbSize);
}

void CameraUI::slotZoomSliderChanged(int size)
{
    d->view->setThumbnailSize(size);
}

void CameraUI::slotThumbSizeChanged(int size)
{
    d->statusZoomBar->setZoomSliderValue(size);
    d->statusZoomBar->setZoomTrackerText(i18n("Size: %1", size));

    d->statusZoomBar->setEnableZoomPlus(true);
    d->statusZoomBar->setEnableZoomMinus(true);
    d->increaseThumbsAction->setEnabled(true);
    d->decreaseThumbsAction->setEnabled(true);

    if (d->view->thumbnailSize() == ThumbnailSize::Small)
    {
        d->decreaseThumbsAction->setEnabled(false);
        d->statusZoomBar->setEnableZoomMinus(false);
    }

    if (d->view->thumbnailSize() == ThumbnailSize::Huge)
    {
        d->increaseThumbsAction->setEnabled(false);
        d->statusZoomBar->setEnableZoomPlus(false);
    }
}

void CameraUI::slotConnected(bool val)
{
    if (!val)
    {
      if (KMessageBox::warningYesNo(this,
                                    i18n("Failed to connect to the camera. "
                                         "Please make sure it is connected "
                                         "properly and turned on. "
                                         "Would you like to try again?"), 
                                    i18n("Connection Failed"),
                                    KGuiItem(i18n("Retry")),
                                    KGuiItem(i18n("Abort")))
          == KMessageBox::Yes)
          QTimer::singleShot(0, d->controller, SLOT(slotConnect()));
      else
          close();
    }
    else
    {
        refreshFreeSpace();
        d->controller->listFolders();
    }
}

void CameraUI::slotFolderList(const QStringList& folderList)
{
    if (d->closed)
        return;

    d->statusProgressBar->setProgressValue(0);
    d->statusProgressBar->setProgressTotalSteps(0);

    d->cameraFolderList = folderList;
    for (QStringList::const_iterator it = folderList.begin();
         it != folderList.end(); ++it)
    {
        d->controller->listFiles(*it);
    }
}

void CameraUI::slotFileList(const GPItemInfoList& fileList)
{
    if (d->closed)
        return;

    if (fileList.empty())
        return;

    for (GPItemInfoList::const_iterator it = fileList.begin();
         it != fileList.end(); ++it)
    {
        GPItemInfo item = *it;

        if (item.mtime > d->lastAccess && item.downloaded == GPItemInfo::DownloadUnknow)
           item.downloaded = GPItemInfo::NewPicture;

        d->view->addItem(item);
        d->controller->getThumbnail(item.folder, item.name);
    }

    d->statusProgressBar->setProgressTotalSteps(d->statusProgressBar->progressTotalSteps() +
                                                fileList.count());
}

void CameraUI::slotThumbnail(const QString& folder, const QString& file,
                             const QImage& thumbnail)
{
    d->view->setThumbnail(folder, file, thumbnail);
    int curr = d->statusProgressBar->progressValue();
    d->statusProgressBar->setProgressValue(curr+1);
}

void CameraUI::slotCapture()
{
    if (d->busy) 
        return;

    CaptureDlg *captureDlg = new CaptureDlg(this, d->controller, d->cameraTitle);
    captureDlg->show();
}

void CameraUI::slotInformations()
{
    if (d->busy) 
        return;

    d->controller->getCameraInformations();
}

void CameraUI::slotCameraInformations(const QString& summary, const QString& manual, const QString& about)
{
    CameraInfoDialog *infoDlg = new CameraInfoDialog(this, summary, manual, about);
    infoDlg->show();
}

void CameraUI::slotErrorMsg(const QString& msg)
{
    KMessageBox::error(this, msg);    
}

void CameraUI::slotUpload()
{
    if (d->busy)
        return;

    QString fileformats;

    QStringList patternList = KImageIO::pattern(KImageIO::Reading).split('\n');

    // All Images from list must been always the first entry given by KDE API
    QString allPictures = patternList[0];

    // Add RAW file format to All Images" type mime and remplace current.
    allPictures.insert(allPictures.indexOf("|"), QString(KDcrawIface::DcrawBinary::instance()->rawFiles()));
    patternList.removeAll(patternList[0]);
    patternList.prepend(allPictures);

    // Added RAW file formats supported by dcraw program like a type mime. 
    // Nota: we cannot use here "image/x-raw" type mime from KDE because it uncomplete 
    // or unavailable(dcraw_0)(see file #121242 in B.K.O).
    patternList.append(QString("\n%1|Camera RAW files").arg(QString(KDcrawIface::DcrawBinary::instance()->rawFiles())));

    fileformats = patternList.join("\n");

    DDebug () << "fileformats=" << fileformats << endl;

    KUrl::List urls = KFileDialog::getOpenUrls(CollectionManager::instance()->oneAlbumRootPath(),
                                               fileformats, this, i18n("Select Image to Upload"));
    if (!urls.isEmpty())
        slotUploadItems(urls);
}

void CameraUI::slotUploadItems(const KUrl::List& urls)
{
    if (d->busy)
        return;

    if (urls.isEmpty())
        return;

    if (d->cameraFreeSpace->isValid())
    {
        // Check if space require to upload new items in camera is enough.
        qint64 totalKbSize = 0;
        for (KUrl::List::const_iterator it = urls.begin() ; it != urls.end() ; ++it)
        {
            QFileInfo fi((*it).path());
            totalKbSize += fi.size()/1024;
        }
    
        if (totalKbSize >= d->cameraFreeSpace->kBAvail())
        {
            KMessageBox::error(this, i18n("There is no enough free space on Camera Media "
                                          "to upload pictures.\n\n"
                                          "Space require: %1\n"
                                          "Available free space: %2",
                                    KIO::convertSizeFromKiB(totalKbSize)),
                                    KIO::convertSizeFromKiB(d->cameraFreeSpace->kBAvail()));
            return;
        } 
    }

    CameraFolderDialog dlg(this, d->view, d->cameraFolderList, 
                           d->controller->cameraTitle(),
                           d->controller->cameraPath());

    if (dlg.exec() != QDialog::Accepted)
        return;

    QString cameraFolder = dlg.selectedFolderPath();

    for (KUrl::List::const_iterator it = urls.begin() ; it != urls.end() ; ++it)
    {
        QFileInfo fi((*it).path());
        if (!fi.exists()) continue;
        if (fi.isDir()) continue;

        QString ext  = QString(".") + fi.completeSuffix();
        QString name = fi.fileName();
        name.truncate(fi.fileName().length() - ext.length());

        bool ok;

        while (d->view->findItem(cameraFolder, name + ext)) 
        {
            QString msg(i18n("Camera Folder <b>%1</b> already contains item <b>%2</b><br>"
                             "Please enter a new file name (without extension):",
                             cameraFolder, fi.fileName()));
            name = KInputDialog::getText(i18n("File already exists"), msg, name, &ok, this);

            if (!ok) 
                return;
        }

        d->controller->upload(fi, name + ext, cameraFolder);
    }
}

void CameraUI::slotUploaded(const GPItemInfo& itemInfo)
{
    if (d->closed)
        return;

    d->view->addItem(itemInfo);
    d->controller->getThumbnail(itemInfo.folder, itemInfo.name);
    refreshFreeSpace();
}

void CameraUI::slotDownloadSelected()
{
    slotDownload(true, false);
}

void CameraUI::slotDownloadAndDeleteSelected()
{
    slotDownload(true, true);
}

void CameraUI::slotDownloadAll()
{
    slotDownload(false, false);
}

void CameraUI::slotDownloadAndDeleteAll()
{
    slotDownload(false, true);
}

void CameraUI::slotDownload(bool onlySelected, bool deleteAfter, Album *album)
{
    // See B.K.O #143934: force to select all items to prevent problem 
    // when !renameCustomizer->useDefault() ==> iconItem->getDownloadName()
    // can return an empty string in this case because it depends on selection.
    if (!onlySelected)
        d->view->slotSelectAll();

    // See B.K.O #139519: Always check free space available before to
    // download items selection from camera.
    unsigned long fSize = 0;
    unsigned long dSize = 0;
    d->view->itemsSelectionSizeInfo(fSize, dSize);
    if (d->albumLibraryFreeSpace->isValid() && 
        (dSize >= d->albumLibraryFreeSpace->kBAvail()))
    {
        KMessageBox::error(this, i18n("There is no enough free space on Album Library Path "
                                      "to download and process selected pictures from camera.\n\n"
                                      "Estimated space require: %1\n"
                                      "Available free space: %2",
                                 KIO::convertSizeFromKiB(dSize)),
                                 KIO::convertSizeFromKiB(d->albumLibraryFreeSpace->kBAvail()));
        return;
    }

    QString   newDirName;
    IconItem* firstItem = d->view->firstItem();
    if (firstItem)
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(firstItem);

        QDateTime dateTime = iconItem->itemInfo()->mtime;

        switch(d->folderDateFormat->currentIndex())
        {
            case CameraUIPriv::TextDateFormat:
                newDirName = dateTime.date().toString(Qt::TextDate);
                break;
            case CameraUIPriv::LocalDateFormat:
                newDirName = dateTime.date().toString(Qt::LocalDate);
                break;
            default:        // IsoDateFormat
                newDirName = dateTime.date().toString(Qt::ISODate);
                break;
        }
    }

    // -- Get the destination album from digiKam library ---------------

    if (!album)
    {
        AlbumManager* man = AlbumManager::instance();

        album = man->currentAlbum();
        if (album && album->type() != Album::PHYSICAL)
            album = 0;

        QString header(i18n("<p>Please select the destination album from the digiKam library to "
                            "import the camera pictures into.</p>"));

        album = AlbumSelectDialog::selectAlbum(this, (PAlbum*)album, header, newDirName,
                                               d->autoAlbumDateCheck->isChecked());

        if (!album) return;
    }

    PAlbum *pAlbum = dynamic_cast<PAlbum*>(album);
    if (!pAlbum) return;

    // -- Prepare downloading of camera items ------------------------

    KUrl url;
    url.setPath(pAlbum->folderPath());

    d->controller->downloadPrep();

    DownloadSettingsContainer downloadSettings;
    QString   downloadName;
    QDateTime dateTime;
    int       total = 0;

    downloadSettings.autoRotate        = d->autoRotateCheck->isChecked();
    downloadSettings.fixDateTime       = d->fixDateTimeCheck->isChecked();
    downloadSettings.newDateTime       = d->dateTimeEdit->dateTime();
    downloadSettings.setPhotographerId = d->setPhotographerId->isChecked();
    downloadSettings.setCredits        = d->setCredits->isChecked();
    downloadSettings.convertJpeg       = convertLosslessJpegFiles();
    downloadSettings.losslessFormat    = losslessFormat();

    AlbumSettings* settings = AlbumSettings::instance();
    if (settings)
    {
        downloadSettings.author      = settings->getAuthor();
        downloadSettings.authorTitle = settings->getAuthorTitle();
        downloadSettings.credit      = settings->getCredit();
        downloadSettings.source      = settings->getSource();
        downloadSettings.copyright   = settings->getCopyright();
    }

    // -- Download camera items -------------------------------

    for (IconItem* item = d->view->firstItem(); item;
         item = item->nextItem())
    {
        if (onlySelected && !(item->isSelected()))
            continue;

        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        downloadSettings.folder      = iconItem->itemInfo()->folder;
        downloadSettings.file        = iconItem->itemInfo()->name;
        downloadName                 = iconItem->getDownloadName();
        dateTime                     = iconItem->itemInfo()->mtime;

        KUrl u(url);
        QString errMsg;
                    
        // Auto sub-albums creation based on file date.     

        if (d->autoAlbumDateCheck->isChecked())
        {
            QString dirName;

            switch(d->folderDateFormat->currentIndex())
            {
                case CameraUIPriv::TextDateFormat:
                    dirName = dateTime.date().toString(Qt::TextDate);
                    break;
                case CameraUIPriv::LocalDateFormat:
                    dirName = dateTime.date().toString(Qt::LocalDate);
                    break;
                default:        // IsoDateFormat
                    dirName = dateTime.date().toString(Qt::ISODate);
                    break;
            }

            // See B.K.O #136927 : we need to support file system which do not
            // handle upper case properly.
            dirName = dirName.toLower();
            if (!createAutoAlbum(url, dirName, dateTime.date(), errMsg))
            {
                KMessageBox::error(this, errMsg);
                return;
            }

            u.addPath(dirName);
        }

        // Auto sub-albums creation based on file extensions.

        if (d->autoAlbumExtCheck->isChecked())           
        {
            // We use the target file name to compute sub-albums name to take a care about 
            // convertion on the fly option.
            QFileInfo fi(downloadName);

            QString subAlbum = fi.suffix().toUpper();
	        if (fi.suffix().toUpper() == QString("JPEG") || 
                fi.suffix().toUpper() == QString("JPE")) 
                subAlbum = QString("JPG");
            if (fi.suffix().toUpper() == QString("TIFF")) 
                subAlbum = QString("TIF");
            if (fi.suffix().toUpper() == QString("MPEG") || 
                fi.suffix().toUpper() == QString("MPE") ||
                fi.suffix().toUpper() == QString("MPO"))
                subAlbum = QString("MPG");

            // See B.K.O #136927 : we need to support file system which do not
            // handle upper case properly.
            subAlbum = subAlbum.toLower();
            if (!createAutoAlbum(u, subAlbum, dateTime.date(), errMsg))
            {
                KMessageBox::error(this, errMsg);
                return;
            }

            u.addPath(subAlbum);
        }

        d->foldersToScan.append(u.path());
        u.addPath(downloadName.isEmpty() ? downloadSettings.file : downloadName);

        downloadSettings.dest = u.path();

        d->controller->download(downloadSettings);
        addFileExtension(QFileInfo(u.path()).suffix());
        total++;
    }

    if (total <= 0)
        return;
    
    d->lastDestURL = url;
    d->statusProgressBar->setProgressValue(0);
    d->statusProgressBar->setProgressTotalSteps(total);
    d->statusProgressBar->progressBarMode(StatusProgressBar::ProgressBarMode);

    // disable settings tab here instead of slotBusy:
    // Only needs to be disabled while downloading
    d->advBox->setEnabled(false);

    if (deleteAfter)
    {
        if (onlySelected)
            slotDeleteSelected();
        else
            slotDeleteAll();
    }
}

void CameraUI::slotDownloaded(const QString& folder, const QString& file, int status)
{
    CameraIconViewItem* iconItem = d->view->findItem(folder, file);
    if (iconItem)
    {
        iconItem->setDownloaded(status);

        //if (iconItem->isSelected())
          //  slotItemsSelected(iconItem, true);
    }

    if (status == GPItemInfo::DownloadedYes || status == GPItemInfo::DownloadFailed)
    {
        int curr = d->statusProgressBar->progressValue();
        d->statusProgressBar->setProgressValue(curr+1);
    }
}

void CameraUI::slotSkipped(const QString& folder, const QString& file)
{
    CameraIconViewItem* iconItem = d->view->findItem(folder, file);
    if (iconItem)
        iconItem->setDownloaded(GPItemInfo::DownloadedNo);

    int curr = d->statusProgressBar->progressValue();
    d->statusProgressBar->setProgressValue(curr+1);
}

void CameraUI::slotToggleLock()
{
    int count = 0;
    for (IconItem* item = d->view->firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        if (iconItem->isSelected())
        {
            QString folder = iconItem->itemInfo()->folder;
            QString file   = iconItem->itemInfo()->name;
            int writePerm  = iconItem->itemInfo()->writePermissions;
            bool lock      = true;

            // If item is currently locked, unlock it.
            if (writePerm == 0) 
                lock = false;

            d->controller->lockFile(folder, file, lock);
            count++;
        }
    }

    if (count > 0)
    {
        d->statusProgressBar->setProgressValue(0);
        d->statusProgressBar->setProgressTotalSteps(count);
        d->statusProgressBar->progressBarMode(StatusProgressBar::ProgressBarMode);
    }
}

void CameraUI::slotLocked(const QString& folder, const QString& file, bool status)
{
    if (status)
    {
        CameraIconViewItem* iconItem = d->view->findItem(folder, file);
        if (iconItem)
        {
            iconItem->toggleLock();
            //if (iconItem->isSelected())
              //  slotItemsSelected(iconItem, true);
        }
    }

    int curr = d->statusProgressBar->progressValue();
    d->statusProgressBar->setProgressValue(curr+1);
}

void CameraUI::slotDeleteSelected()
{
    QStringList folders;
    QStringList files;
    QStringList deleteList;
    QStringList lockedList;

    for (IconItem* item = d->view->firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        if (iconItem->isSelected())
        {
            if (iconItem->itemInfo()->writePermissions != 0)  // Item not locked ?
            {
                QString folder = iconItem->itemInfo()->folder;
                QString file   = iconItem->itemInfo()->name;
                folders.append(folder);
                files.append(file);
                deleteList.append(folder + QString("/") + file);
            }
            else
            {
                lockedList.append(iconItem->itemInfo()->name);
            }
        }
    }

    // If we want to delete some locked files, just give a feedback to user.
    if (!lockedList.isEmpty())
    {
        QString infoMsg(i18n("The items listed below are locked by camera (read-only). "
                             "These items will not be deleted. If you really want to delete these items, "
                             "please unlock them and try again."));
        KMessageBox::informationList(this, infoMsg, lockedList, i18n("Information"));
    }

    if (folders.isEmpty())
        return;

    QString warnMsg(i18np("About to delete this image. "
                          "Deleted file is unrecoverable. "
                          "Are you sure?",
                          "About to delete these %1 images. "
                          "Deleted files are unrecoverable. "
                          "Are you sure?",
                          deleteList.count()));
    if (KMessageBox::warningContinueCancelList(this, warnMsg,
                                               deleteList,
                                               i18n("Warning"),
                                               KGuiItem(i18n("Delete")))
        ==  KMessageBox::Continue) 
    {
        QStringList::iterator itFolder = folders.begin();
        QStringList::iterator itFile   = files.begin();

        d->statusProgressBar->setProgressValue(0);
        d->statusProgressBar->setProgressTotalSteps(deleteList.count());
        d->statusProgressBar->progressBarMode(StatusProgressBar::ProgressBarMode);

        for ( ; itFolder != folders.end(); ++itFolder, ++itFile)
        {
            d->controller->deleteFile(*itFolder, *itFile);
            // the currentlyDeleting list is used to prevent loading items which
            // will immenently be deleted into the sidebar and wasting time
            d->currentlyDeleting.append(*itFolder + *itFile);
        }
    }
}

void CameraUI::slotDeleteAll()
{
    QStringList folders;
    QStringList files;
    QStringList deleteList;
    QStringList lockedList;

    for (IconItem* item = d->view->firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        if (iconItem->itemInfo()->writePermissions != 0)  // Item not locked ?
        {
            QString folder = iconItem->itemInfo()->folder;
            QString file   = iconItem->itemInfo()->name;
            folders.append(folder);
            files.append(file);
            deleteList.append(folder + QString("/") + file);
        }
        else
        {
            lockedList.append(iconItem->itemInfo()->name);
        }
    }

    // If we want to delete some locked files, just give a feedback to user.
    if (!lockedList.isEmpty())
    {
        QString infoMsg(i18n("The items listed below are locked by camera (read-only). "
                             "These items will not be deleted. If you really want to delete these items, "
                             "please unlock them and try again."));
        KMessageBox::informationList(this, infoMsg, lockedList, i18n("Information"));
    }

    if (folders.isEmpty())
        return;

    QString warnMsg(i18np("About to delete this image. "
                          "Deleted file is unrecoverable. "
                          "Are you sure?",
                          "About to delete these %1 images. "
                          "Deleted files are unrecoverable. "
                          "Are you sure?",
                          deleteList.count()));
    if (KMessageBox::warningContinueCancelList(this, warnMsg,
                                               deleteList,
                                               i18n("Warning"),
                                               KGuiItem(i18n("Delete")))
        ==  KMessageBox::Continue) 
    {
        QStringList::iterator itFolder = folders.begin();
        QStringList::iterator itFile   = files.begin();

        d->statusProgressBar->setProgressValue(0);
        d->statusProgressBar->setProgressTotalSteps(deleteList.count());
        d->statusProgressBar->progressBarMode(StatusProgressBar::ProgressBarMode);

        for ( ; itFolder != folders.end(); ++itFolder, ++itFile)
        {
            d->controller->deleteFile(*itFolder, *itFile);
            d->currentlyDeleting.append(*itFolder + *itFile);
        }
    }
}

void CameraUI::slotDeleted(const QString& folder, const QString& file, bool status)
{
    if (status)
    {
        d->view->removeItem(folder, file);
        // do this after removeItem, which will signal to slotItemsSelected, which checks for the list
        d->currentlyDeleting.removeAll(folder + file);
    }

    int curr = d->statusProgressBar->progressValue();
    d->statusProgressBar->setProgressTotalSteps(curr+1);
    refreshFreeSpace();
}

void CameraUI::slotFileView()
{
    CameraIconViewItem* item = d->view->firstItemSelected();
    if (item)
        slotFileView(item);
}

void CameraUI::slotFileView(CameraIconViewItem* item)
{
    d->controller->openFile(item->itemInfo()->folder, item->itemInfo()->name);
}

void CameraUI::slotExifFromFile(const QString& folder, const QString& file)
{
    CameraIconViewItem* item = d->view->findItem(folder, file);
    if (!item)
        return;

    d->rightSidebar->itemChanged(item->itemInfo(), folder + QString("/") + file, 
                                 QByteArray(), d->view, item);
}

void CameraUI::slotExifFromData(const QByteArray& exifData)
{
    CameraIconViewItem* item = dynamic_cast<CameraIconViewItem*>(d->view->currentItem());

    if (!item)
        return;

    KUrl url(item->itemInfo()->folder + '/' + item->itemInfo()->name);

    // Sometimes, GPhoto2 drivers return complete APP1 JFIF section. Exiv2 cannot 
    // decode (yet) exif metadata from APP1. We will find Exif header to get data at this place 
    // to please with Exiv2...

    DDebug() << "Size of Exif metadata from camera = " << exifData.size() << endl;
    char exifHeader[] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };

    if (!exifData.isEmpty())
    {
        int i = exifData.indexOf(*exifHeader);
        if (i != -1)
        {
            DDebug() << "Exif header found at position " << i << endl;
            i = i + sizeof(exifHeader);
            QByteArray data;
            data.resize(exifData.size()-i);
            memcpy(data.data(), exifData.data()+i, data.size());
            d->rightSidebar->itemChanged(item->itemInfo(), url, data, d->view, item);
            return;
        }
    }

    d->rightSidebar->itemChanged(item->itemInfo(), url, exifData, d->view, item);
}

void CameraUI::slotNewSelection(bool hasSelection)
{
    if (!d->controller) return;

    if (!d->renameCustomizer->useDefault())
    {
        d->downloadSelectedAction->setEnabled(hasSelection);
        d->downloadDelSelectedAction->setEnabled(hasSelection & d->controller->cameraDeleteSupport());
        d->deleteSelectedAction->setEnabled(hasSelection & d->controller->cameraDeleteSupport());
        d->imageViewAction->setEnabled(hasSelection);
    }
    else
    {
        d->downloadSelectedAction->setEnabled(hasSelection);
        d->downloadDelSelectedAction->setEnabled(hasSelection & d->controller->cameraDeleteSupport());
        d->deleteSelectedAction->setEnabled(hasSelection & d->controller->cameraDeleteSupport());
        d->imageViewAction->setEnabled(hasSelection);
    }

    unsigned long fSize = 0;
    unsigned long dSize = 0;
    d->view->itemsSelectionSizeInfo(fSize, dSize);
    d->albumLibraryFreeSpace->setEstimatedDSizeKb(dSize);
}

void CameraUI::slotItemsSelected(CameraIconViewItem* item, bool selected)
{
    if (!d->controller) return;

    d->downloadSelectedAction->setEnabled(selected);
    d->downloadDelSelectedAction->setEnabled(selected & d->controller->cameraDeleteSupport());
    d->deleteSelectedAction->setEnabled(selected & d->controller->cameraDeleteSupport());
    d->imageViewAction->setEnabled(selected);

    if (selected)
    {
        // if selected item is in the list of item which will be deleted, set no current item
        if (!d->currentlyDeleting.contains(item->itemInfo()->folder + item->itemInfo()->name))
        {
            KUrl url(item->itemInfo()->folder + '/' + item->itemInfo()->name);
            d->rightSidebar->itemChanged(item->itemInfo(), url, QByteArray(), d->view, item);
            d->controller->getExif(item->itemInfo()->folder, item->itemInfo()->name);
        }
        else
        {
            d->rightSidebar->slotNoCurrentItem();
        }
    }
    else
        d->rightSidebar->slotNoCurrentItem();
}

bool CameraUI::createAutoAlbum(const KUrl& parentURL, const QString& sub,
                               const QDate& date, QString& errMsg)
{
    KUrl u(parentURL);
    u.addPath(sub);

    // first stat to see if the album exists
    QFileInfo info(u.path());
    if (info.exists())
    {
        // now check if its really a directory
        if (info.isDir())
            return true;
        else
        {
            errMsg = i18n("A file with same name (%1) exists in folder %2",
                          sub, parentURL.path());
            return false;
        }
    }

    // looks like the directory does not exist, try to create it

    AlbumManager* aman = AlbumManager::instance();
    PAlbum* parent     = aman->findPAlbum(parentURL);
    if (!parent)
    {
        errMsg = i18n("Failed to find Album for path '%1'", parentURL.path());
        return false;
    }
    QString albumRootPath = CollectionManager::instance()->albumRootPath(parentURL);

    return aman->createPAlbum(albumRootPath, sub, QString(""), date, QString(""), errMsg);
}

void CameraUI::addFileExtension(const QString& ext)
{
    AlbumSettings::instance()->addToImageFileFilter(ext);
}

void CameraUI::slotFirstItem()
{
    CameraIconViewItem *currItem = dynamic_cast<CameraIconViewItem*>(d->view->firstItem());
    d->view->clearSelection();
    d->view->updateContents();
    if (currItem) 
       d->view->setCurrentItem(currItem);
}

void CameraUI::slotPrevItem()
{
    CameraIconViewItem *currItem = dynamic_cast<CameraIconViewItem*>(d->view->currentItem());
    d->view->clearSelection();
    d->view->updateContents();
    if (currItem)
       d->view->setCurrentItem(currItem->prevItem());
}

void CameraUI::slotNextItem()
{
    CameraIconViewItem *currItem = dynamic_cast<CameraIconViewItem*>(d->view->currentItem());
    d->view->clearSelection();
    d->view->updateContents();
    if (currItem) 
       d->view->setCurrentItem(currItem->nextItem());
}

void CameraUI::slotLastItem()
{
    CameraIconViewItem *currItem = dynamic_cast<CameraIconViewItem*>(d->view->lastItem());
    d->view->clearSelection();
    d->view->updateContents();
    if (currItem) 
       d->view->setCurrentItem(currItem);
}

void CameraUI::slotDonateMoney()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=donation");
}

void CameraUI::slotContribute()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=contrib");
}

void CameraUI::slotEditKeys()
{
    KShortcutsDialog dialog(KShortcutsEditor::AllActions,
                            KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection( actionCollection(), i18n( "General" ) );
    dialog.configure();
}

void CameraUI::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config()->group("Camera Settings"));
    KEditToolBar dlg(factory(), this);

    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(slotNewToolbarConfig()));

    dlg.exec();
}

void CameraUI::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config()->group("Camera Settings"));
}

void CameraUI::slotSetup()
{
    Setup setup(this, 0);

    if (setup.exec() != QDialog::Accepted)
        return;

    KGlobal::config()->sync();
}

void CameraUI::slotToggleFullScreen()
{
    if (d->fullScreen) // out of fullscreen
    {
        setWindowState( windowState() & ~Qt::WindowFullScreen ); // reset

        menuBar()->show();
        statusBar()->show();
        showToolBars();

        if (d->removeFullScreenButton)
        {
            QList<KToolBar *> toolbars = toolBars();
            foreach(KToolBar *toolbar, toolbars)
            {
                // name is set in ui.rc XML file
                if (toolbar->objectName() == "ToolBar")
                {
                    toolbar->removeAction(d->fullScreenAction);
                    break;
                }
            }
        }

        d->rightSidebar->restore();

        d->fullScreen = false;
    }
    else  // go to fullscreen
    {
        // hide the menubar and the statusbar
        menuBar()->hide();
        statusBar()->hide();

        if (d->fullScreenHideToolBar)
        {
            hideToolBars();
        }
        else
        {
            showToolBars();

            QList<KToolBar *> toolbars = toolBars();
            KToolBar *mainToolbar = 0;
            foreach(KToolBar *toolbar, toolbars)
            {
                if (toolbar->objectName() == "ToolBar")
                {
                    mainToolbar = toolbar;
                    break;
                }
            }

            // add fullscreen action if necessary
            if ( mainToolbar && !mainToolbar->actions().contains(d->fullScreenAction) )
            {
                mainToolbar->addAction(d->fullScreenAction);
                d->removeFullScreenButton=true;
            }
            else
            {
                // If FullScreen button is enabled in toolbar settings,
                // we shall not remove it when leaving of fullscreen mode.
                d->removeFullScreenButton=false;
            }
        }

        d->rightSidebar->backup();

        setWindowState( windowState() | Qt::WindowFullScreen ); // set
        d->fullScreen = true;
    }
}

void CameraUI::slotEscapePressed()
{
    if (d->fullScreen)
        d->fullScreenAction->activate(QAction::Trigger);
}

void CameraUI::showToolBars()
{
    QList<KToolBar *> toolbars = toolBars();
    foreach(KToolBar *toolbar, toolbars)
    {
        toolbar->show();
    }
}

void CameraUI::hideToolBars()
{
    QList<KToolBar *> toolbars = toolBars();
    foreach(KToolBar *toolbar, toolbars)
    {
        toolbar->hide();
    }
}

void CameraUI::slotCameraFreeSpaceInfo(unsigned long kBSize, unsigned long kBAvail)
{
    d->cameraFreeSpace->setInformations(kBSize, kBSize-kBAvail, kBAvail, QString());
}

bool CameraUI::cameraDeleteSupport()
{
    return d->controller->cameraDeleteSupport();
}

bool CameraUI::cameraUploadSupport()
{
    return d->controller->cameraUploadSupport();
}

bool CameraUI::cameraMkDirSupport()
{
    return d->controller->cameraMkDirSupport();
}

bool CameraUI::cameraDelDirSupport()
{
    return d->controller->cameraDelDirSupport();
}

void CameraUI::slotRawCameraList()
{
    RawCameraDlg dlg(this);
    dlg.exec();
}

void CameraUI::slotThemeChanged()
{
    QStringList themes(ThemeEngine::instance()->themeNames());
    int index = themes.indexOf(AlbumSettings::instance()->getCurrentTheme());
    if (index == -1)
        index = themes.indexOf(i18n("Default"));

    d->themeMenuAction->setCurrentItem(index);
}

void CameraUI::slotChangeTheme(const QString& theme)
{
    // Theme menu entry is returned with keyboard accelerator. We remove it.
    QString name = theme;
    name.remove(QChar('&'));
    AlbumSettings::instance()->setCurrentTheme(theme);
    ThemeEngine::instance()->slotChangeTheme(theme);
}

void CameraUI::slotComponentsInfo()
{
    QMap<QString, QString> list;
    list.insert(i18n("LibQt"),                            qVersion());
    list.insert(i18n("LibKDE"),                           KDE::versionString());
    list.insert(i18n("LibKipi"),                          KIPI::Interface::version());
    list.insert(i18n("LibKdcraw"),                        KDcrawIface::KDcraw::version());
    list.insert(i18n("Dcraw program"),                    KDcrawIface::DcrawBinary::internalVersion());
    list.insert(i18n("LibKExiv2"),                        KExiv2Iface::KExiv2::version());
    list.insert(i18n("LibExiv2"),                         KExiv2Iface::KExiv2::Exiv2Version());
    list.insert(i18n("Exiv2 support XMP metadata"),       KExiv2Iface::KExiv2::supportXmp() ? 
                                                          i18n("Yes") : i18n("No"));
    list.insert(i18n("Exiv2 can write metadata to Tiff"), KExiv2Iface::KExiv2::supportTiffWritting() ? 
                                                          i18n("Yes") : i18n("No"));
    list.insert(i18n("LibPNG"),                           QString(PNG_LIBPNG_VER_STRING));
    list.insert(i18n("LibTIFF"),                          QString(TIFFLIB_VERSION_STR).replace('\n', ' '));
    list.insert(i18n("LibJPEG"),                          QString::number(JPEG_LIB_VERSION));
    list.insert(i18n("LibCImg"),                          GreycstorationIface::cimgVersionString());

#ifdef HAVE_MARBLEWIDGET
    list.insert(i18n("Marble widget"),                    QString(MARBLE_VERSION_STRING));
#endif //HAVE_MARBLEWIDGET

    list.insert(i18n("LibGphoto2"),                       QString(gp_library_version(GP_VERSION_SHORT)[0]));

    LibsInfoDlg dlg(this);
    dlg.setComponentsInfoMap(list);
    dlg.exec();
}

}  // namespace Digikam
