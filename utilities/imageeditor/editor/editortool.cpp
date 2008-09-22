/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : editor tool template class.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QWidget>
#include <QTimer>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "dimgthreadedfilter.h"
#include "imagewidget.h"
#include "imageguidewidget.h"
#include "imagepanelwidget.h"
#include "editortoolsettings.h"
#include "editortooliface.h"
#include "editortool.h"
#include "editortool.moc"

namespace Digikam
{

class EditorToolPriv
{

public:

    EditorToolPriv()
    {
        timer    = 0;
        view     = 0;
        settings = 0;
    }

    QString             helpAnchor;
    QString             name;

    QWidget            *view;

    QPixmap             icon;

    QTimer             *timer;

    EditorToolSettings *settings;
};

EditorTool::EditorTool(QObject *parent)
          : QObject(parent)
{
    d = new EditorToolPriv;
    d->timer = new QTimer(this);

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotEffect()));

    QTimer::singleShot(0, this, SLOT(slotInit()));
}

EditorTool::~EditorTool()
{
    delete d;
}

QPixmap EditorTool::toolIcon() const
{
    return d->icon;
}

void EditorTool::setToolIcon(const QPixmap& icon)
{
    d->icon = icon;
}

QString EditorTool::toolName() const
{
    return d->name;
}

void EditorTool::setToolName(const QString& name)
{
    d->name = name;
}

QWidget* EditorTool::toolView() const
{
    return d->view;
}

void EditorTool::setToolView(QWidget *view)
{
    d->view = view;
}

void EditorTool::setToolHelp(const QString& anchor)
{
    d->helpAnchor = anchor;
    // TODO: use this anchor with editor Help menu
}

QString EditorTool::toolHelp() const
{
    if (d->helpAnchor.isEmpty())
        return (objectName() + QString(".anchor"));

    return d->helpAnchor;
}

EditorToolSettings* EditorTool::toolSettings() const
{
    return d->settings;
}

void EditorTool::setToolSettings(EditorToolSettings *settings)
{
    d->settings = settings;

    connect(d->settings, SIGNAL(signalOkClicked()),
            this, SLOT(slotOk()));

    connect(d->settings, SIGNAL(signalCancelClicked()),
            this, SLOT(slotCancel()));

    connect(d->settings, SIGNAL(signalDefaultClicked()),
            this, SLOT(slotResetSettings()));

    connect(d->settings, SIGNAL(signalSaveAsClicked()),
            this, SLOT(slotSaveAsSettings()));

    connect(d->settings, SIGNAL(signalLoadClicked()),
            this, SLOT(slotLoadSettings()));

    connect(d->settings, SIGNAL(signalTryClicked()),
            this, SLOT(slotEffect()));
}

void EditorTool::setBusy(bool state)
{
    d->settings->setBusy(state);
}

void EditorTool::readSettings()
{
    d->settings->readSettings();
}

void EditorTool::writeSettings()
{
    d->settings->writeSettings();
}

void EditorTool::slotResetSettings()
{
    d->settings->resetSettings();
}

void EditorTool::slotTimer()
{
    d->timer->setSingleShot(true);
    d->timer->start(500);
}

void EditorTool::slotOk()
{
    writeSettings();
    finalRendering();
    emit okClicked();
}

void EditorTool::slotCancel()
{
    writeSettings();
    emit cancelClicked();
}

void EditorTool::slotInit()
{
    readSettings();
}

// ----------------------------------------------------------------

class EditorToolThreadedPriv
{
public:
    enum RunningMode
    {
        NoneRendering=0,
        PreviewRendering,
        FinalRendering
    };

public:

    EditorToolThreadedPriv()
    {
        threadedFilter       = 0;
        currentRenderingMode = NoneRendering;
    }

    int                 currentRenderingMode;

    QString             progressMess;

    DImgThreadedFilter *threadedFilter;
};

EditorToolThreaded::EditorToolThreaded(QObject *parent)
                  : EditorTool(parent)
{
    d = new EditorToolThreadedPriv;
}

EditorToolThreaded::~EditorToolThreaded()
{
    delete d->threadedFilter;
    delete d;
}

void EditorToolThreaded::setProgressMessage(const QString& mess)
{
    d->progressMess = mess;
}

DImgThreadedFilter* EditorToolThreaded::filter() const
{
    return d->threadedFilter;
}

void EditorToolThreaded::setFilter(DImgThreadedFilter *filter)
{
    d->threadedFilter = filter;

    connect(d->threadedFilter, SIGNAL(started()),
            this, SLOT(slotFilterStarted()));

    connect(d->threadedFilter, SIGNAL(finished(bool)),
            this, SLOT(slotFilterFinished(bool)));

    connect(d->threadedFilter, SIGNAL(progress(int)),
            this, SLOT(slotFilterProgress(int)));

    d->threadedFilter->startFilter();
}

