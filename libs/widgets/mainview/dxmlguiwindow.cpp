/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : digiKam XML GUI window
 *
 * Copyright (C) 2013-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dxmlguiwindow.h"

// Qt includes

#include <QString>
#include <QList>
#include <QMap>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QToolButton>
#include <QEvent>
#include <QHoverEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeySequence>
#include <QMenuBar>
#include <QStatusBar>
#include <QMenu>

// KDE includes

#include <kxmlguiwindow.h>
#include <ktogglefullscreenaction.h>
#include <ktoolbar.h>
#include <ktoggleaction.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <khelpclient.h>
#include <kwindowconfig.h>
#include <knotifyconfigwidget.h>

// Local includes

#include "digikam_debug.h"
#include "daboutdata.h"

namespace Digikam
{

class DXmlGuiWindow::Private
{
public:

    Private()
    {
        fsOptions              = FS_NONE;
        fullScreenAction       = 0;
        fullScreenBtn          = 0;
        dirtyMainToolBar       = false;
        fullScreenHideToolBars = false;
        fullScreenHideThumbBar = true;
        fullScreenHideSideBars = false;
        thumbbarVisibility     = true;
        menubarVisibility      = true;
        statusbarVisibility    = true;
        libsInfoAction         = 0;
        showMenuBarAction      = 0;
        about                  = 0;
        dbStatAction           = 0;
        anim                   = 0;
    }

public:

    /** Settings taken from managed window configuration to handle toolbar visibility  in full-screen mode
     */
    bool                     fullScreenHideToolBars;

    /** Settings taken from managed window configuration to handle thumbbar visibility in full-screen mode
     */
    bool                     fullScreenHideThumbBar;

    /** Settings taken from managed window configuration to handle toolbar visibility  in full-screen mode
     */
    bool                     fullScreenHideSideBars;

    /** Full-Screen options. See FullScreenOptions enum and setFullScreenOptions() for details.
     */
    int                      fsOptions;

    /** Action plug in managed window to switch fullscreen state */
    KToggleFullScreenAction* fullScreenAction;

    /** Show only if toolbar is hidden */
    QToolButton*             fullScreenBtn;

    /** Used by slotToggleFullScreen() to manage state of full-screen button on managed window
     */
    bool                     dirtyMainToolBar;

    /** Store previous visibility of toolbars before ful-screen mode.
     */
    QMap<KToolBar*, bool>    toolbarsVisibility;

    /** Store previous visibility of thumbbar before ful-screen mode.
     */
    bool                     thumbbarVisibility;

    /** Store previous visibility of menubar before ful-screen mode.
     */
    bool                     menubarVisibility;

    /** Store previous visibility of statusbar before ful-screen mode.
     */
    bool                     statusbarVisibility;

