/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef HTML_PARAMETERS_PAGE_H
#define HTML_PARAMETERS_PAGE_H

// Qt includes

#include <QLabel>
#include <QScrollArea>
#include <QWidget>
#include <QByteArray>
#include <QMap>

// Local includes

#include "dwizardpage.h"
#include "theme.h"

namespace Digikam
{

class GalleryInfo;

class HTMLParametersPage : public DWizardPage
{
public:

    explicit HTMLParametersPage(QWizard* const dialog, const QString& title);
    ~HTMLParametersPage();

    void fillThemeParametersPage(Theme::Ptr theme, GalleryInfo* const info);

public:

    QScrollArea*                mScrollArea;
    QWidget*                    mContent;
    QMap<QByteArray, QWidget*>  mThemeParameterWidgetFromName;
};

} // namespace Digikam

#endif // HTML_PARAMETERS_PAGE_H