void EditorToolThreaded::slotResized()
{
    if (d->currentRenderingMode == EditorToolThreadedPriv::FinalRendering)
    {
       toolView()->update();
       return;
    }
    else if (d->currentRenderingMode == EditorToolThreadedPriv::PreviewRendering)
    {
       if (filter())
          filter()->cancelFilter();
    }

    QTimer::singleShot(0, this, SLOT(slotEffect()));
}

void EditorToolThreaded::slotAbort()
{
    d->currentRenderingMode = EditorToolThreadedPriv::NoneRendering;
    EditorToolIface::editorToolIface()->setToolStopProgress();

    toolSettings()->enableButton(EditorToolSettings::Ok,      true);
    toolSettings()->enableButton(EditorToolSettings::Load,    true);
    toolSettings()->enableButton(EditorToolSettings::SaveAs,  true);
    toolSettings()->enableButton(EditorToolSettings::Try,     true);
    toolSettings()->enableButton(EditorToolSettings::Default, true);

    renderingFinished();
}

void EditorToolThreaded::slotFilterStarted()
{
}

void EditorToolThreaded::slotFilterFinished(bool success)
{
    if (success)        // Computation Completed !
    {
        switch (d->currentRenderingMode)
        {
            case EditorToolThreadedPriv::PreviewRendering:
            {
                kDebug(50003) << "Preview " << toolName() << " completed..." << endl;
                putPreviewData();
                slotAbort();
                break;
            }

            case EditorToolThreadedPriv::FinalRendering:
            {
                kDebug(50003) << "Final" << toolName() << " completed..." << endl;
                putFinalData();
                EditorToolIface::editorToolIface()->setToolStopProgress();
                kapp->restoreOverrideCursor();
                emit okClicked();
                break;
            }
        }
    }
    else                   // Computation Failed !
    {
        switch (d->currentRenderingMode)
        {
            case EditorToolThreadedPriv::PreviewRendering:
            {
                kDebug(50003) << "Preview " << toolName() << " failed..." << endl;
                slotAbort();
                break;
            }

            case EditorToolThreadedPriv::FinalRendering:
                break;
        }
    }
}

void EditorToolThreaded::slotFilterProgress(int progress)
{
    EditorToolIface::editorToolIface()->setToolProgress(progress);
}

void EditorToolThreaded::setToolView(QWidget *view)
{
    EditorTool::setToolView(view);

    if (dynamic_cast<ImageWidget*>(view) || dynamic_cast<ImageGuideWidget*>(view) ||
        dynamic_cast<ImagePanelWidget*>(view))
    {
        connect(view, SIGNAL(signalResized()),
                this, SLOT(slotResized()));
    }
}

void EditorToolThreaded::slotOk()
{
    writeSettings();

    d->currentRenderingMode = EditorToolThreadedPriv::FinalRendering;
    kDebug(50003) << "Final " << toolName() << " started..." << endl;
    writeSettings();

    toolSettings()->enableButton(EditorToolSettings::Ok,      false);
    toolSettings()->enableButton(EditorToolSettings::SaveAs,  false);
    toolSettings()->enableButton(EditorToolSettings::Load,    false);
    toolSettings()->enableButton(EditorToolSettings::Default, false);
    toolSettings()->enableButton(EditorToolSettings::Try,     false);

    EditorToolIface::editorToolIface()->setToolStartProgress(d->progressMess.isEmpty() ? toolName() : d->progressMess);
    kapp->setOverrideCursor( Qt::WaitCursor );

    if (d->threadedFilter)
    {
        delete d->threadedFilter;
        d->threadedFilter = 0;
    }

    prepareFinal();
}

void EditorToolThreaded::slotEffect()
{
    // Computation already in process.
    if (d->currentRenderingMode != EditorToolThreadedPriv::NoneRendering)
        return;

    d->currentRenderingMode = EditorToolThreadedPriv::PreviewRendering;
    kDebug(50003) << "Preview " << toolName() << " started..." << endl;

    toolSettings()->enableButton(EditorToolSettings::Ok,      false);
    toolSettings()->enableButton(EditorToolSettings::SaveAs,  false);
    toolSettings()->enableButton(EditorToolSettings::Load,    false);
    toolSettings()->enableButton(EditorToolSettings::Default, false);
    toolSettings()->enableButton(EditorToolSettings::Try,     false);

    EditorToolIface::editorToolIface()->setToolStartProgress(d->progressMess.isEmpty() ? toolName() : d->progressMess);

    if (d->threadedFilter)
    {
        delete d->threadedFilter;
        d->threadedFilter = 0;
    }

    prepareEffect();
}

void EditorToolThreaded::slotCancel()
{
    writeSettings();
    slotAbort();
    emit cancelClicked();
}

}  // namespace Digikam
