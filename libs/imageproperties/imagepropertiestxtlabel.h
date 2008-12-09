/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-05
 * Description : simple text labels to display image
 *               properties meta infos
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

#ifndef IMAGPROPERTIESTXTLABEL_H
#define IMAGPROPERTIESTXTLABEL_H

// Qt includes.

#include <QLabel>
#include <QPalette>
#include <QColor>
#include <QString>

// KDE includes.

#include <kglobalsettings.h>
#include <ksqueezedtextlabel.h>

class DTextLabelName : public QLabel
{

public:

    DTextLabelName(const QString& name, QWidget* parent=0)
        : QLabel(parent)
    {
        setText(name);
        setFont(KGlobalSettings::smallestReadableFont());
        setAlignment(Qt::AlignRight | Qt::AlignTop);
        setWordWrap(false);

        QPalette palette  = this->palette();
        QColor foreground = palette.color(QPalette::Foreground);
        foreground.setAlpha(128);
        palette.setColor(QPalette::Foreground, foreground);
        setPalette(palette);
    };

    ~DTextLabelName(){};
};

// -------------------------------------------------------------------

class DTextLabelValue : public KSqueezedTextLabel
{

public:

    DTextLabelValue(const QString& value, QWidget* parent=0)
        : KSqueezedTextLabel(parent)
    {
        setText(value);
        setFont(KGlobalSettings::smallestReadableFont());
        setAlignment(Qt::AlignLeft | Qt::AlignTop);
        setWordWrap(false);
    };

    ~DTextLabelValue(){};
};

#endif /* IMAGPROPERTIESTXTLABEL_H */