    // Common Help actions
    QAction*                 dbStatAction;
    QAction*                 libsInfoAction;
    QAction*                 showMenuBarAction;
    DAboutData*              about;
    DLogoAction*             anim;
};

// --------------------------------------------------------------------------------------------------------

DXmlGuiWindow::DXmlGuiWindow(QWidget* const parent, Qt::WindowFlags f)
    : KXmlGuiWindow(parent, f), d(new Private)
{
    m_animLogo = 0;
    installEventFilter(this);
}

DXmlGuiWindow::~DXmlGuiWindow()
{
    delete d;
}

void DXmlGuiWindow::closeEvent(QCloseEvent* e)
{
    if(fullScreenIsActive())
        slotToggleFullScreen(false);

    KXmlGuiWindow::closeEvent(e);
}

void DXmlGuiWindow::setFullScreenOptions(int options)
{
    d->fsOptions = options;
}

void DXmlGuiWindow::createHelpActions(bool coreOptions)
{
    d->libsInfoAction = new QAction(QIcon::fromTheme("help-about"), i18n("Components Information"), this);
    connect(d->libsInfoAction, SIGNAL(triggered()), this, SLOT(slotComponentsInfo()));
    actionCollection()->addAction("help_librariesinfo", d->libsInfoAction);

    d->about = new DAboutData(this);
    d->about->registerHelpActions();

    m_animLogo = new DLogoAction(this);
    actionCollection()->addAction("logo_action", m_animLogo);

    // Add options only for core components (typically all excepted Showfoto)
    if (coreOptions)
    {
        d->dbStatAction = new QAction(QIcon::fromTheme("network-server-database"), i18n("Database Statistics"), this);
        connect(d->dbStatAction, SIGNAL(triggered()), this, SLOT(slotDBStat()));
        actionCollection()->addAction("help_dbstat", d->dbStatAction);
    }
}

void DXmlGuiWindow::createSidebarActions()
{
    KActionCollection* const ac = actionCollection();
    QAction* const tlsb = new QAction(i18n("Toggle Left Side-bar"), this);
    connect(tlsb, SIGNAL(triggered()), this, SLOT(slotToggleLeftSideBar()));
    ac->addAction("toggle-left-sidebar", tlsb);
    ac->setDefaultShortcut(tlsb, Qt::CTRL + Qt::META + Qt::Key_Left);

    QAction* const trsb = new QAction(i18n("Toggle Right Side-bar"), this);
    connect(trsb, SIGNAL(triggered()), this, SLOT(slotToggleRightSideBar()));
    ac->addAction("toggle-right-sidebar", trsb);
    ac->setDefaultShortcut(trsb, Qt::CTRL + Qt::META + Qt::Key_Right);

    QAction* const plsb = new QAction(i18n("Previous Left Side-bar Tab"), this);
    connect(plsb, SIGNAL(triggered()), this, SLOT(slotPreviousLeftSideBarTab()));
    ac->addAction("previous-left-sidebar-tab", plsb);
    ac->setDefaultShortcut(plsb, Qt::CTRL + Qt::META + Qt::Key_Home);

    QAction* const nlsb = new QAction(i18n("Next Left Side-bar Tab"), this);
    connect(nlsb, SIGNAL(triggered()), this, SLOT(slotNextLeftSideBarTab()));
    ac->addAction("next-left-sidebar-tab", nlsb);
    ac->setDefaultShortcut(nlsb, Qt::CTRL + Qt::META + Qt::Key_End);

    QAction* const prsb = new QAction(i18n("Previous Right Side-bar Tab"), this);
    connect(prsb, SIGNAL(triggered()), this, SLOT(slotPreviousRightSideBarTab()));
    ac->addAction("previous-right-sidebar-tab", prsb);
    ac->setDefaultShortcut(prsb, Qt::CTRL + Qt::META + Qt::Key_PageUp);

    QAction* const nrsb = new QAction(i18n("Next Right Side-bar Tab"), this);
    connect(nrsb, SIGNAL(triggered()), this, SLOT(slotNextRightSideBarTab()));
    ac->addAction("next-right-sidebar-tab", nrsb);
    ac->setDefaultShortcut(nrsb, Qt::CTRL + Qt::META + Qt::Key_PageDown);
}

void DXmlGuiWindow::createSettingsActions()
{
    d->showMenuBarAction = KStandardAction::showMenubar(this, SLOT(slotShowMenuBar()), actionCollection());
    KStandardAction::configureNotifications(this, SLOT(slotConfNotifications()), actionCollection());
}

QAction* DXmlGuiWindow::showMenuBarAction() const
{
    return d->showMenuBarAction;
}

void DXmlGuiWindow::slotShowMenuBar()
{
    menuBar()->setVisible(d->showMenuBarAction->isChecked());
}

void DXmlGuiWindow::slotConfNotifications()
{
    KNotifyConfigWidget::configure(this);
}

void DXmlGuiWindow::createFullScreenAction(const QString& name)
{
    d->fullScreenAction = KStandardAction::fullScreen(0, 0, this, this);
    actionCollection()->addAction(name, d->fullScreenAction);
    d->fullScreenBtn    = new QToolButton(this);
    d->fullScreenBtn->setDefaultAction(d->fullScreenAction);
    d->fullScreenBtn->hide();

    connect(d->fullScreenAction, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleFullScreen(bool)));
}

