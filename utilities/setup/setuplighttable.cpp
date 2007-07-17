/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2007-05-11
 * Description : setup Light Table tab.
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

#include <QColor>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QCheckBox>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kglobal.h>

// Local includes.

#include "setuplighttable.h"
#include "setuplighttable.moc"

namespace Digikam
{
class SetupLightTablePriv
{
public:

    SetupLightTablePriv()
    {
        hideToolBar          = 0;
        autoSyncPreview      = 0;
        autoLoadOnRightPanel = 0;
        loadFullImageSize    = 0;
    }

    QCheckBox *hideToolBar;
    QCheckBox *autoSyncPreview;
    QCheckBox *autoLoadOnRightPanel;
    QCheckBox *loadFullImageSize;
};

SetupLightTable::SetupLightTable(QWidget* parent )
               : QWidget(parent)
{
    d = new SetupLightTablePriv;

    QVBoxLayout *layout = new QVBoxLayout(this);

    // --------------------------------------------------------

    QGroupBox *interfaceOptionsGroup = new QGroupBox(i18n("Interface Options"), this);
    QVBoxLayout *gLayout             = new QVBoxLayout(interfaceOptionsGroup);

    d->autoSyncPreview = new QCheckBox(i18n("Synchronize panels automatically"), interfaceOptionsGroup);
    d->autoSyncPreview->setWhatsThis( i18n("<p>Set this option to automatically synchronize "
                     "zooming and panning between left and right panels if the images have "
                     "the same size."));

    d->autoLoadOnRightPanel = new QCheckBox(i18n("Selecting a thumbbar item loads image to the right panel"),
                                            interfaceOptionsGroup);
    d->autoLoadOnRightPanel->setWhatsThis( i18n("<p>Set this option to automatically load an image "
                     "into the right panel when the corresponding item is selected on the thumbbar."));

    d->loadFullImageSize = new QCheckBox(i18n("Load full image size"), interfaceOptionsGroup);
    d->loadFullImageSize->setWhatsThis( i18n("<p>Set this option to load full image size "
                     "in preview panel instead a reduced one. Because this option will take more time "
                     "to load image, use it only if you have a fast computer."));

    d->hideToolBar = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"), interfaceOptionsGroup);
    
    gLayout->addWidget(d->autoSyncPreview);
    gLayout->addWidget(d->autoLoadOnRightPanel);
    gLayout->addWidget(d->loadFullImageSize);
    gLayout->addWidget(d->hideToolBar);
    gLayout->setMargin(KDialog::spacingHint());
    gLayout->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    layout->addWidget(interfaceOptionsGroup);
    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    layout->addStretch();

    // --------------------------------------------------------

    readSettings();
}

SetupLightTable::~SetupLightTable()
{
    delete d;
}

void SetupLightTable::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("LightTable Settings"));
    QColor Black(Qt::black);
    QColor White(Qt::white);
    d->hideToolBar->setChecked(group.readEntry("FullScreen Hide ToolBar", false));
    d->autoSyncPreview->setChecked(group.readEntry("Auto Sync Preview", true));
    d->autoLoadOnRightPanel->setChecked(group.readEntry("Auto Load Right Panel", true));
    d->loadFullImageSize->setChecked(group.readEntry("Load Full Image size", false));
}

void SetupLightTable::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("LightTable Settings"));
    group.writeEntry("FullScreen Hide ToolBar", d->hideToolBar->isChecked());
    group.writeEntry("Auto Sync Preview", d->autoSyncPreview->isChecked());
    group.writeEntry("Auto Load Right Panel", d->autoLoadOnRightPanel->isChecked());
    group.writeEntry("Load Full Image size", d->loadFullImageSize->isChecked());
    config->sync();
}

}  // namespace Digikam
