/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-02
 * Description : showFoto setup dialog.
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

// Local includes.

#include "setup.h"
#include "setup.moc"

// KDE includes.

#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kvbox.h>

// Digikam includes.

#include "setuptooltip.h"
#include "setupeditor.h"
#include "setupdcraw.h"
#include "setupiofiles.h"
#include "setupslideshow.h"
#include "setupicc.h"

namespace ShowFoto
{

class SetupPrivate
{
public:

    SetupPrivate()
    {
        editorPage      = 0;
        toolTipPage     = 0;
        dcrawPage       = 0;
        iofilesPage     = 0;
        slideshowPage   = 0;
        iccPage         = 0;
        page_editor     = 0;
        page_tooltip    = 0;
        page_dcraw      = 0;
        page_iofiles    = 0;
        page_slideshow  = 0;
        page_icc        = 0;
    }

    KPageWidgetItem          *page_editor;
    KPageWidgetItem          *page_tooltip;
    KPageWidgetItem          *page_dcraw;
    KPageWidgetItem          *page_iofiles;
    KPageWidgetItem          *page_slideshow;
    KPageWidgetItem          *page_icc;

    SetupEditor              *editorPage;
    SetupToolTip             *toolTipPage;

    Digikam::SetupDcraw      *dcrawPage;
    Digikam::SetupIOFiles    *iofilesPage;
    Digikam::SetupSlideShow  *slideshowPage;
    Digikam::SetupICC        *iccPage;
};

Setup::Setup(QWidget* parent, const char* name, Setup::Page page)
     : KPageDialog(parent)
{
    d = new SetupPrivate;
    setObjectName(name);
    setCaption(i18n("Configure"));
    setButtons( KDialog::Help|KDialog::Ok|KDialog::Cancel );
    setDefaultButton(KDialog::Ok);
    setHelp("setupdialog.anchor", "showfoto");
    setFaceType(KPageDialog::List);
    setModal(true);

    d->editorPage  = new SetupEditor();
    d->page_editor = addPage( d->editorPage, i18n("General") );
    d->page_editor->setHeader( i18n("General Settings") );
    d->page_editor->setIcon( KIcon("showfoto") );

    d->toolTipPage = new SetupToolTip();
    d->page_tooltip = addPage( d->toolTipPage, i18n("Tool Tip") );
    d->page_tooltip->setHeader( i18n("Thumbbar Items Tool Tip Settings") );
    d->page_tooltip->setIcon( KIcon("dialog-information") );

    d->dcrawPage = new Digikam::SetupDcraw();
    d->page_dcraw = addPage( d->dcrawPage, i18n("RAW decoding") );
    d->page_dcraw->setHeader( i18n("RAW Files Decoding Settings") );
    d->page_dcraw->setIcon( KIcon("kdcraw") );

    d->iccPage = new Digikam::SetupICC(0, this);
    d->page_icc = addPage( d->iccPage, i18n("Color Management") );
    d->page_icc->setHeader( i18n("Color Management Settings") );
    d->page_icc->setIcon( KIcon("colormanagement") );

    d->iofilesPage = new Digikam::SetupIOFiles();
    d->page_iofiles = addPage( d->iofilesPage, i18n("Save Images") );
    d->page_iofiles->setHeader( i18n("Save Images' Files' Settings") );
    d->page_iofiles->setIcon( KIcon("document-save-all") );

    d->slideshowPage = new Digikam::SetupSlideShow();
    d->page_slideshow = addPage( d->slideshowPage, i18n("Slide Show") );
    d->page_slideshow->setHeader( i18n("Slide Show Settings") );
    d->page_slideshow->setIcon( KIcon("view-presentation") );

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOkClicked()) );

    if (page != LastPageUsed)
        showPage(page);
    else
    {
        KSharedConfig::Ptr config = KGlobal::config();
        KConfigGroup group = config->group(QString("General Settings"));
        showPage((Page)group.readEntry("Setup Page", (int)EditorPage));
    }

    show();
}

Setup::~Setup()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("General Settings"));
    group.writeEntry("Setup Page", (int)activePageIndex());
    config->sync();
    delete d;
}

void Setup::slotOkClicked()
{
    d->editorPage->applySettings();
    d->toolTipPage->applySettings();
    d->dcrawPage->applySettings();
    d->iofilesPage->applySettings();
    d->slideshowPage->applySettings();
    d->iccPage->applySettings();
    close();
}

void Setup::showPage(Setup::Page page)
{
    switch(page)
    {
        case ToolTipPage:
            setCurrentPage(d->page_tooltip);
            break;
        case DcrawPage:
            setCurrentPage(d->page_dcraw);
            break;
        case IOFilesPage:
            setCurrentPage(d->page_iofiles);
            break;
        case SlideshowPage:
            setCurrentPage(d->page_slideshow);
            break;
        case ICCPage:
            setCurrentPage(d->page_icc);
            break;
        default:
            setCurrentPage(d->page_editor);
            break;
    }
}

Setup::Page Setup::activePageIndex()
{
    KPageWidgetItem *cur = currentPage();
    if (cur == d->page_tooltip)   return ToolTipPage;
    if (cur == d->page_dcraw)     return DcrawPage;
    if (cur == d->page_iofiles)   return IOFilesPage;
    if (cur == d->page_slideshow) return SlideshowPage;
    if (cur == d->page_icc)       return ICCPage;

    return EditorPage;
}

}   // namespace ShowFoto
