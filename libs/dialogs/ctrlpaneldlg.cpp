/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-07
 * Description : A threaded filter control panel dialog for
 *               image editor plugins using DImg
 *
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <q3vgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <q3whatsthis.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <q3frame.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QCustomEvent>
#include <QCloseEvent>
#include <QKeyEvent>
#include <Q3VBoxLayout>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <ktoolinvocation.h>

// Local includes.

#include "ddebug.h"
#include "dimgthreadedfilter.h"
#include "dimginterface.h"
#include "ctrlpaneldlg.h"
#include "ctrlpaneldlg.moc"

namespace Digikam
{

class CtrlPanelDlgPriv
{
public:

    enum RunningMode
    {
        NoneRendering=0,
        PreviewRendering,
        FinalRendering
    };
    
    CtrlPanelDlgPriv()
    {
        parent               = 0;
        timer                = 0;
        aboutData            = 0;
        progressBar          = true;
        tryAction            = false;
        currentRenderingMode = NoneRendering;
    }

    bool         tryAction;
    bool         progressBar;

    int          currentRenderingMode;

    QWidget     *parent;

    QTimer      *timer;

    QString      name;

    KAboutData  *aboutData;
};

CtrlPanelDlg::CtrlPanelDlg(QWidget* parent, QString title, QString name,
                           bool loadFileSettings, bool tryAction, bool progressBar,
                           int separateViewMode, Q3Frame* bannerFrame)
            : KDialogBase(Plain, 0,
                          Help|Default|User1|User2|User3|Try|Ok|Cancel, Ok,
                          parent, 0, true, true,
                          i18n("&Abort"),
                          i18n("&Save As..."),
                          i18n("&Load..."))
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    setCaption(DImgInterface::defaultInterface()->getImageFileName() + QString(" - ") + title);
    
    d = new CtrlPanelDlgPriv;
    d->parent               = parent;
    d->name                 = name;
    d->tryAction            = tryAction;
    d->progressBar          = progressBar;
    m_threadedFilter        = 0;
    QString whatsThis;

    setButtonWhatsThis ( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User1, i18n("<p>Abort the current image rendering.") );
    setButtonWhatsThis ( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User2, i18n("<p>Save all filter parameters to settings text file.") );
    showButton(User2, loadFileSettings);
    showButton(User3, loadFileSettings);
    showButton(Try, tryAction);

    resize(configDialogSize(name + QString(" Tool Dialog")));
    Q3VBoxLayout *topLayout = new Q3VBoxLayout( plainPage(), 0, spacingHint());
    
    // -------------------------------------------------------------

    if (bannerFrame)
    {
        bannerFrame->reparent( plainPage(), QPoint(0, 0) );
        topLayout->addWidget(bannerFrame);
    }

    // -------------------------------------------------------------

    m_imagePreviewWidget = new ImagePannelWidget(470, 350, name + QString(" Tool Dialog"),
                               plainPage(), separateViewMode);
    topLayout->addWidget(m_imagePreviewWidget);

    // -------------------------------------------------------------

    QTimer::singleShot(0, this, SLOT(slotInit()));
    kapp->restoreOverrideCursor();
}

CtrlPanelDlg::~CtrlPanelDlg()
{
    if (d->aboutData)
       delete d->aboutData;
       
    if (d->timer)
       delete d->timer;

    if (m_threadedFilter)
       delete m_threadedFilter;

    delete d;
}

void CtrlPanelDlg::slotInit()
{
    // Reset values to defaults.
    QTimer::singleShot(0, this, SLOT(readUserSettings()));

    if (!d->tryAction)
    {
       connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
               this, SLOT(slotFocusChanged()));
    }
    else
    {
       connect(m_imagePreviewWidget, SIGNAL(signalResized()),
               this, SLOT(slotFocusChanged()));
    }
}

