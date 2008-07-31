/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : save JPEG image options.
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QString>
#include <QLabel>
#include <QLayout>
#include <QFrame>
#include <QGridLayout>
#include <QComboBox>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <knuminput.h>

// Local includes.

#include "jpegsettings.h"
#include "jpegsettings.moc"

namespace Digikam
{

class JPEGSettingsPriv
{

public:

    JPEGSettingsPriv()
    {
        JPEGGrid             = 0;
        labelJPEGcompression = 0;
        JPEGcompression      = 0;
        labelWarning         = 0;
        labelSubSampling     = 0;
        subSamplingCB        = 0;
    }

    QGridLayout  *JPEGGrid;

    QLabel       *labelJPEGcompression;
    QLabel       *labelWarning;
    QLabel       *labelSubSampling;

    QComboBox    *subSamplingCB;

    KIntNumInput *JPEGcompression;
};

JPEGSettings::JPEGSettings(QWidget *parent)
            : QWidget(parent)
{
    d = new JPEGSettingsPriv;
    setAttribute(Qt::WA_DeleteOnClose);

    d->JPEGGrid             = new QGridLayout(this);
    d->JPEGcompression      = new KIntNumInput(75, this);
    d->JPEGcompression->setRange(1, 100);
    d->labelJPEGcompression = new QLabel(i18n("JPEG quality:"), this);

    d->JPEGcompression->setWhatsThis( i18n("<p>The JPEG image quality:<p>"
                                           "<b>1</b>: low quality (high compression and small "
                                           "file size)<p>"
                                           "<b>50</b>: medium quality<p>"
                                           "<b>75</b>: good quality (default)<p>"
                                           "<b>100</b>: high quality (no compression and "
                                           "large file size)<p>"
                                           "<b>Note: JPEG always uses lossy compression.</b>"));

    d->labelWarning = new QLabel(i18n("<qt><font size=-1 color=\"red\"><i>"
                          "Warning: <a href='http://en.wikipedia.org/wiki/JPEG'>JPEG</a> is a<br>"
                          "lossy compression<br>"
                          "image format!</p>"
                          "</i></qt>"), this);

    d->labelWarning->setOpenExternalLinks(true);
    d->labelWarning->setFrameStyle(QFrame::Box | QFrame::Plain);
    d->labelWarning->setLineWidth(1);
    d->labelWarning->setFrameShape(QFrame::Box);

    d->labelSubSampling = new QLabel(i18n("Chroma subsampling:"), this);

    d->subSamplingCB = new QComboBox(this);
    d->subSamplingCB->insertItem(0, i18n("None"));    // 1x1, 1x1, 1x1 (4:4:4)
    d->subSamplingCB->insertItem(1, i18n("Medium"));  // 2x1, 1x1, 1x1 (4:2:2)
    d->subSamplingCB->insertItem(2, i18n("High"));    // 2x2, 1x1, 1x1 (4:1:1)
    d->subSamplingCB->setWhatsThis( i18n("<p>JPEG Chroma subsampling level \n(color is saved with less resolution "
                                         "than luminance):<p>"
                                         "<b>None</b>=best: uses 4:4:4 ratio. Does not employ chroma "
                                         "subsampling at all. This preserves edges and contrasting "
                                         "colors, whilst adding no additional compression<p>"
                                         "<b>Medium</b>: uses 4:2:2 ratio. Medium compression: reduces "
                                         "the color resolution by one-third with little to "
                                         "no visual difference<p>"
                                         "<b>High</b>: use 4:1:1 ratio. High compression: suits "
                                         "images with soft edges but tends to alter colors<p>"
                                         "<b>Note: JPEG always uses lossy compression.</b>"));

    d->JPEGGrid->addWidget(d->labelJPEGcompression, 0, 0, 1, 1);
    d->JPEGGrid->addWidget(d->JPEGcompression,      0, 2, 1, 1);
    d->JPEGGrid->addWidget(d->labelSubSampling,     1, 0, 1, 1);
    d->JPEGGrid->addWidget(d->subSamplingCB,        1, 1, 1, 2);
    d->JPEGGrid->addWidget(d->labelWarning,         0, 3, 2, 1);
    d->JPEGGrid->setColumnStretch(1, 10);
    d->JPEGGrid->setRowStretch(2, 10);
    d->JPEGGrid->setAlignment(d->JPEGcompression, Qt::AlignCenter);
    d->JPEGGrid->setMargin(KDialog::spacingHint());
    d->JPEGGrid->setSpacing(KDialog::spacingHint());
}

JPEGSettings::~JPEGSettings()
{
    delete d;
}

void JPEGSettings::setCompressionValue(int val)
{
    d->JPEGcompression->setValue(val);
}

int JPEGSettings::getCompressionValue()
{
    return d->JPEGcompression->value();
}

void JPEGSettings::setSubSamplingValue(int val)
{
    d->subSamplingCB->setCurrentIndex(val);
}

int JPEGSettings::getSubSamplingValue()
{
    return d->subSamplingCB->currentIndex();
}

}  // namespace Digikam