void DXmlGuiWindow::readFullScreenSettings(const KConfigGroup& group)
{
    if (d->fsOptions & FS_TOOLBARS)
        d->fullScreenHideToolBars  = group.readEntry(s_configFullScreenHideToolBarsEntry,  false);

    if (d->fsOptions & FS_THUMBBAR)
        d->fullScreenHideThumbBar = group.readEntry(s_configFullScreenHideThumbBarEntry, true);

    if (d->fsOptions & FS_SIDEBARS)
        d->fullScreenHideSideBars  = group.readEntry(s_configFullScreenHideSideBarsEntry,  false);
}

void DXmlGuiWindow::slotToggleFullScreen(bool set)
{
    KToggleFullScreenAction::setFullScreen(this, set);

    customizedFullScreenMode(set);

    if (!set)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "TURN OFF fullscreen";

        // restore menubar

        if (d->menubarVisibility)
            menuBar()->show();

        // restore statusbar

        if (d->statusbarVisibility)
            statusBar()->show();

        // restore sidebars

        if ((d->fsOptions & FS_SIDEBARS) && d->fullScreenHideSideBars)
            showSideBars(true);

        // restore thummbbar

        if ((d->fsOptions & FS_THUMBBAR) && d->fullScreenHideThumbBar)
            showThumbBar(d->thumbbarVisibility);

        // restore toolbars and manage full-screen button

        showToolBars(true);
        d->fullScreenBtn->hide();

        if (d->dirtyMainToolBar)
        {
            KToolBar* const mainbar = mainToolBar();

            if (mainbar)
            {
                mainbar->removeAction(d->fullScreenAction);
            }
        }
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "TURN ON fullscreen";

        // hide menubar

        d->menubarVisibility = menuBar()->isVisible();
        menuBar()->hide();

        // hide statusbar

        d->statusbarVisibility = statusBar()->isVisible();
        statusBar()->hide();

        // hide sidebars

        if ((d->fsOptions & FS_SIDEBARS) && d->fullScreenHideSideBars)
            showSideBars(false);

        // hide thummbbar

        d->thumbbarVisibility = thumbbarVisibility();

        if ((d->fsOptions & FS_THUMBBAR) && d->fullScreenHideThumbBar)
            showThumbBar(false);

        // hide toolbars and manage full-screen button

        if ((d->fsOptions & FS_TOOLBARS) && d->fullScreenHideToolBars)
        {
            showToolBars(false);
        }
        else
        {
            showToolBars(true);

            // add fullscreen action if necessary in toolbar

            KToolBar* const mainbar = mainToolBar();

            if (mainbar && !mainbar->actions().contains(d->fullScreenAction))
            {
                if (mainbar->actions().isEmpty())
                {
                    mainbar->addAction(d->fullScreenAction);
                }
                else
                {
                    mainbar->insertAction(mainbar->actions().first(), d->fullScreenAction);
                }

                d->dirtyMainToolBar = true;
            }
            else
            {
                // If FullScreen button is enabled in toolbar settings,
                // we shall not remove it when leaving of fullscreen mode.
                d->dirtyMainToolBar = false;
            }
        }
    }
}

bool DXmlGuiWindow::fullScreenIsActive() const
{
    if (d->fullScreenAction)
        return d->fullScreenAction->isChecked();

    qCDebug(DIGIKAM_GENERAL_LOG) << "FullScreenAction is not initialized";
    return false;
}

