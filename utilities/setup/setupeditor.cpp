/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-03
 * Description : setup Image Editor tab.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "setupeditor.h"
#include "setupeditor.moc"

// Qt includes.

#include <QColor>
#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kvbox.h>
#include <kaboutdata.h>


namespace Digikam
{
class SetupEditorPriv
{
public:

    SetupEditorPriv()
    {
        hideToolBar          = 0;
        themebackgroundColor = 0;
        backgroundColor      = 0;
        colorBox             = 0;
        overExposureColor    = 0;
        underExposureColor   = 0;
        hideThumbBar         = 0;
        horizontalThumbBar   = 0;
        useRawImportTool     = 0;
    }

    KHBox        *colorBox;

    QCheckBox    *hideToolBar;
    QCheckBox    *themebackgroundColor;
    QCheckBox    *hideThumbBar;
    QCheckBox    *horizontalThumbBar;
    QCheckBox    *useRawImportTool;

    KColorButton *backgroundColor;
    KColorButton *underExposureColor;
    KColorButton *overExposureColor;
};

SetupEditor::SetupEditor(QWidget* parent)
           : QWidget(parent)
{
    d = new SetupEditorPriv;

    QVBoxLayout *layout = new QVBoxLayout(this);

    // --------------------------------------------------------

    QGroupBox *interfaceOptionsGroup = new QGroupBox(i18n("Interface Options"), this);
    QVBoxLayout *gLayout1            = new QVBoxLayout(interfaceOptionsGroup);

    d->themebackgroundColor = new QCheckBox(i18n("&Use theme background color"),
                                            interfaceOptionsGroup);

    d->themebackgroundColor->setWhatsThis( i18n("Enable this option to use background theme "
                                                "color in image editor area") );

    d->colorBox = new KHBox(interfaceOptionsGroup);

    QLabel *backgroundColorlabel = new QLabel( i18n("&Background color:"), d->colorBox );

    d->backgroundColor = new KColorButton(d->colorBox);
    backgroundColorlabel->setBuddy(d->backgroundColor);
    d->backgroundColor->setWhatsThis( i18n("Customize the background color to use "
                                           "in image editor area.") );

    d->horizontalThumbBar = new QCheckBox(i18n("Use &horizontal thumbbar (needs to restart %1)",
                                               KGlobal::mainComponent().aboutData()->programName()),
                                          interfaceOptionsGroup);
    d->horizontalThumbBar->setWhatsThis( i18n("If this option is enabled, the thumbnails bar will be displayed "
                                              "horizontally behind the image area. You need to restart editor "
                                              "for this option take effect."));

    d->hideThumbBar = new QCheckBox(i18n("Hide &thumbbar in fullscreen mode"), interfaceOptionsGroup);
    d->hideToolBar  = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"),
                                    interfaceOptionsGroup);

    d->useRawImportTool = new QCheckBox(i18n("Use Raw Import Tool to handle Raw image"), interfaceOptionsGroup);
    d->useRawImportTool->setWhatsThis(i18n("Set this option to use Raw Import "
                                           "tool to load a RAW image. "
                                           "With this tool you are able to customize indeep decoding settings."));

    gLayout1->addWidget(d->themebackgroundColor);
    gLayout1->addWidget(d->colorBox);
    gLayout1->addWidget(d->hideToolBar);
    gLayout1->addWidget(d->hideThumbBar);
    gLayout1->addWidget(d->horizontalThumbBar);
    gLayout1->addWidget(d->useRawImportTool);
    gLayout1->setMargin(KDialog::spacingHint());
    gLayout1->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    QGroupBox *exposureOptionsGroup = new QGroupBox(i18n("Exposure Indicators"), this);
    QVBoxLayout *gLayout2           = new QVBoxLayout(exposureOptionsGroup);

    KHBox *underExpoBox         = new KHBox(exposureOptionsGroup);
    QLabel *underExpoColorlabel = new QLabel( i18n("&Under-exposure color:"), underExpoBox);
    d->underExposureColor       = new KColorButton(underExpoBox);
    underExpoColorlabel->setBuddy(d->underExposureColor);
    d->underExposureColor->setWhatsThis( i18n("Customize color used in image editor to identify "
                                              "under-exposed pixels.") );

    KHBox *overExpoBox         = new KHBox(exposureOptionsGroup);
    QLabel *overExpoColorlabel = new QLabel( i18n("&Over-exposure color:"), overExpoBox);
    d->overExposureColor       = new KColorButton(overExpoBox);
    overExpoColorlabel->setBuddy(d->overExposureColor);
    d->overExposureColor->setWhatsThis( i18n("Customize color used in image editor to identify "
                                             "over-exposed pixels.") );

    gLayout2->addWidget(underExpoBox);
    gLayout2->addWidget(overExpoBox);
    gLayout2->setMargin(KDialog::spacingHint());
    gLayout2->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    layout->addWidget(interfaceOptionsGroup);
    layout->addWidget(exposureOptionsGroup);
    layout->addStretch();
    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(d->themebackgroundColor, SIGNAL(toggled(bool)),
            this, SLOT(slotThemeBackgroundColor(bool)));

    readSettings();
}

SetupEditor::~SetupEditor()
{
    delete d;
}

void SetupEditor::slotThemeBackgroundColor(bool e)
{
    d->colorBox->setEnabled(!e);
}

void SetupEditor::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("ImageViewer Settings"));
    QColor Black(Qt::black);
    QColor White(Qt::white);
    d->themebackgroundColor->setChecked(group.readEntry("UseThemeBackgroundColor", true));
    d->backgroundColor->setColor(group.readEntry("BackgroundColor", Black));
    d->hideToolBar->setChecked(group.readEntry("FullScreen Hide ToolBar", false));
    d->hideThumbBar->setChecked(group.readEntry("FullScreenHideThumbBar", true));
    d->horizontalThumbBar->setChecked(group.readEntry("HorizontalThumbbar", false));
    d->underExposureColor->setColor(group.readEntry("UnderExposureColor", White));
    d->overExposureColor->setColor(group.readEntry("OverExposureColor", Black));
    d->useRawImportTool->setChecked(group.readEntry("UseRawImportTool", false));
}

void SetupEditor::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("ImageViewer Settings"));
    group.writeEntry("UseThemeBackgroundColor", d->themebackgroundColor->isChecked());
    group.writeEntry("BackgroundColor",         d->backgroundColor->color());
    group.writeEntry("FullScreen Hide ToolBar", d->hideToolBar->isChecked());
    group.writeEntry("FullScreenHideThumbBar",  d->hideThumbBar->isChecked());
    group.writeEntry("HorizontalThumbbar",      d->horizontalThumbBar->isChecked());
    group.writeEntry("UnderExposureColor",      d->underExposureColor->color());
    group.writeEntry("OverExposureColor",       d->overExposureColor->color());
    group.writeEntry("UseRawImportTool",        d->useRawImportTool->isChecked());
    group.sync();
}

}  // namespace Digikam
