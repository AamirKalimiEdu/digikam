/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-07-09
 * Description : tool tip contents configuration setup tab
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


#include "setuptooltip.h"
#include "setuptooltip.moc"

// Qt includes.

#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>

namespace ShowFoto
{

class SetupToolTipPriv
{
public:

    SetupToolTipPriv()
    {
        showToolTipsBox   = 0;

        showFileNameBox   = 0;
        showFileDateBox   = 0;
        showFileSizeBox   = 0;
        showImageTypeBox  = 0;
        showImageDimBox   = 0;

        showPhotoMakeBox  = 0;
        showPhotoDateBox  = 0;
        showPhotoFocalBox = 0;
        showPhotoExpoBox  = 0;
        showPhotoModeBox  = 0;
        showPhotoFlashBox = 0;
        showPhotoWbBox    = 0;

        fileSettingBox    = 0;
        photoSettingBox   = 0;
    }

    QCheckBox *showToolTipsBox;

    QCheckBox *showFileNameBox;
    QCheckBox *showFileDateBox;
    QCheckBox *showFileSizeBox;
    QCheckBox *showImageTypeBox;
    QCheckBox *showImageDimBox;

    QCheckBox *showPhotoMakeBox;
    QCheckBox *showPhotoDateBox;
    QCheckBox *showPhotoFocalBox;
    QCheckBox *showPhotoExpoBox;
    QCheckBox *showPhotoModeBox;
    QCheckBox *showPhotoFlashBox;
    QCheckBox *showPhotoWbBox;

    QGroupBox *fileSettingBox;
    QGroupBox *photoSettingBox;
};

SetupToolTip::SetupToolTip(QWidget* parent)
            : QWidget(parent)
{
    d = new SetupToolTipPriv;
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setSpacing( KDialog::spacingHint() );

    d->showToolTipsBox = new QCheckBox(i18n("Show Thumbbar items toolti&ps"), this);
    d->showToolTipsBox->setWhatsThis( i18n("Set this option to display the image information when "
                                           "the mouse hovers over a thumbbar item."));

    layout->addWidget(d->showToolTipsBox);

    // --------------------------------------------------------

    d->fileSettingBox     = new QGroupBox(i18n("File/Image Information"), this);
    QVBoxLayout *gLayout1 = new QVBoxLayout();
    gLayout1->setSpacing( KDialog::spacingHint() );

    d->showFileNameBox = new QCheckBox(i18n("Show file name"), d->fileSettingBox);
    d->showFileNameBox->setWhatsThis( i18n("Set this option to display the image file name."));

    d->showFileDateBox = new QCheckBox(i18n("Show file date"), d->fileSettingBox);
    d->showFileDateBox->setWhatsThis( i18n("Set this option to display the image file date."));

    d->showFileSizeBox = new QCheckBox(i18n("Show file size"), d->fileSettingBox);
    d->showFileSizeBox->setWhatsThis( i18n("Set this option to display the image file size."));

    d->showImageTypeBox = new QCheckBox(i18n("Show image type"), d->fileSettingBox);
    d->showImageTypeBox->setWhatsThis( i18n("Set this option to display the image type."));

    d->showImageDimBox = new QCheckBox(i18n("Show image dimensions"), d->fileSettingBox);
    d->showImageDimBox->setWhatsThis( i18n("Set this option to display the image dimensions in pixels."));

    gLayout1->addWidget(d->showFileNameBox);
    gLayout1->addWidget(d->showFileDateBox);
    gLayout1->addWidget(d->showFileSizeBox);
    gLayout1->addWidget(d->showImageTypeBox);
    gLayout1->addWidget(d->showImageDimBox);
    d->fileSettingBox->setLayout(gLayout1);

    layout->addWidget(d->fileSettingBox);

    // --------------------------------------------------------

    d->photoSettingBox    = new QGroupBox(i18n("Photograph Information"), this);
    QVBoxLayout *gLayout2 = new QVBoxLayout();
    gLayout2->setSpacing( KDialog::spacingHint() );

    d->showPhotoMakeBox = new QCheckBox(i18n("Show camera make and model"), d->photoSettingBox);
    d->showPhotoMakeBox->setWhatsThis( i18n("Set this option to display the make and model of the "
                                               "camera with which the image has been taken."));

    d->showPhotoDateBox = new QCheckBox(i18n("Show camera date"), d->photoSettingBox);
    d->showPhotoDateBox->setWhatsThis( i18n("Set this option to display the date when the image was taken."));

    d->showPhotoFocalBox = new QCheckBox(i18n("Show camera aperture and focal length"), d->photoSettingBox);
    d->showPhotoFocalBox->setWhatsThis( i18n("Set this option to display the camera aperture and focal settings "
                     "used to take the image."));

    d->showPhotoExpoBox = new QCheckBox(i18n("Show camera exposure and sensitivity"), d->photoSettingBox);
    d->showPhotoExpoBox->setWhatsThis( i18n("Set this option to display the camera exposure and sensitivity "
                     "used to take the image."));

    d->showPhotoModeBox = new QCheckBox(i18n("Show camera mode and program"), d->photoSettingBox);
    d->showPhotoModeBox->setWhatsThis( i18n("Set this option to display the camera mode and program "
                     "used to take the image."));

    d->showPhotoFlashBox = new QCheckBox(i18n("Show camera flash settings"), d->photoSettingBox);
    d->showPhotoFlashBox->setWhatsThis( i18n("Set this option to display the camera flash settings "
                     "used to take the image."));

    d->showPhotoWbBox = new QCheckBox(i18n("Show camera white balance settings"), d->photoSettingBox);
    d->showPhotoWbBox->setWhatsThis( i18n("Set this option to display the camera white balance settings "
                     "used to take the image."));

    gLayout2->addWidget(d->showPhotoMakeBox);
    gLayout2->addWidget(d->showPhotoDateBox);
    gLayout2->addWidget(d->showPhotoFocalBox);
    gLayout2->addWidget(d->showPhotoExpoBox);
    gLayout2->addWidget(d->showPhotoModeBox);
    gLayout2->addWidget(d->showPhotoFlashBox);
    gLayout2->addWidget(d->showPhotoWbBox);
    d->photoSettingBox->setLayout(gLayout2);

    layout->addWidget(d->photoSettingBox);
    layout->addStretch();

    // --------------------------------------------------------

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->fileSettingBox, SLOT(setEnabled(bool)));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->photoSettingBox, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    readSettings();
    adjustSize();
}