void CtrlPanelDlg::setAboutData(KAboutData *about)
{
    d->aboutData            = about;
    QPushButton *helpButton = actionButton( Help );
    KHelpMenu* helpMenu     = new KHelpMenu(this, d->aboutData, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("digiKam Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    helpButton->setPopup( helpMenu->menu() );
}

void CtrlPanelDlg::abortPreview()
{
    d->currentRenderingMode = CtrlPanelDlgPriv::NoneRendering;
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_imagePreviewWidget->setEnable(true);
    m_imagePreviewWidget->setProgressVisible(false);
    enableButton(Ok,      true);
    enableButton(User1,   false);
    enableButton(User2,   true);
    enableButton(User3,   true);
    enableButton(Try,     true);
    enableButton(Default, true);
    renderingFinished();
}

void CtrlPanelDlg::slotTry()
{
    slotEffect();
}

void CtrlPanelDlg::slotUser1()
{
    if (d->currentRenderingMode != CtrlPanelDlgPriv::NoneRendering)
        if (m_threadedFilter)
           m_threadedFilter->stopComputation();
}

void CtrlPanelDlg::slotDefault()
{
   resetValues();
   slotEffect();
}

void CtrlPanelDlg::slotCancel()
{
    if (d->currentRenderingMode != CtrlPanelDlgPriv::NoneRendering)
    {
       if (m_threadedFilter)
          m_threadedFilter->stopComputation();

       kapp->restoreOverrideCursor();
    }
    
    saveDialogSize(d->name + QString(" Tool Dialog"));
    done(Cancel);
}

void CtrlPanelDlg::closeEvent(QCloseEvent *e)
{
    if (d->currentRenderingMode != CtrlPanelDlgPriv::NoneRendering)
    {
       if (m_threadedFilter)
          m_threadedFilter->stopComputation();

       kapp->restoreOverrideCursor();
    }

    saveDialogSize(d->name + QString(" Tool Dialog"));
    e->accept();
}

void CtrlPanelDlg::slotFocusChanged(void)
{
    if (d->currentRenderingMode == CtrlPanelDlgPriv::FinalRendering)
    {
       m_imagePreviewWidget->update();
       return;
    }
    else if (d->currentRenderingMode == CtrlPanelDlgPriv::PreviewRendering)
    {
       if (m_threadedFilter)
          m_threadedFilter->stopComputation();
    }

    QTimer::singleShot(0, this, SLOT(slotEffect()));
}

void CtrlPanelDlg::slotHelp()
{
    // If setAboutData() is called by plugin, well DigikamImagePlugins help is lauched, 
    // else digiKam help. In this case, setHelp() method must be used to set anchor and handbook name.

    if (d->aboutData)
        KToolInvocation::invokeHelp(d->name, "digikam");
    else
        KDialogBase::slotHelp();
}

void CtrlPanelDlg::slotTimer()
{
    if (d->timer)
    {
       d->timer->stop();
       delete d->timer;
    }

    d->timer = new QTimer( this );
    connect( d->timer, SIGNAL(timeout()),
             this, SLOT(slotEffect()) );
    d->timer->start(500, true);
}

void CtrlPanelDlg::slotEffect()
{
    // Computation already in process.
    if (d->currentRenderingMode != CtrlPanelDlgPriv::NoneRendering)
        return;

    d->currentRenderingMode = CtrlPanelDlgPriv::PreviewRendering;
    DDebug() << "Preview " << d->name << " started..." << endl;

    m_imagePreviewWidget->setEnable(false);
    m_imagePreviewWidget->setProgressVisible(true);
    enableButton(Ok,      false);
    enableButton(User1,   true);
    enableButton(User2,   false);
    enableButton(User3,   false);
    enableButton(Try,     false);
    enableButton(Default, false);
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    m_imagePreviewWidget->setProgress(0);

    if (m_threadedFilter)
    {
       delete m_threadedFilter;
       m_threadedFilter = 0;
    }

    prepareEffect();
}

void CtrlPanelDlg::slotOk()
{
    d->currentRenderingMode = CtrlPanelDlgPriv::FinalRendering;
    DDebug() << "Final " << d->name << " started..." << endl;
    saveDialogSize(d->name + QString(" Tool Dialog"));
    writeUserSettings();

    m_imagePreviewWidget->setEnable(false);
    m_imagePreviewWidget->setProgressVisible(true);
    enableButton(Ok,      false);
    enableButton(User1,   false);
    enableButton(User2,   false);
    enableButton(User3,   false);
    enableButton(Try,     false);
    enableButton(Default, false);
    kapp->setOverrideCursor( Qt::WaitCursor );
    m_imagePreviewWidget->setProgress(0);

    if (m_threadedFilter)
    {
       delete m_threadedFilter;
       m_threadedFilter = 0;
    }

    prepareFinal();
}

void CtrlPanelDlg::customEvent(QCustomEvent *event)
{
    if (!event) return;

    DImgThreadedFilter::EventData *ed = (DImgThreadedFilter::EventData*) event->data();

    if (!ed) return;

    if (ed->starting)           // Computation in progress !
    {
        m_imagePreviewWidget->setProgress(ed->progress);
    }
    else
    {
        if (ed->success)        // Computation Completed !
        {
            switch (d->currentRenderingMode)
            {
              case CtrlPanelDlgPriv::PreviewRendering:
              {
                 DDebug() << "Preview " << d->name << " completed..." << endl;
                 putPreviewData();
                 abortPreview();
                 break;
              }

              case CtrlPanelDlgPriv::FinalRendering:
              {
                 DDebug() << "Final" << d->name << " completed..." << endl;
                 putFinalData();
                 kapp->restoreOverrideCursor();
                 accept();
                 break;
              }
            }
        }
        else                   // Computation Failed !
        {
            switch (d->currentRenderingMode)
            {
                case CtrlPanelDlgPriv::PreviewRendering:
                {
                    DDebug() << "Preview " << d->name << " failed..." << endl;
                    // abortPreview() must be call here for set progress bar to 0 properly.
                    abortPreview();
                    break;
                }

                case CtrlPanelDlgPriv::FinalRendering:
                    break;
            }
        }
    }

    delete ed;
}

// Backport KDialog::keyPressEvent() implementation from KDELibs to ignore Enter/Return Key events 
// to prevent any conflicts between dialog keys events and SpinBox keys events.

void CtrlPanelDlg::keyPressEvent(QKeyEvent *e)
{
    if ( e->state() == 0 )
    {
        switch ( e->key() )
        {
        case Qt::Key_Escape:
            e->accept();
            reject();
        break;
        case Qt::Key_Enter:            
        case Qt::Key_Return:     
            e->ignore();              
        break;
        default:
            e->ignore();
            return;
        }
    }
    else
    {
        // accept the dialog when Ctrl-Return is pressed
        if ( e->state() == Qt::ControlModifier &&
            (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) )
        {
            e->accept();
            accept();
        }
        else
        {
            e->ignore();
        }
    }
}

}  // NameSpace Digikam

