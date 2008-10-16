/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : setup tab for slideshow options.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>

// KDE includes.

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <knuminput.h>

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
        delayInput          = 0;
        startWithCurrent    = 0;
        loopMode            = 0;
        showName            = 0;
        showDate            = 0;
        showApertureFocal   = 0;
        showExpoSensitivity = 0;
        showMakeModel       = 0;
        showComment         = 0;
        showRating          = 0;
    }

    QCheckBox    *startWithCurrent;
    QCheckBox    *loopMode;
    QCheckBox    *showName;
    QCheckBox    *showDate;
    QCheckBox    *showApertureFocal;
    QCheckBox    *showExpoSensitivity;
    QCheckBox    *showMakeModel;
    QCheckBox    *showComment;
    QCheckBox    *showRating;

    KIntNumInput *delayInput;
};

SetupSlideShow::SetupSlideShow(QWidget* parent)
              : QWidget(parent)
{
    d = new SetupSlideShowPriv;
    QVBoxLayout *layout = new QVBoxLayout(this);

    d->delayInput = new KIntNumInput(5, this);
    d->delayInput->setRange(1, 3600, 1);
    d->delayInput->setSliderEnabled(true);
    d->delayInput->setLabel(i18n("&Delay between images:"), Qt::AlignLeft | Qt::AlignTop);
    d->delayInput->setWhatsThis(i18n("The delay, in seconds, between images."));

    d->startWithCurrent = new QCheckBox(i18n("Start with current image"), this);
    d->startWithCurrent->setWhatsThis( i18n("If this option is enabled, the Slideshow will be started "
                                            "with the current image selected in the images list."));

    d->loopMode = new QCheckBox(i18n("Slideshow runs in a loop"), this);
    d->loopMode->setWhatsThis( i18n("Run the slideshow in a loop."));

    d->showName = new QCheckBox(i18n("Show image file name"), this);
    d->showName->setWhatsThis( i18n("Show the image file name at the bottom of the screen."));

    d->showDate = new QCheckBox(i18n("Show image creation date"), this);
    d->showDate->setWhatsThis( i18n("Show the image creation time/date at the bottom of the screen."));

    d->showApertureFocal = new QCheckBox(i18n("Show camera aperture and focal length"), this);
    d->showApertureFocal->setWhatsThis( i18n("Show the camera aperture and focal length at the bottom of the screen."));

    d->showExpoSensitivity = new QCheckBox(i18n("Show camera exposure and sensitivity"), this);
    d->showExpoSensitivity->setWhatsThis( i18n("Show the camera exposure and sensitivity at the bottom of the screen."));

    d->showMakeModel = new QCheckBox(i18n("Show camera make and model"), this);
    d->showMakeModel->setWhatsThis( i18n("Show the camera make and model at the bottom of the screen."));

    d->showComment = new QCheckBox(i18n("Show image caption"), this);
    d->showComment->setWhatsThis( i18n("Show the image caption at the bottom of the screen."));

    d->showRating = new QCheckBox(i18n("Show image rating"), this);
    d->showRating->setWhatsThis( i18n("Show the digiKam image rating at the bottom of the screen."));

    // Only digiKam support this feature, showFoto do not support digiKam database information.
    if (kapp->applicationName() == "showfoto")
        d->showRating->hide();

    layout->addWidget(d->delayInput);
    layout->addWidget(d->startWithCurrent);
    layout->addWidget(d->loopMode);
    layout->addWidget(d->showName);
    layout->addWidget(d->showDate);
    layout->addWidget(d->showApertureFocal);
    layout->addWidget(d->showExpoSensitivity);
    layout->addWidget(d->showMakeModel);
    layout->addWidget(d->showComment);
    layout->addWidget(d->showRating);
    layout->addStretch();
    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());

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
    group.writeEntry("SlideShowPrintName", d->showName->isChecked());
    group.writeEntry("SlideShowPrintDate", d->showDate->isChecked());
    group.writeEntry("SlideShowPrintApertureFocal", d->showApertureFocal->isChecked());
    group.writeEntry("SlideShowPrintExpoSensitivity", d->showExpoSensitivity->isChecked());
    group.writeEntry("SlideShowPrintMakeModel", d->showMakeModel->isChecked());
    group.writeEntry("SlideShowPrintComment", d->showComment->isChecked());
    group.writeEntry("SlideShowPrintRating", d->showRating->isChecked());
    config->sync();
}

void SetupSlideShow::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("ImageViewer Settings"));

    d->delayInput->setValue(group.readEntry("SlideShowDelay", 5));
    d->startWithCurrent->setChecked(group.readEntry("SlideShowStartCurrent", false));
    d->loopMode->setChecked(group.readEntry("SlideShowLoop", false));
    d->showName->setChecked(group.readEntry("SlideShowPrintName", true));
    d->showDate->setChecked(group.readEntry("SlideShowPrintDate", false));
    d->showApertureFocal->setChecked(group.readEntry("SlideShowPrintApertureFocal", false));
    d->showExpoSensitivity->setChecked(group.readEntry("SlideShowPrintExpoSensitivity", false));
    d->showMakeModel->setChecked(group.readEntry("SlideShowPrintMakeModel", false));
    d->showComment->setChecked(group.readEntry("SlideShowPrintComment", false));
    d->showRating->setChecked(group.readEntry("SlideShowPrintRating", false));
}

}   // namespace Digikam
