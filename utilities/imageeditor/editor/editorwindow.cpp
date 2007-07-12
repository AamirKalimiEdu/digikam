/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2006-01-20
 * Description : main image editor GUI implementation
 *
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

// C Ansi includes.

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
}

// C++ includes.

#include <cmath>

// Qt includes.

#include <Q3PtrList>
#include <QLabel>
#include <QLayout>
#include <QToolButton>
#include <QSplitter>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QCursor>
#include <QTimer>
#include <QByteArray>
#include <QProgressBar>
#include <QWidgetAction>
#include <QImageReader>

// KDE includes.

#include <kxmlguifactory.h>
#include <ktoolinvocation.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kprinter.h>
#include <kedittoolbar.h>
#include <kaboutdata.h>
#include <kcursor.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kimageio.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kprotocolinfo.h>
#include <kglobalsettings.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <kwindowsystem.h>
#include <kcombobox.h>
#include <ktoggleaction.h>
#include <kshortcutsdialog.h>
#include <ktoolbarpopupaction.h>
#include <kservice.h>
#include <kservicetype.h>
#include <kservicetypetrader.h>

// Local includes.

#include "ddebug.h"
#include "dpopupmenu.h"
#include "canvas.h"
#include "dimginterface.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imageresize.h"
#include "imageprint.h"
#include "filesaveoptionsbox.h"
#include "statusprogressbar.h"
#include "iccsettingscontainer.h"
#include "exposurecontainer.h"
#include "iofilesettingscontainer.h"
#include "savingcontextcontainer.h"
#include "loadingcacheinterface.h"
#include "slideshowsettings.h"
#include "editorwindowprivate.h"
#include "editorwindow.h"
#include "editorwindow.moc"

namespace Digikam
{

EditorWindow::EditorWindow(const char *name)
            : KXmlGuiWindow(0)
{
    d = new EditorWindowPriv;

    setObjectName(name);
    setWindowFlags(Qt::Window);

    m_contextMenu            = 0;
    m_canvas                 = 0;
    m_imagePluginLoader      = 0;
    m_undoAction             = 0;
    m_redoAction             = 0;
    m_fullScreenAction       = 0;
    m_saveAction             = 0;
    m_saveAsAction           = 0;
    m_revertAction           = 0;
    m_fileDeleteAction       = 0;
    m_forwardAction          = 0;
    m_backwardAction         = 0;
    m_firstAction            = 0;
    m_lastAction             = 0;
    m_undoAction             = 0;
    m_redoAction             = 0;
    m_fullScreen             = false;
    m_rotatedOrFlipped       = false;
    m_setExifOrientationTag  = true;
    m_cancelSlideShow        = false;

    // Settings containers instance.

    d->ICCSettings      = new ICCSettingsContainer();
    d->exposureSettings = new ExposureSettingsContainer();
    m_IOFileSettings    = new IOFileSettingsContainer();
    m_savingContext     = new SavingContextContainer();
    d->waitingLoop      = new QEventLoop(this);
}

EditorWindow::~EditorWindow()
{
    delete m_canvas;
    delete m_IOFileSettings;
    delete m_savingContext;
    delete d->ICCSettings;
    delete d->exposureSettings;
    delete d;
}

void EditorWindow::setupContextMenu()
{
    m_contextMenu         = new DPopupMenu(this);
    KActionCollection *ac = actionCollection();
    if (ac->action("editorwindow_backward")) 
        m_contextMenu->addAction(ac->action("editorwindow_backward"));
    if (ac->action("editorwindow_forward")) 
        m_contextMenu->addAction(ac->action("editorwindow_forward"));
    m_contextMenu->addSeparator();
    if (ac->action("editorwindow_slideshow")) 
        m_contextMenu->addAction(ac->action("editorwindow_slideshow"));
    if (ac->action("editorwindow_rotate_left"))
        m_contextMenu->addAction(ac->action("editorwindow_rotate_left"));
    if (ac->action("editorwindow_rotate_right"))
        m_contextMenu->addAction(ac->action("editorwindow_rotate_right"));
    if (ac->action("editorwindow_crop")) 
        m_contextMenu->addAction(ac->action("editorwindow_crop"));
    m_contextMenu->addSeparator();
    if (ac->action("editorwindow_delete")) 
        m_contextMenu->addAction(ac->action("editorwindow_delete"));
}

void EditorWindow::setupStandardConnections()
{
    // -- Canvas connections ------------------------------------------------

    connect(m_canvas, SIGNAL(signalToggleOffFitToWindow()),
            this, SLOT(slotToggleOffFitToWindow()));

    connect(m_canvas, SIGNAL(signalShowNextImage()),
            this, SLOT(slotForward()));

    connect(m_canvas, SIGNAL(signalShowPrevImage()),
            this, SLOT(slotBackward()));

    connect(m_canvas, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));

    connect(m_canvas, SIGNAL(signalZoomChanged(double)),
            this, SLOT(slotZoomChanged(double)));

    connect(m_canvas, SIGNAL(signalChanged()),
            this, SLOT(slotChanged()));

    connect(m_canvas, SIGNAL(signalUndoStateChanged(bool, bool, bool)),
            this, SLOT(slotUndoStateChanged(bool, bool, bool)));

    connect(m_canvas, SIGNAL(signalSelected(bool)),
            this, SLOT(slotSelected(bool)));

    connect(m_canvas, SIGNAL(signalLoadingStarted(const QString &)),
            this, SLOT(slotLoadingStarted(const QString &)));

    connect(m_canvas, SIGNAL(signalLoadingFinished(const QString &, bool)),
            this, SLOT(slotLoadingFinished(const QString &, bool)));

    connect(m_canvas, SIGNAL(signalLoadingProgress(const QString &, float)),
            this, SLOT(slotLoadingProgress(const QString &, float)));

    connect(m_canvas, SIGNAL(signalSavingStarted(const QString&)),
            this, SLOT(slotSavingStarted(const QString&)));

    connect(m_canvas, SIGNAL(signalSavingFinished(const QString&, bool)),
            this, SLOT(slotSavingFinished(const QString&, bool)));

    connect(m_canvas, SIGNAL(signalSavingProgress(const QString&, float)),
            this, SLOT(slotSavingProgress(const QString&, float)));

    connect(m_canvas, SIGNAL(signalSelectionChanged(const QRect&)),
            this, SLOT(slotSelectionChanged(const QRect&)));

    // -- if rotating/flipping set the rotatedflipped flag to true -----------

    connect(d->rotateLeftAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));

    connect(d->rotateRightAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));

    connect(d->flipHorizAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));

    connect(d->flipVertAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));

    // -- status bar connections --------------------------------------

    connect(m_nameLabel, SIGNAL(signalCancelButtonPressed()),
            this, SLOT(slotNameLabelCancelButtonPressed()));

    // -- Core plugin connections -------------------------------------

    ImagePlugin *corePlugin = m_imagePluginLoader->pluginInstance("digikamimageplugin_core");
    if ( corePlugin )
    {
        connect(m_canvas, SIGNAL(signalColorManagementTool()),
                corePlugin, SLOT(slotColorManagement()));
    }
}

