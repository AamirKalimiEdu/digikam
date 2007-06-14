/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2004-08-03
 * Description : setup Image Editor tab.
 *
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qlayout.h>
#include <qcolor.h>
#include <q3hbox.h>
#include <q3vgroupbox.h>
#include <qlabel.h>
#include <q3whatsthis.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>
#include <klistview.h>
#include <ktrader.h>

// Local includes.

#include "setupeditor.h"
#include "setupeditor.moc"

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
    }

    Q3HBox        *colorBox;

    QCheckBox    *hideToolBar;
    QCheckBox    *themebackgroundColor;

    KColorButton *backgroundColor;
    KColorButton *underExposureColor;
    KColorButton *overExposureColor;
};

SetupEditor::SetupEditor(QWidget* parent )
           : QWidget(parent)
{
    d = new SetupEditorPriv;
    Q3VBoxLayout *layout = new Q3VBoxLayout( parent, 0, KDialog::spacingHint() );

    // --------------------------------------------------------

    Q3VGroupBox *interfaceOptionsGroup = new Q3VGroupBox(i18n("Interface Options"), parent);

    d->themebackgroundColor = new QCheckBox(i18n("&Use theme background color"),
                                            interfaceOptionsGroup);

    Q3WhatsThis::add( d->themebackgroundColor, i18n("<p>Enable this option to use the background theme "
                                              "color in the image editor area") );

    d->colorBox = new Q3HBox(interfaceOptionsGroup);

    QLabel *backgroundColorlabel = new QLabel( i18n("&Background color:"), d->colorBox );

    d->backgroundColor = new KColorButton(d->colorBox);
    backgroundColorlabel->setBuddy(d->backgroundColor);
    Q3WhatsThis::add( d->backgroundColor, i18n("<p>Customize the background color to use "
                                              "in the image editor area.") );

    d->hideToolBar = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"),
                                   interfaceOptionsGroup);

    // --------------------------------------------------------

    Q3VGroupBox *exposureOptionsGroup = new Q3VGroupBox(i18n("Exposure Indicators"), parent);

    Q3HBox *underExpoBox         = new Q3HBox(exposureOptionsGroup);
    QLabel *underExpoColorlabel = new QLabel( i18n("&Under-exposure color:"), underExpoBox);
    d->underExposureColor       = new KColorButton(underExpoBox);
    underExpoColorlabel->setBuddy(d->underExposureColor);
    Q3WhatsThis::add( d->underExposureColor, i18n("<p>Customize the color used in image editor to identify "
                                                 "the under-exposed pixels.") );

    Q3HBox *overExpoBox         = new Q3HBox(exposureOptionsGroup);
    QLabel *overExpoColorlabel = new QLabel( i18n("&Over-exposure color:"), overExpoBox);
    d->overExposureColor       = new KColorButton(overExpoBox);
    overExpoColorlabel->setBuddy(d->overExposureColor);
    Q3WhatsThis::add( d->overExposureColor, i18n("<p>Customize the color used in image editor to identify "
                                                "the over-exposed pixels.") );

    layout->addWidget(interfaceOptionsGroup);
    layout->addWidget(exposureOptionsGroup);
    layout->addStretch();

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
    KConfig* config = kapp->config();
    QColor Black(Qt::black);
    QColor White(Qt::white);
    config->setGroup("ImageViewer Settings");
    d->themebackgroundColor->setChecked(config->readBoolEntry("UseThemeBackgroundColor", true));
    d->backgroundColor->setColor(config->readColorEntry("BackgroundColor", &Black));
    d->hideToolBar->setChecked(config->readBoolEntry("FullScreen Hide ToolBar", false));
    d->underExposureColor->setColor(config->readColorEntry("UnderExposureColor", &White));
    d->overExposureColor->setColor(config->readColorEntry("OverExposureColor", &Black));
}

void SetupEditor::applySettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    config->writeEntry("UseThemeBackgroundColor", d->themebackgroundColor->isChecked());
    config->writeEntry("BackgroundColor", d->backgroundColor->color());
    config->writeEntry("FullScreen Hide ToolBar", d->hideToolBar->isChecked());
    config->writeEntry("UnderExposureColor", d->underExposureColor->color());
    config->writeEntry("OverExposureColor", d->overExposureColor->color());
    config->sync();
}

}  // namespace Digikam

