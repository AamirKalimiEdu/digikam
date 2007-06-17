/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-16
 * Description : a dialog to display icc profile information.
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

// KDE includes.

#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "iccprofilewidget.h"
#include "iccprofileinfodlg.h"

namespace Digikam
{

ICCProfileInfoDlg::ICCProfileInfoDlg(QWidget* parent, const QString& profilePath,
                                     const QByteArray& profileData)
                 : KDialog(parent)
{
    setCaption(i18n("Color Profile Info"));
    setButtons(Help|Ok);
    setDefaultButton(Ok);
    setModal(true);
    setHelp("iccprofile.anchor", "digikam");
    setCaption(profilePath);
    QWidget *page=new QWidget(this);
    setMainWidget(page); 
    ICCProfileWidget *profileWidget = new ICCProfileWidget(page, 0, 340, 256);
    
    if (profileData.isEmpty())
        profileWidget->loadFromURL(KUrl(profilePath));
    else
        profileWidget->loadFromData(profilePath, profileData); 
                                     
    setMainWidget(profileWidget);
}

ICCProfileInfoDlg::~ICCProfileInfoDlg()
{
}

}  // NameSpace Digikam