void EditorWindow::setupStandardActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    m_backwardAction = actionCollection()->addAction(KStandardAction::Back, "editorwindow_backward", 
                                                     this, SLOT(slotBackward()));

    m_forwardAction = actionCollection()->addAction(KStandardAction::Forward, "editorwindow_forward", 
                                                    this, SLOT(slotForward()));

    m_firstAction = new KAction(KIcon("go-first"), i18n("&First"), this);
    m_firstAction->setShortcut(KStandardShortcut::Home);
    connect(m_firstAction, SIGNAL(triggered()), this, SLOT(slotFirst()));
    actionCollection()->addAction("editorwindow_first", m_firstAction);

    m_lastAction = new KAction(KIcon("go-last"), i18n("&Last"), this);
    m_lastAction->setShortcut(KStandardShortcut::End);
    connect(m_lastAction, SIGNAL(triggered()), this, SLOT(slotLast()));
    actionCollection()->addAction("editorwindow_last", m_lastAction);

    m_saveAction = actionCollection()->addAction(KStandardAction::Save, "editorwindow_save", 
                                                 this, SLOT(slotSave()));

    m_saveAsAction = actionCollection()->addAction(KStandardAction::SaveAs, "editorwindow_saveas", 
                                                   this, SLOT(slotSaveAs()));

    m_revertAction = actionCollection()->addAction(KStandardAction::Revert, "editorwindow_revert", 
                                                   m_canvas, SLOT(slotRestore()));

    m_saveAction->setEnabled(false);
    m_saveAsAction->setEnabled(false);
    m_revertAction->setEnabled(false);

    d->filePrintAction = new KAction(KIcon("print-frame"), i18n("Print Image..."), this);
    d->filePrintAction->setShortcut(Qt::CTRL+Qt::Key_P);
    connect(d->filePrintAction, SIGNAL(triggered()), this, SLOT(slotFilePrint()));
    actionCollection()->addAction("editorwindow_print", d->filePrintAction);

    m_fileDeleteAction = new KAction(KIcon("edit-trash"), i18n("Move to Trash"), this);
    m_fileDeleteAction->setShortcut(Qt::Key_Delete);
    connect(m_fileDeleteAction, SIGNAL(triggered()), this, SLOT(slotDeleteCurrentItem()));
    actionCollection()->addAction("editorwindow_delete", m_fileDeleteAction);


    actionCollection()->addAction(KStandardAction::Close, "editorwindow_close", 
                                  this, SLOT(close()));

    // -- Standard 'Edit' menu actions ---------------------------------------------

    d->copyAction = actionCollection()->addAction(KStandardAction::Copy, "editorwindow_copy", 
                                                  m_canvas, SLOT(slotCopy()));
    d->copyAction->setEnabled(false);

    m_undoAction = new KToolBarPopupAction(KIcon("edit-undo"), i18n("Undo"), this);
    m_undoAction->setShortcut(KStandardShortcut::Undo);
    connect(m_undoAction, SIGNAL(triggered()), m_canvas, SLOT(slotUndo()));
    actionCollection()->addAction("editorwindow_undo", m_undoAction);

    connect(m_undoAction->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowUndoMenu()));

    // TODO: KDE4PORT: this activated(int) have been replaced by triggered(QAction *)
    connect(m_undoAction->menu(), SIGNAL(activated(int)),
            m_canvas, SLOT(slotUndo(int)));

    m_undoAction->setEnabled(false);

    m_redoAction = new KToolBarPopupAction(KIcon("edit-redo"), i18n("Redo"), this);
    m_redoAction->setShortcut(KStandardShortcut::Redo);
    connect(m_redoAction, SIGNAL(triggered()), m_canvas, SLOT(slotRedo()));
    actionCollection()->addAction("editorwindow_redo", m_redoAction);

    connect(m_redoAction->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowRedoMenu()));

    // TODO: KDE4PORT: this activated(int) have been replaced by triggered(QAction *)
    connect(m_redoAction->menu(), SIGNAL(activated(int)),
            m_canvas, SLOT(slotRedo(int)));

    m_redoAction->setEnabled(false);

    d->selectAllAction = new KAction(i18n("Select All"), this);
    d->selectAllAction->setShortcut(Qt::CTRL+Qt::Key_A);
    connect(d->selectAllAction, SIGNAL(triggered()), m_canvas, SLOT(slotSelectAll()));
    actionCollection()->addAction("editorwindow_selectAll", d->selectAllAction);

    d->selectNoneAction = new KAction(i18n("Select None"), this);
    d->selectNoneAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_A);
    connect(d->selectNoneAction, SIGNAL(triggered()), m_canvas, SLOT(slotSelectNone()));
    actionCollection()->addAction("editorwindow_selectNone", d->selectNoneAction);

    // -- Standard 'View' menu actions ---------------------------------------------

    d->zoomPlusAction = actionCollection()->addAction(KStandardAction::ZoomIn, "editorwindow_zoomplus", 
                                                 this, SLOT(slotIncreaseZoom()));

    d->zoomMinusAction = actionCollection()->addAction(KStandardAction::ZoomOut, "editorwindow_zoomminus", 
                                                 this, SLOT(slotDecreaseZoom()));

    d->zoomTo100percents = new KAction(KIcon("viewmag1"), i18n("Zoom to 1:1"), this);
    d->zoomTo100percents->setShortcut(Qt::ALT+Qt::CTRL+Qt::Key_0);       // NOTE: Photoshop 7 use ALT+CTRL+0
    connect(d->zoomTo100percents, SIGNAL(triggered()), this, SLOT(slotZoomTo100Percents()));
    actionCollection()->addAction("editorwindow_zoomto100percents", d->zoomTo100percents);

    d->zoomFitToWindowAction = new KToggleAction(KIcon("zoom-best-fit"), i18n("Fit to &Window"), this);
    d->zoomFitToWindowAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_E); // NOTE: Gimp 2 use CTRL+SHIFT+E.
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), this, SLOT(slotToggleFitToWindow()));
    actionCollection()->addAction("editorwindow_zoomfit2window", d->zoomFitToWindowAction);

    d->zoomFitToSelectAction = new KAction(KIcon("viewmagfit"), i18n("Fit to &Selection"), this);
    d->zoomFitToSelectAction->setShortcut(Qt::ALT+Qt::CTRL+Qt::Key_S);   // NOTE: Photoshop 7 use ALT+CTRL+0
    connect(d->zoomFitToSelectAction, SIGNAL(triggered()), this, SLOT(slotFitToSelect()));
    actionCollection()->addAction("editorwindow_zoomfit2select", d->zoomFitToSelectAction);
    d->zoomFitToSelectAction->setEnabled(false);
    d->zoomFitToSelectAction->setWhatsThis(i18n("This option can be used to zoom the image to the "
                                                "current selection area."));

    d->zoomCombo = new KComboBox(true);
    d->zoomCombo->setDuplicatesEnabled(false);
    d->zoomCombo->setFocusPolicy(Qt::ClickFocus);
    d->zoomCombo->setInsertPolicy(QComboBox::NoInsert);
    d->zoomCombo->insertItem(-1, QString("10%"));
    d->zoomCombo->insertItem(-1, QString("25%"));
    d->zoomCombo->insertItem(-1, QString("50%"));
    d->zoomCombo->insertItem(-1, QString("75%"));
    d->zoomCombo->insertItem(-1, QString("100%"));
    d->zoomCombo->insertItem(-1, QString("150%"));
    d->zoomCombo->insertItem(-1, QString("200%"));
    d->zoomCombo->insertItem(-1, QString("300%"));
    d->zoomCombo->insertItem(-1, QString("450%"));
    d->zoomCombo->insertItem(-1, QString("600%"));
    d->zoomCombo->insertItem(-1, QString("800%"));
    d->zoomCombo->insertItem(-1, QString("1200%"));

    connect(d->zoomCombo, SIGNAL(activated(int)),
            this, SLOT(slotZoomSelected()) );

    connect(d->zoomCombo, SIGNAL(returnPressed(const QString&)),
            this, SLOT(slotZoomTextChanged(const QString &)) );

    d->zoomComboAction = new QWidgetAction(this);
    d->zoomComboAction->setDefaultWidget(d->zoomCombo);
    d->zoomComboAction->setText(i18n("Zoom"));
    actionCollection()->addAction("editorwindow_zoomto", d->zoomComboAction);

    m_fullScreenAction = actionCollection()->addAction(KStandardAction::FullScreen,
                         "editorwindow_fullscreen", this, SLOT(slotToggleFullScreen()));

    d->slideShowAction = new KAction(KIcon("datashow"), i18n("Slide Show"), this);
    d->slideShowAction->setShortcut(Qt::Key_F9);
    connect(d->slideShowAction, SIGNAL(triggered()), this, SLOT(slotToggleSlideShow()));
    actionCollection()->addAction("editorwindow_slideshow", d->slideShowAction);

    d->viewUnderExpoAction = new KToggleAction(KIcon("underexposure"), 
                                               i18n("Under-Exposure Indicator"), this);
    d->viewUnderExpoAction->setShortcut(Qt::Key_F10); 
    connect(d->viewUnderExpoAction, SIGNAL(triggered()), this, SLOT(slotToggleUnderExposureIndicator()));
    actionCollection()->addAction("editorwindow_underexposure", d->viewUnderExpoAction);


    d->viewOverExpoAction = new KToggleAction(KIcon("overexposure"), 
                                              i18n("Over-Exposure Indicator"), this);
    d->viewOverExpoAction->setShortcut(Qt::Key_F11); 
    connect(d->viewOverExpoAction, SIGNAL(triggered()), this, SLOT(slotToggleOverExposureIndicator()));
    actionCollection()->addAction("editorwindow_overexposure", d->viewOverExpoAction);

    d->viewCMViewAction = new KToggleAction(KIcon("video-display"), i18n("Color Managed View"), this);
    d->viewCMViewAction->setShortcut(Qt::Key_F12); 
    connect(d->viewCMViewAction, SIGNAL(triggered()), this, SLOT(slotToggleColorManagedView()));
    actionCollection()->addAction("editorwindow_cmview", d->viewCMViewAction);

    // -- Standard 'Transform' menu actions ---------------------------------------------

    d->resizeAction = new KAction(KIcon("resize_image"), i18n("&Resize..."), this);
    connect(d->resizeAction, SIGNAL(triggered()), this, SLOT(slotResize()));
    actionCollection()->addAction("editorwindow_resize", d->resizeAction);

    d->cropAction = new KAction(KIcon("crop"), i18n("Crop"), this);
    d->cropAction->setShortcut(Qt::CTRL+Qt::Key_X);
    connect(d->cropAction, SIGNAL(triggered()), m_canvas, SLOT(slotCrop()));
    actionCollection()->addAction("editorwindow_crop", d->cropAction);
    d->cropAction->setEnabled(false);
    d->cropAction->setWhatsThis(i18n("This option can be used to crop the image. "
                                     "Select a region of the image to enable this action."));

    // -- Standard 'Flip' menu actions ---------------------------------------------

    d->flipHorizAction = new KAction(KIcon("mirror"), i18n("Flip Horizontally"), this);
    d->flipHorizAction->setShortcut(Qt::CTRL+Qt::Key_Asterisk);
    connect(d->flipHorizAction, SIGNAL(triggered()), m_canvas, SLOT(slotFlipHoriz()));
    actionCollection()->addAction("editorwindow_flip_horiz", d->flipHorizAction);
    d->flipHorizAction->setEnabled(false);

    d->flipVertAction = new KAction(KIcon("flip"), i18n("Flip Vertically"), this);
    d->flipVertAction->setShortcut(Qt::CTRL+Qt::Key_Slash);
    connect(d->flipVertAction, SIGNAL(triggered()), m_canvas, SLOT(slotFlipVert()));
    actionCollection()->addAction("editorwindow_flip_vert", d->flipVertAction);
    d->flipVertAction->setEnabled(false);

    // -- Standard 'Rotate' menu actions ----------------------------------------

    d->rotateLeftAction = new KAction(KIcon("object-rotate-left"), i18n("Rotate Left"), this);
    d->rotateLeftAction->setShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_Left);
    connect(d->rotateLeftAction, SIGNAL(triggered()), m_canvas, SLOT(slotRotate270()));
    actionCollection()->addAction("editorwindow_rotate_left", d->rotateLeftAction);
    d->rotateLeftAction->setEnabled(false);

    d->rotateRightAction = new KAction(KIcon("object-rotate-right"), i18n("Rotate Right"), this);
    d->rotateRightAction->setShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_Right);
    connect(d->rotateRightAction, SIGNAL(triggered()), m_canvas, SLOT(slotRotate90()));
    actionCollection()->addAction("editorwindow_rotate_right", d->rotateRightAction);
    d->rotateRightAction->setEnabled(false);

    // -- Standard 'Configure' menu actions ----------------------------------------

    KStandardAction::keyBindings(this, SLOT(slotEditKeys()),           actionCollection());
    KStandardAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStandardAction::preferences(this, SLOT(slotSetup()),              actionCollection());

    // -- Standard 'Help' menu actions ---------------------------------------------

    d->donateMoneyAction = new KAction(i18n("Donate Money..."), this);
    connect(d->donateMoneyAction, SIGNAL(triggered()), this, SLOT(slotDonateMoney()));
    actionCollection()->addAction("editorwindow_donatemoney", d->donateMoneyAction);
}