bool DXmlGuiWindow::eventFilter(QObject* obj, QEvent* ev)
{
    if (this && (obj == this))
    {
        if (ev && (ev->type() == QEvent::HoverMove) && fullScreenIsActive())
        {
            // We will handle a stand alone FullScreen button action on top/right corner of screen
            // only if managed window tool bar is hidden, and if we switched already in Full Screen mode.

            KToolBar* const mainbar = mainToolBar();

            if (mainbar)
            {
                if (((d->fsOptions & FS_TOOLBARS) && d->fullScreenHideToolBars) || !mainbar->isVisible())
                {
                    QHoverEvent* const mev = dynamic_cast<QHoverEvent*>(ev);

                    if (mev)
                    {
                        QPoint pos(mev->pos());
                        QRect  desktopRect = QApplication::desktop()->screenGeometry(this);

                        QRect sizeRect(QPoint(0, 0), d->fullScreenBtn->size());
                        QRect topLeft, topRight;
                        QRect topRightLarger;

                        desktopRect       = QRect(desktopRect.y(), desktopRect.y(), desktopRect.width(), desktopRect.height());
                        topLeft           = sizeRect;
                        topRight          = sizeRect;

                        topLeft.moveTo(desktopRect.x(), desktopRect.y());
                        topRight.moveTo(desktopRect.x() + desktopRect.width() - sizeRect.width() - 1, topLeft.y());

                        topRightLarger    = topRight.adjusted(-25, 0, 0, 10);

                        if (topRightLarger.contains(pos))
                        {
                            d->fullScreenBtn->move(topRight.topLeft());
                            d->fullScreenBtn->show();
                        }
                        else
                        {
                            d->fullScreenBtn->hide();
                        }

                        return false;
                    }
                }
            }
        }
    }

    // pass the event on to the parent class
    return QObject::eventFilter(obj, ev);
}

void DXmlGuiWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        if (fullScreenIsActive())
        {
            d->fullScreenAction->activate(QAction::Trigger);
        }
    }
}

KToolBar* DXmlGuiWindow::mainToolBar() const
{
    QList<KToolBar*> toolbars = toolBars();
    KToolBar* mainToolbar     = 0;

    foreach(KToolBar* const toolbar, toolbars)
    {
        if (toolbar && (toolbar->objectName() == "mainToolBar"))
        {
            mainToolbar = toolbar;
            break;
        }
    }

    return mainToolbar;
}

void DXmlGuiWindow::showToolBars(bool visible)
{
    // We will hide toolbars: store previous state for future restoring.
    if (!visible)
    {
        d->toolbarsVisibility.clear();

        foreach(KToolBar* const toolbar, toolBars())
        {
            if (toolbar)
            {
                bool visibility = toolbar->isVisible();
                d->toolbarsVisibility.insert(toolbar, visibility);
            }
        }
    }

    // Switch toolbars visibility

    for (QMap<KToolBar*, bool>::const_iterator it = d->toolbarsVisibility.constBegin(); it != d->toolbarsVisibility.constEnd(); ++it)
    {
        KToolBar* const toolbar = it.key();
        bool visibility         = it.value();

        if (toolbar)
        {
            if (visible && visibility)
                toolbar->show();
            else
                toolbar->hide();
        }
    }

    // We will show toolbars: restore previous state.
    if (visible)
    {
        for (QMap<KToolBar*, bool>::const_iterator it = d->toolbarsVisibility.constBegin(); it != d->toolbarsVisibility.constEnd(); ++it)
        {
            KToolBar* const toolbar = it.key();
            bool visibility         = it.value();

            if (toolbar)
            {
                visibility ? toolbar->show() : toolbar->hide();
            }
        }
    }
}

QAction* DXmlGuiWindow::statusBarMenuAction() const
{
    QList<QAction*> lst = actionCollection()->actions();

    foreach(QAction* const act, lst)
    {
        if (act && QString(act->objectName()) == QString("options_show_statusbar"))
            return act;
    }

    return 0;
}

void DXmlGuiWindow::showSideBars(bool visible)
{
    Q_UNUSED(visible);
}

void DXmlGuiWindow::showThumbBar(bool visible)
{
    Q_UNUSED(visible);
}

void DXmlGuiWindow::customizedFullScreenMode(bool set)
{
    Q_UNUSED(set);
}

bool DXmlGuiWindow::thumbbarVisibility() const
{
    return true;
}

void DXmlGuiWindow::openHandbook(const QString& anchor, const QString& appname)
{
    KHelpClient::invokeHelp(anchor, appname);
}

void DXmlGuiWindow::restoreWindowSize(QWindow* const win, const KConfigGroup& group)
{
    KWindowConfig::restoreWindowSize(win, group);
}

void DXmlGuiWindow::saveWindowSize(QWindow* const win, KConfigGroup& group)
{
    KWindowConfig::saveWindowSize(win, group);
}

} // namespace Digikam
