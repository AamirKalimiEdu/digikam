/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2007-02-06
 * Description : setup RAW decoding settings.
 * 
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <kcolorbutton.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kglobal.h>

// LibKDcraw includes.

#include <libkdcraw/dcrawsettingswidget.h>


// Local includes.

#include "setupdcraw.h"
#include "setupdcraw.moc"

namespace Digikam
{

class SetupDcrawPriv
{
public:


    SetupDcrawPriv()
    {
        dcrawSettings = 0;
    }

    KDcrawIface::DcrawSettingsWidget *dcrawSettings;
};

SetupDcraw::SetupDcraw(QWidget* parent )
          : QWidget(parent)
{
    d = new SetupDcrawPriv;
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    d->dcrawSettings    = new KDcrawIface::DcrawSettingsWidget(this, true, false);
    layout->addWidget(d->dcrawSettings);
    layout->addStretch();

    readSettings();
}

SetupDcraw::~SetupDcraw()
{
    delete d;
}

void SetupDcraw::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("ImageViewer Settings"));
    group.writeEntry("SixteenBitsImage", d->dcrawSettings->sixteenBits());
    group.writeEntry("CameraColorBalance", d->dcrawSettings->useCameraWB());
    group.writeEntry("AutomaticColorBalance", d->dcrawSettings->useAutoColorBalance());
    group.writeEntry("RGBInterpolate4Colors", d->dcrawSettings->useFourColor());
    group.writeEntry("DontStretchPixels", d->dcrawSettings->useDontStretchPixels());
    group.writeEntry("EnableNoiseReduction", d->dcrawSettings->useNoiseReduction());
    group.writeEntry("NRThreshold", d->dcrawSettings->NRThreshold());
    group.writeEntry("UnclipColors", d->dcrawSettings->unclipColor());
    group.writeEntry("RAWBrightness", d->dcrawSettings->brightness());
    group.writeEntry("RAWQuality", (int)d->dcrawSettings->quality());
    config->sync();
}

void SetupDcraw::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("ImageViewer Settings"));
    d->dcrawSettings->setSixteenBits(group.readEntry("SixteenBitsImage", false));
    d->dcrawSettings->setNoiseReduction(group.readEntry("EnableNoiseReduction", false));
    d->dcrawSettings->setNRThreshold(group.readEntry("NRThreshold", 100));
    d->dcrawSettings->setDontStretchPixels(group.readEntry("DontStretchPixels", false));
    d->dcrawSettings->setUnclipColor(group.readEntry("UnclipColors", 0));
    d->dcrawSettings->setCameraWB(group.readEntry("CameraColorBalance", true));
    d->dcrawSettings->setAutoColorBalance(group.readEntry("AutomaticColorBalance", true));
    d->dcrawSettings->setFourColor(group.readEntry("RGBInterpolate4Colors", false));
    d->dcrawSettings->setQuality((KDcrawIface::RawDecodingSettings::DecodingQuality)
                                  group.readEntry("RAWQuality",
                                  (int)KDcrawIface::RawDecodingSettings::BILINEAR));
    d->dcrawSettings->setBrightness(group.readEntry("RAWBrightness", 1.0));
}

}  // namespace Digikam