void EditorWindow::setupStandardAccelerators()
{
#warning "TODO: kde4 port it";
/*  // TODO: KDE4PORT: use KAction/QAction framework instead KAccel

    d->accelerators = new KAccel(this);

    d->accelerators->insert("Exit fullscreen", i18n("Exit Fullscreen mode"),
                    i18n("Exit out of the fullscreen mode"),
                    Qt::Key_Escape, this, SLOT(slotEscapePressed()),
                    false, true);

    d->accelerators->insert("Next Image Qt::Key_Space", i18n("Next Image"),
                    i18n("Load Next Image"),
                    Qt::Key_Space, this, SLOT(slotForward()),
                    false, true);

    d->accelerators->insert("Next Image SHIFT+Qt::Key_Space", i18n("Next Image"),
                    i18n("Load Next Image"),
                    Qt::SHIFT+Qt::Key_Space, this, SLOT(slotForward()),
                    false, true);

    d->accelerators->insert("Previous Image Qt::Key_Backspace", i18n("Previous Image"),
                    i18n("Load Previous Image"),
                    Qt::Key_Backspace, this, SLOT(slotBackward()),
                    false, true);

    d->accelerators->insert("Next Image Qt::Key_Next", i18n("Next Image"),
                    i18n("Load Next Image"),
                    Qt::Key_PageDown, this, SLOT(slotForward()),
                    false, true);

    d->accelerators->insert("Previous Image Qt::Key_Prior", i18n("Previous Image"),
                    i18n("Load Previous Image"),
                    Qt::Key_PageUp, this, SLOT(slotBackward()),
                    false, true);

    d->accelerators->insert("Zoom Plus Qt::Key_Plus", i18n("Zoom In"),
                    i18n("Zoom in on Image"),
                    Qt::Key_Plus, this, SLOT(slotIncreaseZoom()),
                    false, true);

    d->accelerators->insert("Zoom Plus Qt::Key_Minus", i18n("Zoom Out"),
                    i18n("Zoom out of Image"),
                    Qt::Key_Minus, this, SLOT(slotDecreaseZoom()),
                    false, true);

    d->accelerators->insert("Redo CTRL+Qt::Key_Y", i18n("Redo"),
                    i18n("Redo Last action"),
                    Qt::CTRL+Qt::Key_Y, m_canvas, SLOT(slotRedo()),
                    false, true);
*/
}

void EditorWindow::setupStatusBar()
{
    m_nameLabel = new StatusProgressBar(statusBar());
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addWidget(m_nameLabel, 100);

    d->selectLabel = new QLabel(i18n("No selection"), statusBar());
    d->selectLabel->setAlignment(Qt::AlignCenter);
    d->selectLabel->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addWidget(d->selectLabel, 100);
    d->selectLabel->setToolTip( i18n("Information about current selection area"));

    m_resLabel  = new QLabel(statusBar());
    m_resLabel->setAlignment(Qt::AlignCenter);
    m_resLabel->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addWidget(m_resLabel, 100);
    m_resLabel->setToolTip( i18n("Information about image size"));

    QSize iconSize(fontMetrics().height()+2, fontMetrics().height()+2);
    d->underExposureIndicator = new QToolButton(statusBar());
    d->underExposureIndicator->setIcon( SmallIcon("underexposure"));
    d->underExposureIndicator->setCheckable(true);
    d->underExposureIndicator->setMaximumSize(iconSize);
    statusBar()->addPermanentWidget(d->underExposureIndicator);

    d->overExposureIndicator = new QToolButton(statusBar());
    d->overExposureIndicator->setIcon(SmallIcon("overexposure"));
    d->overExposureIndicator->setCheckable(true);
    d->overExposureIndicator->setMaximumSize(iconSize);
    statusBar()->addPermanentWidget(d->overExposureIndicator);

    d->cmViewIndicator = new QToolButton(statusBar());
    d->cmViewIndicator->setIcon(SmallIcon("video-display"));
    d->cmViewIndicator->setCheckable(true);
    d->cmViewIndicator->setMaximumSize(iconSize);
    statusBar()->addPermanentWidget(d->cmViewIndicator);

    connect(d->underExposureIndicator, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleUnderExposureIndicator()));

    connect(d->overExposureIndicator, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleOverExposureIndicator()));

    connect(d->cmViewIndicator, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleColorManagedView()));
}

