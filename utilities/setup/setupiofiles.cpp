/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2006-01-23
 * Description : setup image editor output files settings.
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

// QT includes.

#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kseparator.h>
#include <kglobal.h>

// Local includes.

#include "jpegsettings.h"
#include "pngsettings.h"
#include "tiffsettings.h"
#include "jp2ksettings.h"
#include "setupiofiles.h"
#include "setupiofiles.moc"

namespace Digikam
{

class SetupIOFilesPriv
{
public:


    SetupIOFilesPriv()
    {
        JPEGOptions     = 0;
        PNGOptions      = 0;
        TIFFOptions     = 0;
        JPEG2000Options = 0;
    }

    JPEGSettings *JPEGOptions;

    PNGSettings  *PNGOptions;

    TIFFSettings *TIFFOptions;

    JP2KSettings *JPEG2000Options;
};

SetupIOFiles::SetupIOFiles(QWidget* parent )
            : QWidget(parent)
{
    d = new SetupIOFilesPriv;

    QVBoxLayout* vbox = new QVBoxLayout(this);

    //-- JPEG Settings ------------------------------------------------------

    d->JPEGOptions    = new JPEGSettings(this);
    KSeparator *line1 = new KSeparator(Qt::Horizontal, this);

    //-- PNG Settings -------------------------------------------------------

    d->PNGOptions     = new PNGSettings(this);
    KSeparator *line2 = new KSeparator(Qt::Horizontal, this);

    //-- TIFF Settings ------------------------------------------------------

    d->TIFFOptions    = new TIFFSettings(this);
    KSeparator *line3 = new KSeparator(Qt::Horizontal, this);

    //-- JPEG 2000 Settings -------------------------------------------------

    d->JPEG2000Options = new JP2KSettings(this);

    vbox->setMargin(0);
    vbox->setSpacing(KDialog::spacingHint());
    vbox->addWidget(d->JPEGOptions);
    vbox->addWidget(line1);
    vbox->addWidget(d->PNGOptions);
    vbox->addWidget(line2);
    vbox->addWidget(d->TIFFOptions);
    vbox->addWidget(line3);
    vbox->addWidget(d->JPEG2000Options);
    vbox->addStretch(10);

    readSettings();
}

SetupIOFiles::~SetupIOFiles()
{
    delete d;
}

void SetupIOFiles::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("ImageViewer Settings"));
    group.writeEntry("JPEGCompression", d->JPEGOptions->getCompressionValue());
    group.writeEntry("PNGCompression", d->PNGOptions->getCompressionValue());
    group.writeEntry("TIFFCompression", d->TIFFOptions->getCompression());
    group.writeEntry("JPEG2000Compression", d->JPEG2000Options->getCompressionValue());
    group.writeEntry("JPEG2000LossLess", d->JPEG2000Options->getLossLessCompression());
    config->sync();
}

void SetupIOFiles::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("ImageViewer Settings"));
    d->JPEGOptions->setCompressionValue(group.readEntry("JPEGCompression", 75) );
    d->PNGOptions->setCompressionValue(group.readEntry("PNGCompression", 9) );
    d->TIFFOptions->setCompression(group.readEntry("TIFFCompression", false));
    d->JPEG2000Options->setCompressionValue( group.readEntry("JPEG2000Compression", 75) );
    d->JPEG2000Options->setLossLessCompression( group.readEntry("JPEG2000LossLess", true) );
}

}  // namespace Digikam