SetupToolTip::~SetupToolTip()
{
    delete d;
}

void SetupToolTip::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("ImageViewer Settings"));

    d->showToolTipsBox->setChecked(group.readEntry("Show ToolTips", true));

    d->showFileNameBox->setChecked(group.readEntry("ToolTips Show File Name", true));
    d->showFileDateBox->setChecked(group.readEntry("ToolTips Show File Date", false));
    d->showFileSizeBox->setChecked(group.readEntry("ToolTips Show File Size", false));
    d->showImageTypeBox->setChecked(group.readEntry("ToolTips Show Image Type", false));
    d->showImageDimBox->setChecked(group.readEntry("ToolTips Show Image Dim", true));

    d->showPhotoMakeBox->setChecked(group.readEntry("ToolTips Show Photo Make", true));
    d->showPhotoDateBox->setChecked(group.readEntry("ToolTips Show Photo Date", true));
    d->showPhotoFocalBox->setChecked(group.readEntry("ToolTips Show Photo Focal", true));
    d->showPhotoExpoBox->setChecked(group.readEntry("ToolTips Show Photo Expo", true));
    d->showPhotoModeBox->setChecked(group.readEntry("ToolTips Show Photo Mode", true));
    d->showPhotoFlashBox->setChecked(group.readEntry("ToolTips Show Photo Flash", false));
    d->showPhotoWbBox->setChecked(group.readEntry("ToolTips Show Photo WB", false));

    d->fileSettingBox->setEnabled(d->showToolTipsBox->isChecked());
    d->photoSettingBox->setEnabled(d->showToolTipsBox->isChecked());
}

void SetupToolTip::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("ImageViewer Settings"));

    group.writeEntry("Show ToolTips", d->showToolTipsBox->isChecked());

    group.writeEntry("ToolTips Show File Name", d->showFileNameBox->isChecked());
    group.writeEntry("ToolTips Show File Date", d->showFileDateBox->isChecked());
    group.writeEntry("ToolTips Show File Size", d->showFileSizeBox->isChecked());
    group.writeEntry("ToolTips Show Image Type", d->showImageTypeBox->isChecked());
    group.writeEntry("ToolTips Show Image Dim", d->showImageDimBox->isChecked());

    group.writeEntry("ToolTips Show Photo Make", d->showPhotoMakeBox->isChecked());
    group.writeEntry("ToolTips Show Photo Date", d->showPhotoDateBox->isChecked());
    group.writeEntry("ToolTips Show Photo Focal", d->showPhotoFocalBox->isChecked());
    group.writeEntry("ToolTips Show Photo Expo", d->showPhotoExpoBox->isChecked());
    group.writeEntry("ToolTips Show Photo Mode", d->showPhotoModeBox->isChecked());
    group.writeEntry("ToolTips Show Photo Flash", d->showPhotoFlashBox->isChecked());
    group.writeEntry("ToolTips Show Photo WB", d->showPhotoWbBox->isChecked());

    config->sync();
}

}  // namespace ShowFoto