void EditorWindow::printImage(KUrl url)
{
    uchar* ptr      = m_canvas->interface()->getImage();
    int w           = m_canvas->interface()->origWidth();
    int h           = m_canvas->interface()->origHeight();
    bool hasAlpha   = m_canvas->interface()->hasAlpha();
    bool sixteenBit = m_canvas->interface()->sixteenBit();

    if (!ptr || !w || !h)
        return;

    DImg image(w, h, sixteenBit, hasAlpha, ptr);

    KPrinter printer;
    QString appName = KGlobal::mainComponent().aboutData()->appName();
    printer.setDocName( url.fileName() );
    printer.setCreator( appName );
    printer.setUsePrinterResolution(true);

    KPrinter::addDialogPage( new ImageEditorPrintDialogPage(image, this, (appName.append(" page")).toAscii() ));

    if ( printer.setup( this, i18n("Print %1",printer.docName().section('/', -1)) ) )
    {
        ImagePrint printOperations(image, printer, url.fileName());
        if (!printOperations.printImageWithQt())
        {
            KMessageBox::error(this, i18n("Failed to print file: '%1'",
                               url.fileName()));
        }
    }
}

void EditorWindow::slotEditKeys()
{
    KShortcutsDialog dialog(KShortcutsEditor::AllActions,
                            KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection( actionCollection(), i18n( "General" ) );

    Q3PtrList<ImagePlugin> pluginList = ImagePluginLoader::componentData()->pluginList();

    for (ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next())
    {
        if (plugin)
        {
            dialog.addCollection(plugin->actionCollection(), plugin->objectName());
        }
    }

    dialog.configure();
}

void EditorWindow::slotResize()
{
    ImageResize dlg(this);
    dlg.exec();
}

void EditorWindow::slotAboutToShowUndoMenu()
{
    m_undoAction->menu()->clear();
    QStringList titles;
    m_canvas->getUndoHistory(titles);

    if(!titles.isEmpty())
    {
        QStringList::Iterator iter = titles.begin();
        for(; iter != titles.end(); ++iter)
        {
            m_undoAction->menu()->addAction(*iter);
        }
    }
}

void EditorWindow::slotAboutToShowRedoMenu()
{
    m_redoAction->menu()->clear();
    QStringList titles;
    m_canvas->getRedoHistory(titles);

    if(!titles.isEmpty())
    {
        QStringList::Iterator iter = titles.begin();
        for(; iter != titles.end(); ++iter)
        {
            m_redoAction->menu()->addAction(*iter);
        }
    }
}

void EditorWindow::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config()->group("ImageViewer Settings"));
    KEditToolBar dlg(factory(), this);

    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(slotNewToolbarConfig()));

    dlg.exec();
}

void EditorWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config()->group("ImageViewer Settings"));
}

void EditorWindow::slotIncreaseZoom()
{
    m_canvas->slotIncreaseZoom();
}

void EditorWindow::slotDecreaseZoom()
{
    m_canvas->slotDecreaseZoom();
}

void EditorWindow::slotToggleFitToWindow()
{
    d->zoomPlusAction->setEnabled(true);
    d->zoomComboAction->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_canvas->toggleFitToWindow();
}

void EditorWindow::slotFitToSelect()
{
    d->zoomPlusAction->setEnabled(true);
    d->zoomComboAction->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_canvas->fitToSelect();
}

void EditorWindow::slotZoomTo100Percents()
{
    d->zoomPlusAction->setEnabled(true);
    d->zoomComboAction->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_canvas->setZoomFactor(1.0);
}

void EditorWindow::slotZoomSelected()
{
    QString txt = d->zoomCombo->currentText();
    txt = txt.left(txt.indexOf('%'));
    slotZoomTextChanged(txt);
}

void EditorWindow::slotZoomTextChanged(const QString &txt)
{
    bool r      = false;
    double zoom = KGlobal::locale()->readNumber(txt, &r) / 100.0;
    if (r && zoom > 0.0)
        m_canvas->setZoomFactor(zoom);
}

void EditorWindow::slotZoomChanged(double zoom)
{
    d->zoomPlusAction->setEnabled(!m_canvas->maxZoom());
    d->zoomMinusAction->setEnabled(!m_canvas->minZoom());

    d->zoomCombo->blockSignals(true);
    d->zoomCombo->setItemText(d->zoomCombo->currentIndex(), 
                              QString::number(lround(zoom*100.0)) + QString("%"));
    d->zoomCombo->blockSignals(false);
}

void EditorWindow::slotToggleOffFitToWindow()
{
    d->zoomFitToWindowAction->blockSignals(true);
    d->zoomFitToWindowAction->setChecked(false);
    d->zoomFitToWindowAction->blockSignals(false);
}

void EditorWindow::slotEscapePressed()
{
    if (m_fullScreen)
        m_fullScreenAction->activate(QAction::Trigger);
}

void EditorWindow::plugActionAccel(KAction* action)
{
    if (!action)
        return;

#warning "TODO: kde4 port it";
/*  // TODO: KDE4PORT: use KAction/QAction framework instead KAccel

    d->accelerators->insert(action->text(),
                    action->text(),
                    action->whatsThis(),
                    action->shortcut(),
                    action,
                    SLOT(activate()));*/
}

void EditorWindow::unplugActionAccel(KAction* /*action*/)
{
#warning "TODO: kde4 port it";
/*  // TODO: KDE4PORT: use KAction/QAction framework instead KAccel

    d->accelerators->remove(action->text());*/
}

void EditorWindow::loadImagePlugins()
{
    Q3PtrList<ImagePlugin> pluginList = m_imagePluginLoader->pluginList();

    for (ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next())
    {
        if (plugin)
        {
            guiFactory()->addClient(plugin);
            plugin->setParentWidget(this);
            plugin->setEnabledSelectionActions(false);
        }
        else
            DDebug() << "Invalid plugin to add!" << endl;
    }
}

void EditorWindow::unLoadImagePlugins()
{
    Q3PtrList<ImagePlugin> pluginList = m_imagePluginLoader->pluginList();

    for (ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next())
    {
        if (plugin) 
        {
            guiFactory()->removeClient(plugin);
            plugin->setParentWidget(0);
            plugin->setEnabledSelectionActions(false);
        }
    }
}

void EditorWindow::readStandardSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("ImageViewer Settings");

    // Restore full screen Mode ?

    if (group.readEntry("FullScreen", false))
    {
        m_fullScreenAction->activate(QAction::Trigger);
        m_fullScreen = true;
    }

    // Restore Auto zoom action ?
    bool autoZoom = group.readEntry("AutoZoom", true);
    if (autoZoom)
        d->zoomFitToWindowAction->activate(QAction::Trigger);
}

