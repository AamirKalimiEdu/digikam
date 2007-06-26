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

#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

// Qt includes.

#include <QColor>
#include <QString>
#include <QRect>

// KDE includes.

#include <kxmlguiwindow.h>
#include <kurl.h>

// Local includes.

#include "digikam_export.h"

class Q3PopupMenu;
class QSplitter;
class QLabel;
class QAction;

class KToolBarPopupAction;
class KToggleAction;
class KAction;

namespace Digikam
{

class DPopupMenu;
class Canvas;
class ImagePluginLoader;
class IOFileSettingsContainer;
class SavingContextContainer;
class StatusProgressBar;
class SlideShowSettings;
class EditorWindowPriv;

class DIGIKAM_EXPORT EditorWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:

    EditorWindow(const char *name);
    ~EditorWindow();

    virtual void applySettings(){};
    virtual bool setup(bool iccSetupPage=false)=0;

signals:

    void signalSelectionChanged( const QRect & );
    void signalNoCurrentItem();

protected:

    bool                     m_cancelSlideShow;
    bool                     m_fullScreen;
    bool                     m_rotatedOrFlipped;
    bool                     m_setExifOrientationTag;

    QLabel                  *m_resLabel;

    QColor                   m_bgColor;

    QSplitter               *m_splitter;

    QAction                 *m_saveAction;
    QAction                 *m_saveAsAction;
    QAction                 *m_revertAction;
    QAction                 *m_fileDeleteAction;
    QAction                 *m_forwardAction;
    QAction                 *m_backwardAction;
    QAction                 *m_fullScreenAction;

    KAction                 *m_lastAction;
    KAction                 *m_firstAction;

    KToolBarPopupAction     *m_undoAction;
    KToolBarPopupAction     *m_redoAction;

    DPopupMenu              *m_contextMenu;
    Canvas                  *m_canvas;
    ImagePluginLoader       *m_imagePluginLoader;
    StatusProgressBar       *m_nameLabel;
    IOFileSettingsContainer *m_IOFileSettings;
    SavingContextContainer  *m_savingContext;

protected:

    void saveStandardSettings();
    void readStandardSettings();
    void applyStandardSettings();

    void setupStandardConnections();
    void setupStandardActions();
    void setupStandardAccelerators();
    void setupStatusBar();
    void setupContextMenu();
    void toggleStandardActions(bool val);

    void printImage(KUrl url);

    void plugActionAccel(KAction* action);
    void unplugActionAccel(KAction* action);

    void unLoadImagePlugins();
    void loadImagePlugins();

    bool promptUserSave(const KUrl& url);
    bool waitForSavingToComplete();
    void startingSave(const KUrl& url);
    bool startingSaveAs(const KUrl& url);
    bool checkPermissions(const KUrl& url);
    bool moveFile();

    virtual void finishSaving(bool success);

    virtual void readSettings()               { readStandardSettings();     };
    virtual void saveSettings()               { saveStandardSettings();     };
    virtual void toggleActions(bool val)      { toggleStandardActions(val); };
    virtual void toggleGUI2FullScreen()       {};

    virtual void slideShow(bool startWithCurrent, SlideShowSettings& settings)=0;

    virtual void setupConnections()=0;
    virtual void setupActions()=0;
    virtual void setupUserArea()=0;
    virtual bool saveAs()=0; 
    virtual bool save()=0;

    virtual void saveIsComplete()=0;
    virtual void saveAsIsComplete()=0; 

protected slots:

    void slotSave();
    void slotSaveAs() { saveAs(); };

    void slotImagePluginsHelp();
    void slotEditKeys();
    void slotResize();

    void slotAboutToShowUndoMenu();
    void slotAboutToShowRedoMenu();

    void slotConfToolbars();
    void slotNewToolbarConfig();

    void slotToggleFullScreen();
    void slotEscapePressed();

    void slotSelected(bool);

    void slotLoadingProgress(const QString& filePath, float progress);
    void slotSavingProgress(const QString& filePath, float progress);

    void slotNameLabelCancelButtonPressed();

    virtual void slotLoadingStarted(const QString& filename);
    virtual void slotLoadingFinished(const QString &filename, bool success);
    virtual void slotSavingStarted(const QString &filename);

    virtual void slotSetup(){ setup(); };

    virtual void slotFilePrint()=0;
    virtual void slotDeleteCurrentItem()=0;
    virtual void slotBackward()=0;
    virtual void slotForward()=0;
    virtual void slotFirst()=0;
    virtual void slotLast()=0;
    virtual void slotUpdateItemInfo()=0;
    virtual void slotChanged()=0;
    virtual void slotContextMenu()=0;

private slots:

    void slotToggleUnderExposureIndicator();
    void slotToggleOverExposureIndicator();
    void slotToggleColorManagedView();
    void slotRotatedOrFlipped();
    void slotSavingFinished(const QString &filename, bool success);
    void slotDonateMoney();
    void slotToggleSlideShow();
    void slotZoomTo100Percents();
    void slotZoomSelected();
    void slotZoomTextChanged(const QString &);
    void slotZoomChanged(double zoom);
    void slotSelectionChanged(const QRect& sel);
    void slotToggleFitToWindow();
    void slotToggleOffFitToWindow();
    void slotFitToSelect();
    void slotIncreaseZoom();
    void slotDecreaseZoom();

private:

    void enter_loop();
    void hideToolBars();
    void showToolBars();
    void setColorManagedViewIndicatorToolTip(bool available, bool cmv);
    void setUnderExposureToolTip(bool uei);
    void setOverExposureToolTip(bool oei);

private:

    EditorWindowPriv *d;
};

}  // namespace Digikam

#endif /* EDITORWINDOW_H */
