/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : setup tab for slideshow options.
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

// QT includes.

#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kglobal.h>

// Local includes.

#include "setupslideshow.h"
#include "setupslideshow.moc"

namespace Digikam
{

class SetupSlideShowPriv
{
public:

    SetupSlideShowPriv()
    {
        delayInput           = 0;
        startWithCurrent     = 0;
        loopMode             = 0;
        printName            = 0;
        printDate            = 0;
        printApertureFocal   = 0;
        printExpoSensitivity = 0;
        printMakeModel       = 0;
        printComment         = 0;
    }

    QCheckBox    *startWithCurrent;
    QCheckBox    *loopMode;
    QCheckBox    *printName;
    QCheckBox    *printDate;
    QCheckBox    *printApertureFocal;
    QCheckBox    *printExpoSensitivity;
    QCheckBox    *printMakeModel;
    QCheckBox    *printComment;
    
    KIntNumInput *delayInput;
};    
    
SetupSlideShow::SetupSlideShow(QWidget* parent )
              : QWidget(parent)
{
    d = new SetupSlideShowPriv;
    QVBoxLayout *layout = new QVBoxLayout( parent );
    
    d->delayInput = new KIntNumInput(5, parent);
    d->delayInput->setRange(1, 3600, 1, true );
    d->delayInput->setLabel( i18n("&Delay between images:"), Qt::AlignLeft|Qt::AlignTop );
    d->delayInput->setWhatsThis( i18n("<p>The delay, in seconds, between images."));
    
    d->startWithCurrent = new QCheckBox(i18n("Start with current image"), parent);
    d->startWithCurrent->setWhatsThis( i18n("<p>If this option is enabled, Slideshow will be started "
                                            "with current image selected from the images list."));
    
    d->loopMode = new QCheckBox(i18n("Display in loop"), parent);
    d->loopMode->setWhatsThis( i18n("<p>Run the slideshow in a loop."));
    
    d->printName = new QCheckBox(i18n("Print image file name"), parent);
    d->printName->setWhatsThis( i18n("<p>Print image file name to the screen bottom."));

    d->printDate = new QCheckBox(i18n("Print image creation date"), parent);
    d->printDate->setWhatsThis( i18n("<p>Print image creation to the screen bottom."));

    d->printApertureFocal = new QCheckBox(i18n("Print camera aperture and focal length"), parent);
    d->printApertureFocal->setWhatsThis( i18n("<p>Print camera aperture and focal length to the screen bottom."));

    d->printExpoSensitivity = new QCheckBox(i18n("Print camera exposure and sensitivity"), parent);
    d->printExpoSensitivity->setWhatsThis( i18n("<p>Print camera exposure and sensitivity to the screen  bottom."));

    d->printMakeModel = new QCheckBox(i18n("Print camera make and model"), parent);
    d->printMakeModel->setWhatsThis( i18n("<p>Print camera make and model to the screen bottom."));

    d->printComment = new QCheckBox(i18n("Print image comment"), parent);
    d->printComment->setWhatsThis( i18n("<p>Print image comment on bottom of screen."));
    
    layout->addWidget(d->delayInput);
    layout->addWidget(d->startWithCurrent);
    layout->addWidget(d->loopMode);
    layout->addWidget(d->printName);
    layout->addWidget(d->printDate);
    layout->addWidget(d->printApertureFocal);
    layout->addWidget(d->printExpoSensitivity);
    layout->addWidget(d->printMakeModel);
    layout->addWidget(d->printComment);
    layout->addStretch();
    
    readSettings();
}

SetupSlideShow::~SetupSlideShow()
{
    delete d;
}

void SetupSlideShow::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("ImageViewer Settings"));

    group.writeEntry("SlideShowDelay", d->delayInput->value());
    group.writeEntry("SlideShowStartCurrent", d->startWithCurrent->isChecked());
    group.writeEntry("SlideShowLoop", d->loopMode->isChecked());
    group.writeEntry("SlideShowPrintName", d->printName->isChecked());
    group.writeEntry("SlideShowPrintDate", d->printDate->isChecked());
    group.writeEntry("SlideShowPrintApertureFocal", d->printApertureFocal->isChecked());
    group.writeEntry("SlideShowPrintExpoSensitivity", d->printExpoSensitivity->isChecked());
    group.writeEntry("SlideShowPrintMakeModel", d->printMakeModel->isChecked());
    group.writeEntry("SlideShowPrintComment", d->printComment->isChecked());
    config->sync();
}

void SetupSlideShow::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("ImageViewer Settings"));

    d->delayInput->setValue(group.readEntry("SlideShowDelay", 5));
    d->startWithCurrent->setChecked(group.readEntry("SlideShowStartCurrent", false));
    d->loopMode->setChecked(group.readEntry("SlideShowLoop", false));
    d->printName->setChecked(group.readEntry("SlideShowPrintName", true));
    d->printDate->setChecked(group.readEntry("SlideShowPrintDate", false));
    d->printApertureFocal->setChecked(group.readEntry("SlideShowPrintApertureFocal", false));
    d->printExpoSensitivity->setChecked(group.readEntry("SlideShowPrintExpoSensitivity", false));
    d->printMakeModel->setChecked(group.readEntry("SlideShowPrintMakeModel", false));
    d->printComment->setChecked(group.readEntry("SlideShowPrintComment", false));
}

}   // namespace Digikam