void EditorWindow::applyStandardSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();

    // -- Settings for Color Management stuff ----------------------------------------------

    KConfigGroup group = config->group("Color Management");

    d->ICCSettings->enableCMSetting       = group.readEntry("EnableCM", false);
    d->ICCSettings->askOrApplySetting     = group.readEntry("BehaviourICC", false);
    d->ICCSettings->BPCSetting            = group.readEntry("BPCAlgorithm",false);
    d->ICCSettings->managedViewSetting    = group.readEntry("ManagedView", false);
    d->ICCSettings->renderingSetting      = group.readEntry("RenderingIntent", 0);
    d->ICCSettings->inputSetting          = group.readEntry("InProfileFile", QString());
    d->ICCSettings->workspaceSetting      = group.readEntry("WorkProfileFile", QString());
    d->ICCSettings->monitorSetting        = group.readEntry("MonitorProfileFile", QString());
    d->ICCSettings->proofSetting          = group.readEntry("ProofProfileFile", QString());
    d->ICCSettings->CMInRawLoadingSetting = group.readEntry("CMInRawLoading", false);

    d->viewCMViewAction->setEnabled(d->ICCSettings->enableCMSetting);
    d->viewCMViewAction->setChecked(d->ICCSettings->managedViewSetting);
    d->cmViewIndicator->setEnabled(d->ICCSettings->enableCMSetting);
    d->cmViewIndicator->setChecked(d->ICCSettings->managedViewSetting);
    setColorManagedViewIndicatorToolTip(d->ICCSettings->enableCMSetting, d->ICCSettings->managedViewSetting);
    m_canvas->setICCSettings(d->ICCSettings);

    // -- JPEG, PNG, TIFF JPEG2000 files format settings --------------------------------------

    group = config->group("ImageViewer Settings");

    // JPEG quality slider settings : 1 - 100 ==> libjpeg settings : 25 - 100.
    m_IOFileSettings->JPEGCompression     = (int)((75.0/100.0)*
                                                 (float)group.readEntry("JPEGCompression", 75)
                                                 + 26.0 - (75.0/100.0));

    // PNG compression slider settings : 1 - 9 ==> libpng settings : 100 - 1.
    m_IOFileSettings->PNGCompression      = (int)(((1.0-100.0)/8.0)*
                                                 (float)group.readEntry("PNGCompression", 1)
                                                 + 100.0 - ((1.0-100.0)/8.0));

    // TIFF compression setting.
    m_IOFileSettings->TIFFCompression     = group.readEntry("TIFFCompression", false);

    // JPEG2000 quality slider settings : 1 - 100
    m_IOFileSettings->JPEG2000Compression = group.readEntry("JPEG2000Compression", 100);

    // JPEG2000 LossLess setting.
    m_IOFileSettings->JPEG2000LossLess    = group.readEntry("JPEG2000LossLess", true);

    // -- RAW pictures decoding settings ------------------------------------------------------

    // If digiKam Color Management is enable, no need to correct color of decoded RAW image,
    // else, sRGB color workspace will be used.

    if (d->ICCSettings->enableCMSetting) 
        m_IOFileSettings->rawDecodingSettings.outputColorSpace = KDcrawIface::RawDecodingSettings::RAWCOLOR;
    else
        m_IOFileSettings->rawDecodingSettings.outputColorSpace = KDcrawIface::RawDecodingSettings::SRGB;

    m_IOFileSettings->rawDecodingSettings.sixteenBitsImage        = group.readEntry("SixteenBitsImage", false);
    m_IOFileSettings->rawDecodingSettings.automaticColorBalance   = group.readEntry("AutomaticColorBalance", true);
    m_IOFileSettings->rawDecodingSettings.cameraColorBalance      = group.readEntry("CameraColorBalance", true);
    m_IOFileSettings->rawDecodingSettings.RGBInterpolate4Colors   = group.readEntry("RGBInterpolate4Colors", false);
    m_IOFileSettings->rawDecodingSettings.DontStretchPixels       = group.readEntry("DontStretchPixels", false);
    m_IOFileSettings->rawDecodingSettings.enableNoiseReduction    = group.readEntry("EnableNoiseReduction", false);
    m_IOFileSettings->rawDecodingSettings.unclipColors            = group.readEntry("UnclipColors", 0);
    m_IOFileSettings->rawDecodingSettings.RAWQuality = (KDcrawIface::RawDecodingSettings::DecodingQuality)
                                                       group.readEntry("RAWQuality",
                                                       (int)KDcrawIface::RawDecodingSettings::BILINEAR);
    m_IOFileSettings->rawDecodingSettings.NRThreshold             = group.readEntry("NRThreshold", 100);
    m_IOFileSettings->rawDecodingSettings.brightness              = group.readEntry("RAWBrightness", 1.0);

    // -- GUI Settings -------------------------------------------------------

    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    rightSzPolicy.setHorizontalStretch(2);
    rightSzPolicy.setVerticalStretch(1);
    QList<int> list;
    if(config->hasKey("Splitter Sizes"))
        m_splitter->setSizes(group.readEntry("Splitter Sizes", list));
    else 
        m_canvas->setSizePolicy(rightSzPolicy);

    d->fullScreenHideToolBar = group.readEntry("FullScreen Hide ToolBar", false);

    // -- Exposure Indicators Settings --------------------------------------- 

    QColor black(Qt::black);
    QColor white(Qt::white);
    d->exposureSettings->underExposureIndicator = group.readEntry("UnderExposureIndicator", false);
    d->exposureSettings->overExposureIndicator  = group.readEntry("OverExposureIndicator", false);
    d->exposureSettings->underExposureColor     = group.readEntry("UnderExposureColor", white);
    d->exposureSettings->overExposureColor      = group.readEntry("OverExposureColor", black);

    d->viewUnderExpoAction->setChecked(d->exposureSettings->underExposureIndicator);
    d->viewOverExpoAction->setChecked(d->exposureSettings->overExposureIndicator);
    d->underExposureIndicator->setChecked(d->exposureSettings->underExposureIndicator);
    d->overExposureIndicator->setChecked(d->exposureSettings->overExposureIndicator);
    setUnderExposureToolTip(d->exposureSettings->underExposureIndicator);
    setOverExposureToolTip(d->exposureSettings->overExposureIndicator);
    m_canvas->setExposureSettings(d->exposureSettings);
}

void EditorWindow::saveStandardSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("ImageViewer Settings");

    group.writeEntry("AutoZoom", d->zoomFitToWindowAction->isChecked());
    group.writeEntry("Splitter Sizes", m_splitter->sizes());

    group.writeEntry("FullScreen", m_fullScreenAction->isChecked());
    group.writeEntry("UnderExposureIndicator", d->exposureSettings->underExposureIndicator);
    group.writeEntry("OverExposureIndicator", d->exposureSettings->overExposureIndicator);

    config->sync();
}

void EditorWindow::toggleStandardActions(bool val)
{
    d->zoomFitToWindowAction->setEnabled(val);
    d->rotateLeftAction->setEnabled(val);
    d->rotateRightAction->setEnabled(val);
    d->flipHorizAction->setEnabled(val);
    d->flipVertAction->setEnabled(val);
    d->filePrintAction->setEnabled(val);
    d->resizeAction->setEnabled(val);
    m_fileDeleteAction->setEnabled(val);
    m_saveAsAction->setEnabled(val);
    d->selectAllAction->setEnabled(val);
    d->selectNoneAction->setEnabled(val);

    // these actions are special: They are turned off if val is false,
    // but if val is true, they may be turned on or off.
    if (val)
    {
        // Trigger sending of signalUndoStateChanged
        // Note that for saving and loading, this is not necessary
        // because the signal will be sent later anyway.
        m_canvas->updateUndoState();
    }
    else
    {
        m_saveAction->setEnabled(val);
        m_undoAction->setEnabled(val);
        m_redoAction->setEnabled(val);
    }

    Q3PtrList<ImagePlugin> pluginList = m_imagePluginLoader->pluginList();

    for (ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next())
    {
        if (plugin) 
        {
            plugin->setEnabledActions(val);
        }
    }
}

void EditorWindow::slotToggleFullScreen()
{
    if (m_fullScreen) // out of fullscreen
    {
        m_canvas->setBackgroundColor(m_bgColor);

        setWindowState( windowState() & ~Qt::WindowFullScreen );
        menuBar()->show();
        statusBar()->show();

#warning "TODO: kde4 port it";
/* TODO: KDE4PORT: Check these methods
        leftDock()->show();
        rightDock()->show();
        topDock()->show();
        bottomDock()->show();*/

#warning "TODO: kde4 port it";
/*
        QObject* obj = child("ToolBar","KToolBar");

        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);

            if (m_fullScreenAction->isPlugged(toolBar) && d->removeFullScreenButton)
                m_fullScreenAction->unplug(toolBar);

            if (toolBar->isHidden())
                showToolBars();
        }

        // -- remove the gui action accels ----

        unplugActionAccel(m_forwardAction);
        unplugActionAccel(m_backwardAction);
        unplugActionAccel(m_firstAction);
        unplugActionAccel(m_lastAction);
        unplugActionAccel(m_saveAction);
        unplugActionAccel(m_saveAsAction);
        unplugActionAccel(d->zoomPlusAction);
        unplugActionAccel(d->zoomMinusAction);
        unplugActionAccel(d->zoomFitToWindowAction);
        unplugActionAccel(d->zoomFitToSelectAction);
        unplugActionAccel(d->cropAction);
        unplugActionAccel(d->filePrintAction);
        unplugActionAccel(m_fileDeleteAction);
        unplugActionAccel(d->selectAllAction);
        unplugActionAccel(d->selectNoneAction);
*/
        toggleGUI2FullScreen();
        m_fullScreen = false;
    }
    else  // go to fullscreen
    {
        m_canvas->setBackgroundColor(QColor(Qt::black));

        // hide the menubar and the statusbar
        menuBar()->hide();
        statusBar()->hide();

#warning "TODO: kde4 port it";
/* TODO: KDE4PORT: Check these methods
        topDock()->hide();
        leftDock()->hide();
        rightDock()->hide();
        bottomDock()->hide();*/

#warning "TODO: kde4 port it";
/*
        QObject* obj = child("ToolBar","KToolBar");

        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);

            if (d->fullScreenHideToolBar)
            {
                hideToolBars();
            }
            else
            {   
                showToolBars();

                if ( !m_fullScreenAction->isPlugged(toolBar) )
                {
                    m_fullScreenAction->plug(toolBar);
                    d->removeFullScreenButton=true;
                }
                else    
                {
                    // If FullScreen button is enable in toolbar settings
                    // We don't remove it when we out of fullscreen mode.
                    d->removeFullScreenButton=false;
                }
            }
        }

        // -- Insert all the gui actions into the accel --

        plugActionAccel(m_forwardAction);
        plugActionAccel(m_backwardAction);
        plugActionAccel(m_firstAction);
        plugActionAccel(m_lastAction);
        plugActionAccel(m_saveAction);
        plugActionAccel(m_saveAsAction);
        plugActionAccel(d->zoomPlusAction);
        plugActionAccel(d->zoomMinusAction);
        plugActionAccel(d->zoomFitToWindowAction);
        plugActionAccel(d->zoomFitToSelectAction);
        plugActionAccel(d->cropAction);
        plugActionAccel(d->filePrintAction);
        plugActionAccel(m_fileDeleteAction);
        plugActionAccel(d->selectAllAction);
        plugActionAccel(d->selectNoneAction);
*/
        toggleGUI2FullScreen();
        showFullScreen();
        m_fullScreen = true;
    }
}

void EditorWindow::slotRotatedOrFlipped()
{
    m_rotatedOrFlipped = true;
}

void EditorWindow::slotLoadingProgress(const QString&, float progress)
{
    m_nameLabel->setProgressValue((int)(progress*100.0));
}

void EditorWindow::slotSavingProgress(const QString&, float progress)
{
    m_nameLabel->setProgressValue((int)(progress*100.0));
}

bool EditorWindow::promptUserSave(const KUrl& url)
{
    if (m_saveAction->isEnabled())
    {
        // if window is iconified, show it
        if (isMinimized())
        {
            KWindowSystem::unminimizeWindow(winId());
        }

        int result = KMessageBox::warningYesNoCancel(this,
                                  i18n("The image '%1' has been modified.\n"
                                       "Do you want to save it?",
                                       url.fileName()),
                                  QString(),
                                  KStandardGuiItem::save(),
                                  KStandardGuiItem::discard());

        if (result == KMessageBox::Yes)
        {
            bool saving;

            if (m_canvas->isReadOnly())
                saving = saveAs();
            else
                saving = save();

            // save and saveAs return false if they were cancelled and did not enter saving at all
            // In this case, do not call enterWaitingLoop because quitWaitingloop will not be called.
            if (saving)
            {
                // Waiting for asynchronous image file saving operation runing in separate thread.
                m_savingContext->synchronizingState = SavingContextContainer::SynchronousSaving;
                enterWaitingLoop();
                m_savingContext->synchronizingState = SavingContextContainer::NormalSaving;
                return m_savingContext->synchronousSavingResult;
            }
            else
            {
                return false;
            }
        }
        else if (result == KMessageBox::No)
        {
            m_saveAction->setEnabled(false);
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool EditorWindow::waitForSavingToComplete()
{
    // avoid reentrancy - return false means we have reentered the loop already.
    if (m_savingContext->synchronizingState == SavingContextContainer::SynchronousSaving)
        return false;

    if (m_savingContext->savingState != SavingContextContainer::SavingStateNone)
    {
        // Waiting for asynchronous image file saving operation runing in separate thread.
        m_savingContext->synchronizingState = SavingContextContainer::SynchronousSaving;
        KMessageBox::queuedMessageBox(this,
                                      KMessageBox::Information,
                                      i18n("Please wait while the image is being saved..."));
        enterWaitingLoop();
        m_savingContext->synchronizingState = SavingContextContainer::NormalSaving;
    }
    return true;
}

void EditorWindow::enterWaitingLoop()
{
    d->waitingLoop->exec(QEventLoop::ExcludeUserInputEvents);
}

void EditorWindow::quitWaitingLoop()
{
    d->waitingLoop->quit();
}

void EditorWindow::slotSelected(bool val)
{
    // Update menu actions.
    d->cropAction->setEnabled(val);
    d->zoomFitToSelectAction->setEnabled(val);
    d->copyAction->setEnabled(val);

    for (ImagePlugin* plugin = m_imagePluginLoader->pluginList().first();
         plugin; plugin = m_imagePluginLoader->pluginList().next())
    {
        if (plugin) 
        {
            plugin->setEnabledSelectionActions(val);
        }
    }

    QRect sel = m_canvas->getSelectedArea();
    // Update histogram into sidebar.
    emit signalSelectionChanged(sel);

    // Update status bar
    if (val)
        d->selectLabel->setText(QString("(%1, %2) (%3 x %4)").arg(sel.x()).arg(sel.y())
                               .arg(sel.width()).arg(sel.height()));
    else 
        d->selectLabel->setText(i18n("No selection"));
}

void EditorWindow::hideToolBars()
{
#warning "TODO: kde4 port it";
/*
    Q3PtrListIterator<KToolBar> it = toolBarIterator();
    KToolBar* bar;

    for(;it.current()!=0L; ++it)
    {
        bar=it.current();

        if (bar->area()) 
            bar->area()->hide();
        else 
            bar->hide();
    }
*/
}

void EditorWindow::showToolBars()
{
#warning "TODO: kde4 port it";
/*
    Q3PtrListIterator<KToolBar> it = toolBarIterator();
    KToolBar* bar;

    for( ; it.current()!=0L ; ++it)
    {
        bar=it.current();

        if (bar->area())
            bar->area()->show();
        else
            bar->show();
    }
*/
}

void EditorWindow::slotLoadingStarted(const QString& /*filename*/)
{
    setCursor( Qt::WaitCursor );

    // Disable actions as appropriate during loading
    emit signalNoCurrentItem();
    toggleActions(false);

    m_nameLabel->progressBarMode(StatusProgressBar::ProgressBarMode, i18n("Loading: "));
}

void EditorWindow::slotLoadingFinished(const QString& filename, bool success)
{
    m_nameLabel->progressBarMode(StatusProgressBar::TextMode);
    slotUpdateItemInfo();

    // Enable actions as appropriate after loading
    // No need to re-enable image properties sidebar here, it's will be done
    // automatically by a signal from canvas
    toggleActions(success);
    unsetCursor();

    // Note: in showfoto, we using a null filename to clear canvas.
    if (!success && filename != QString())
    {
        QFileInfo fi(filename);
        QString message = i18n("Failed to load image \"%1\"",fi.fileName());
        KMessageBox::error(this, message);
        DWarning() << "Failed to load image " << fi.fileName() << endl;
    }
}

void EditorWindow::slotNameLabelCancelButtonPressed()
{
    // If we saving a picture...
    if (m_savingContext->savingState != SavingContextContainer::SavingStateNone)
    {
        m_savingContext->abortingSaving = true;
        m_canvas->abortSaving();
    }

    // If we preparing SlideShow...
    m_cancelSlideShow = true;
}

void EditorWindow::slotSave()
{
    if (m_canvas->isReadOnly())
        saveAs();
    else
        save();
}
void EditorWindow::slotSavingStarted(const QString& /*filename*/)
{
    setCursor( Qt::WaitCursor );

    // Disable actions as appropriate during saving
    emit signalNoCurrentItem();
    toggleActions(false);

    m_nameLabel->progressBarMode(StatusProgressBar::CancelProgressBarMode, i18n("Saving: "));
}

void EditorWindow::slotSavingFinished(const QString& filename, bool success)
{
    if (m_savingContext->savingState == SavingContextContainer::SavingStateSave)
    {
        // from save()
        m_savingContext->savingState = SavingContextContainer::SavingStateNone;

        if (!success)
        {
            if (!m_savingContext->abortingSaving)
            {
                KMessageBox::error(this, i18n("Failed to save file\n\"%1\"\nto\n\"%2\".",
                                              m_savingContext->destinationURL.fileName(),
                                              m_savingContext->destinationURL.path()));
            }
            finishSaving(false);
            return;
        }

        DDebug() << "renaming to " << m_savingContext->destinationURL.path() << endl;

        if (!moveFile())
        {
            finishSaving(false);
            return;
        }

        m_canvas->setUndoHistoryOrigin();

        // remove image from cache since it has changed
        LoadingCacheInterface::cleanFromCache(m_savingContext->destinationURL.path());
        // this won't be in the cache, but does not hurt to do it anyway
        LoadingCacheInterface::cleanFromCache(filename);

        // restore state of disabled actions. saveIsComplete can start any other task
        // (loading!) which might itself in turn change states
        finishSaving(true);

        saveIsComplete();

        // Take all actions necessary to update information and re-enable sidebar
        slotChanged();
    }
    else if (m_savingContext->savingState == SavingContextContainer::SavingStateSaveAs)
    {
        m_savingContext->savingState = SavingContextContainer::SavingStateNone;

        // from saveAs()
        if (!success)
        {
            if (!m_savingContext->abortingSaving)
            {
                KMessageBox::error(this, i18n("Failed to save file\n\"%1\"\nto\n\"%2\".",
                                              m_savingContext->destinationURL.fileName(),
                                              m_savingContext->destinationURL.path()));
            }
            finishSaving(false);
            return;
        }

        // Only try to write exif if both src and destination are jpeg files

        DDebug() << "renaming to " << m_savingContext->destinationURL.path() << endl;

        if (!moveFile())
        {
            finishSaving(false);
            return;
        }

        m_canvas->setUndoHistoryOrigin();

        LoadingCacheInterface::cleanFromCache(m_savingContext->destinationURL.path());
        LoadingCacheInterface::cleanFromCache(filename);

        finishSaving(true);
        saveAsIsComplete();

        // Take all actions necessary to update information and re-enable sidebar
        slotChanged();
    }
}

void EditorWindow::finishSaving(bool success)
{
    m_savingContext->synchronousSavingResult = success;

    if (m_savingContext->saveTempFile)
    {
        delete m_savingContext->saveTempFile;
        m_savingContext->saveTempFile = 0;
    }

    // Exit of internal Qt event loop to unlock promptUserSave() method.
    if (m_savingContext->synchronizingState == SavingContextContainer::SynchronousSaving)
        quitWaitingLoop();

    // Enable actions as appropriate after saving
    toggleActions(true);
    unsetCursor();

    m_nameLabel->progressBarMode(StatusProgressBar::TextMode);

    // On error, continue using current image
    if (!success)
    {
        m_canvas->switchToLastSaved(m_savingContext->srcURL.path());
    }
}

void EditorWindow::startingSave(const KUrl& url)
{
    // avoid any reentrancy. Should be impossible anyway since actions will be disabled.
    if (m_savingContext->savingState != SavingContextContainer::SavingStateNone)
        return;

    if (!checkPermissions(url))
        return;

    m_savingContext->srcURL             = url;
    m_savingContext->destinationURL     = m_savingContext->srcURL;
    m_savingContext->destinationExisted = true;
    m_savingContext->originalFormat     = m_canvas->currentImageFileFormat();
    m_savingContext->format             = m_savingContext->originalFormat;
    m_savingContext->abortingSaving     = false;
    m_savingContext->savingState        = SavingContextContainer::SavingStateSave;
    // use magic file extension which tells the digikamalbums ioslave to ignore the file
    m_savingContext->saveTempFile       = new KTemporaryFile();
    m_savingContext->saveTempFile->setPrefix(m_savingContext->srcURL.directory(false));
    m_savingContext->saveTempFile->setSuffix(".digikamtempfile.tmp");
    m_savingContext->saveTempFile->setAutoRemove(true);

    m_canvas->saveAs(m_savingContext->saveTempFile->fileName(), m_IOFileSettings,
                     m_setExifOrientationTag && (m_rotatedOrFlipped || m_canvas->exifRotated()));
}

bool EditorWindow::startingSaveAs(const KUrl& url)
{
    if (m_savingContext->savingState != SavingContextContainer::SavingStateNone)
        return false;

    QStringList writableMimetypes = KImageIO::mimeTypes(KImageIO::Writing);
    // Put first class citizens at first place
    writableMimetypes.removeAll("image/jpeg");
    writableMimetypes.removeAll("image/tiff");
    writableMimetypes.removeAll("image/png");
    writableMimetypes.removeAll("image/jpeg2000");
    writableMimetypes.insert(0, "image/png");
    writableMimetypes.insert(1, "image/jpeg");
    writableMimetypes.insert(2, "image/tiff");
    writableMimetypes.insert(3, "image/jpeg2000");

    DDebug () << "startingSaveAs: Offered mimetypes: " << writableMimetypes << endl;

    // Determine the default mime type.
    // Determine mime type from image format of the src image
    QString defaultMimeType;
    QString originalFormat = m_canvas->currentImageFileFormat().toLower();
    // inspired by kimageio.cpp, typeForMime(). This here is "mimeForType".
    KService::List services = KServiceTypeTrader::self()->query("QImageIOPlugins");
    KService::Ptr service;
    foreach(service, services) {
        if (service->property("X-KDE-ImageFormat").toStringList().contains(originalFormat))
        {
            defaultMimeType = service->property("X-KDE-MimeType").toString();
            break;
        }
    }
    // default to PNG
    if (defaultMimeType.isEmpty())
        defaultMimeType = "image/png";

    m_savingContext->srcURL = url;

    FileSaveOptionsBox *options = new FileSaveOptionsBox();
    KFileDialog imageFileSaveDialog(m_savingContext->srcURL.isLocalFile() ? 
                                    m_savingContext->srcURL : KUrl(QDir::homePath()),
                                    QString(),
                                    this,
                                    options);

    imageFileSaveDialog.setModal(false);
    imageFileSaveDialog.setOperationMode(KFileDialog::Saving);
    imageFileSaveDialog.setMode(KFile::File);
    imageFileSaveDialog.setSelection(m_savingContext->srcURL.fileName());
    imageFileSaveDialog.setCaption(i18n("New Image File Name"));
    imageFileSaveDialog.setMimeFilter(writableMimetypes, defaultMimeType);

    connect(&imageFileSaveDialog, SIGNAL(filterChanged(const QString &)),
            options, SLOT(slotImageFileFormatChanged(const QString &)));

    connect(&imageFileSaveDialog, SIGNAL(fileSelected(const QString &)),
            options, SLOT(slotImageFileSelected(const QString &)));

    options->slotImageFileSelected(m_savingContext->srcURL.path());

    // Start dialog and check if canceled.
    if ( imageFileSaveDialog.exec() != KFileDialog::Accepted )
       return false;

    // Update file save settings in editor instance.
    options->applySettings();
    applyStandardSettings();

    KUrl newURL = imageFileSaveDialog.selectedUrl();

    // Check if target image format have been selected from Combo List of SaveAs dialog.

    QStringList mimes =KImageIO::typeForMime(imageFileSaveDialog.currentMimeFilter());
    if (!mimes.isEmpty())
    {
        m_savingContext->format = mimes.first();
    }
    else
    {
        // Else, check if target image format have been add to target image file name using extension.

        QFileInfo fi(newURL.path());
        m_savingContext->format = fi.suffix();

        if ( m_savingContext->format.isEmpty() )
        {
            // If format is empty then file format is same as that of the original file.
            m_savingContext->format = QImageReader::imageFormat(m_savingContext->srcURL.path());
        }
        else
        {
            // Else, check if format from file name extension is include on file mime type list.

            QStringList imgExtList = KImageIO::types(KImageIO::Writing);
            imgExtList << "TIF";
            imgExtList << "TIFF";
            imgExtList << "JPG";
            imgExtList << "JPE";

            if ( !imgExtList.contains( m_savingContext->format.toUpper() ) )
            {
                KMessageBox::error(this, i18n("Target image file format \"%1\" unsupported.", m_savingContext->format));
                DWarning() << k_funcinfo << "target image file format " << m_savingContext->format << " unsupported!" << endl;
                return false;
            }
        }
    }

    if (!newURL.isValid())
    {
        KMessageBox::error(this, i18n("Failed to save file\n\"%1\" to\n\"%2\".",
                                      newURL.fileName(),
                                      newURL.path().section('/', -2, -2)));
        DWarning() << k_funcinfo << "target URL is not valid !" << endl;
        return false;
    }

    // if new and original url are equal use slotSave() ------------------------------

    KUrl currURL(m_savingContext->srcURL);
    currURL.cleanPath();
    newURL.cleanPath();

    if (currURL.equals(newURL))
    {
        slotSave();
        return false;
    }

    // Check for overwrite ----------------------------------------------------------

    QFileInfo fi(newURL.path());
    m_savingContext->destinationExisted = fi.exists();
    if ( m_savingContext->destinationExisted )
    {
        int result =

            KMessageBox::warningYesNo( this, i18n("A file named \"%1\" already "
                                                  "exists. Are you sure you want "
                                                  "to overwrite it?",
                                                  newURL.fileName()),
                                       i18n("Overwrite File?"),
                                       KStandardGuiItem::overwrite(),
                                       KStandardGuiItem::cancel() );

        if (result != KMessageBox::Yes)
            return false;

        // There will be two message boxes if the file is not writable.
        // This may be controversial, and it may be changed, but it was a deliberate decision.
        if (!checkPermissions(newURL))
            return false;
    }

    // Now do the actual saving -----------------------------------------------------

    // use magic file extension which tells the digikamalbums ioslave to ignore the file

    m_savingContext->destinationURL = newURL;
    m_savingContext->originalFormat = m_canvas->currentImageFileFormat();
    m_savingContext->savingState    = SavingContextContainer::SavingStateSaveAs;
    m_savingContext->abortingSaving = false;
    m_savingContext->saveTempFile   = new KTemporaryFile();
    m_savingContext->saveTempFile->setPrefix(newURL.directory(false));
    m_savingContext->saveTempFile->setSuffix(".digikamtempfile.tmp");
    m_savingContext->saveTempFile->setAutoRemove(true);

    m_canvas->saveAs(m_savingContext->saveTempFile->fileName(), m_IOFileSettings,
                     m_setExifOrientationTag && (m_rotatedOrFlipped || m_canvas->exifRotated()),
                     m_savingContext->format.toLower());

    return true;
}

bool EditorWindow::checkPermissions(const KUrl& url)
{
    //TODO: Check that the permissions can actually be changed
    //      if write permissions are not available.

    QFileInfo fi(url.path());

    if (fi.exists() && !fi.isWritable())
    {
       int result =

            KMessageBox::warningYesNo( this, i18n("You do not have write permissions "
                                                  "for the file named \"%1\". "
                                                  "Are you sure you want "
                                                  "to overwrite it?",
                                                  url.fileName()),
                                       i18n("Overwrite File?"),
                                       KStandardGuiItem::overwrite(),
                                       KStandardGuiItem::cancel() );

        if (result != KMessageBox::Yes)
            return false;
    }

    return true;
}

bool EditorWindow::moveFile()
{
    QByteArray dstFileName = QFile::encodeName(m_savingContext->destinationURL.path());

    // store old permissions
    mode_t filePermissions = S_IREAD | S_IWRITE;
    if (m_savingContext->destinationExisted)
    {
        struct stat stbuf;
        if (::stat(dstFileName, &stbuf) == 0)
        {
            filePermissions = stbuf.st_mode;
        }
    }

    // rename tmp file to dest
    if (::rename(QFile::encodeName(m_savingContext->saveTempFile->fileName()), dstFileName) != 0)
    {
        KMessageBox::error(this, i18n("Failed to overwrite original file"),
                           i18n("Error Saving File"));
        return false;
    }

    // restore permissions
    if (m_savingContext->destinationExisted)
    {
        if (::chmod(dstFileName, filePermissions) != 0)
        {
            DWarning() << "Failed to restore file permissions for file " << dstFileName << endl;
        }
    }

    return true;
}

void EditorWindow::slotToggleColorManagedView()
{
    d->cmViewIndicator->blockSignals(true);
    d->viewCMViewAction->blockSignals(true);
    bool cmv = false;
    if (d->ICCSettings->enableCMSetting)
    {
        cmv = !d->ICCSettings->managedViewSetting;
        d->ICCSettings->managedViewSetting = cmv;
        m_canvas->setICCSettings(d->ICCSettings);

        // Save Color Managed View setting in config file. For performance 
        // reason, no need to flush file, it cached in memory and will be flushed 
        // to disk at end of session.  
        KSharedConfig::Ptr config = KGlobal::config();
        KConfigGroup group = config->group("Color Management");
        group.writeEntry("ManagedView", cmv);
    }

    d->cmViewIndicator->setChecked(cmv);
    d->viewCMViewAction->setChecked(cmv);
    setColorManagedViewIndicatorToolTip(d->ICCSettings->enableCMSetting, cmv);
    d->cmViewIndicator->blockSignals(false);
    d->viewCMViewAction->blockSignals(false);
}

void EditorWindow::setColorManagedViewIndicatorToolTip(bool available, bool cmv)
{
    QString tooltip;
    if (available)
    {
        if (cmv)
            tooltip = i18n("Color Managed View is enabled");
        else
            tooltip = i18n("Color Managed View is disabled");
    }
    else
    {
        tooltip = i18n("Color Management is not configured, so the Color Managed View is not available");
    }
    d->cmViewIndicator->setToolTip(tooltip);
}

void EditorWindow::slotToggleUnderExposureIndicator()
{
    d->underExposureIndicator->blockSignals(true);
    d->viewUnderExpoAction->blockSignals(true);
    bool uei = !d->exposureSettings->underExposureIndicator;
    d->underExposureIndicator->setChecked(uei);
    d->viewUnderExpoAction->setChecked(uei);
    d->exposureSettings->underExposureIndicator = uei;
    m_canvas->setExposureSettings(d->exposureSettings);
    setUnderExposureToolTip(uei);
    d->underExposureIndicator->blockSignals(false);
    d->viewUnderExpoAction->blockSignals(false);
}

void EditorWindow::setUnderExposureToolTip(bool uei)
{
    d->underExposureIndicator->setToolTip( 
                  uei ? i18n("Under-Exposure indicator is enabled") 
                      : i18n("Under-Exposure indicator is disabled"));
}

void EditorWindow::slotToggleOverExposureIndicator()
{
    d->overExposureIndicator->blockSignals(true);
    d->viewOverExpoAction->blockSignals(true);
    bool oei = !d->exposureSettings->overExposureIndicator;
    d->overExposureIndicator->setChecked(oei);
    d->viewOverExpoAction->setChecked(oei);
    d->exposureSettings->overExposureIndicator = oei;
    m_canvas->setExposureSettings(d->exposureSettings);
    setOverExposureToolTip(oei);
    d->overExposureIndicator->blockSignals(false);
    d->viewOverExpoAction->blockSignals(false);
}

void EditorWindow::setOverExposureToolTip(bool oei)
{
    d->overExposureIndicator->setToolTip( 
                  oei ? i18n("Over-Exposure indicator is enabled") 
                      : i18n("Over-Exposure indicator is disabled"));
}

void EditorWindow::slotDonateMoney()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=donation");
}

void EditorWindow::slotToggleSlideShow()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("ImageViewer Settings");
    bool startWithCurrent = group.readEntry("SlideShowStartCurrent", false);

    SlideShowSettings settings;
    settings.delay                = group.readEntry("SlideShowDelay", 5) * 1000;
    settings.printName            = group.readEntry("SlideShowPrintName", true);
    settings.printDate            = group.readEntry("SlideShowPrintDate", false);
    settings.printApertureFocal   = group.readEntry("SlideShowPrintApertureFocal", false);
    settings.printExpoSensitivity = group.readEntry("SlideShowPrintExpoSensitivity", false);
    settings.printMakeModel       = group.readEntry("SlideShowPrintMakeModel", false);
    settings.printComment         = group.readEntry("SlideShowPrintComment", false);
    settings.loop                 = group.readEntry("SlideShowLoop", false);
    slideShow(startWithCurrent, settings);
}

void EditorWindow::slotSelectionChanged(const QRect& sel)
{
    d->selectLabel->setText(QString("(%1, %2) (%3 x %4)").arg(sel.x()).arg(sel.y())
                           .arg(sel.width()).arg(sel.height()));
}

}  // namespace Digikam
