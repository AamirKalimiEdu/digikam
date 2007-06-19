/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2003-02-01
 * Description : general configuration setup tab
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#include <qlayout.h>
#include <qcombobox.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qdir.h>
#include <qlistbox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qfileinfo.h>

// KDE includes.

#include <klocale.h>
#include <kdialogbase.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kurlrequester.h>

// // Local includes.

#include "albumsettings.h"
#include "setupgeneral.h"
#include "setupgeneral.moc"

namespace Digikam
{

class SetupGeneralPriv
{
public:

    SetupGeneralPriv()
    {
        albumPathEdit            = 0;
        iconTreeThumbSize        = 0;
        iconTreeThumbLabel       = 0;
        iconShowNameBox          = 0;
        iconShowSizeBox          = 0;
        iconShowDateBox          = 0;
        iconShowModDateBox       = 0;
        iconShowResolutionBox    = 0;
        iconShowCommentsBox      = 0;
        iconShowTagsBox          = 0;
        iconShowRatingBox        = 0;
        rightClickActionComboBox = 0;
        previewLoadFullImageSize = 0;
    }

    QLabel        *iconTreeThumbLabel;

    QCheckBox     *iconShowNameBox;
    QCheckBox     *iconShowSizeBox;
    QCheckBox     *iconShowDateBox;
    QCheckBox     *iconShowModDateBox;
    QCheckBox     *iconShowResolutionBox;
    QCheckBox     *iconShowCommentsBox;
    QCheckBox     *iconShowTagsBox;
    QCheckBox     *iconShowRatingBox;
    QCheckBox     *previewLoadFullImageSize;

    QComboBox     *iconTreeThumbSize;
    QComboBox     *rightClickActionComboBox;

    KURLRequester *albumPathEdit;

    KDialogBase   *mainDialog;
};

SetupGeneral::SetupGeneral(QWidget* parent, KDialogBase* dialog )
            : QWidget(parent)
{
    d = new SetupGeneralPriv;
    d->mainDialog       = dialog;
    QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint() );

    // --------------------------------------------------------

    QHGroupBox *albumPathBox = new QHGroupBox(parent);
    albumPathBox->setTitle(i18n("Album &Library Path"));

    d->albumPathEdit = new KURLRequester(albumPathBox);
    d->albumPathEdit->setMode(KFile::Directory | KFile::LocalOnly | KFile::ExistingOnly);    
    QToolTip::add( d->albumPathEdit, i18n("<p>Here you can set the main path to the digiKam album "
                                          "library in your computer."
                                          "<p>Write access is required for this path and do not use a "
                                          "remote path here, like an NFS mounted file system."));

    connect(d->albumPathEdit, SIGNAL(urlSelected(const QString &)),
            this, SLOT(slotChangeAlbumPath(const QString &)));

    connect(d->albumPathEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotPathEdited(const QString&)) );

    layout->addWidget(albumPathBox);

    // --------------------------------------------------------

    QVGroupBox *iconTextGroup = new QVGroupBox(i18n("Thumbnail Information"), parent);
      
    d->iconShowNameBox = new QCheckBox(i18n("Show file &name"), iconTextGroup);
    QWhatsThis::add( d->iconShowNameBox, i18n("<p>Set this option to show file name below image thumbnail."));

    d->iconShowSizeBox = new QCheckBox(i18n("Show file si&ze"), iconTextGroup);
    QWhatsThis::add( d->iconShowSizeBox, i18n("<p>Set this option to show file size below image thumbnail."));

    d->iconShowDateBox = new QCheckBox(i18n("Show file creation &date"), iconTextGroup);
    QWhatsThis::add( d->iconShowDateBox, i18n("<p>Set this option to show file creation date "
                                              "below image thumbnail."));

    d->iconShowModDateBox = new QCheckBox(i18n("Show file &modification date"), iconTextGroup);
    QWhatsThis::add( d->iconShowModDateBox, i18n("<p>Set this option to show file modification date "
                                                 "below image thumbnail."));

    d->iconShowCommentsBox = new QCheckBox(i18n("Show digiKam &comments"), iconTextGroup);
    QWhatsThis::add( d->iconShowCommentsBox, i18n("<p>Set this option to show digiKam comments "
                                                  "below image thumbnail."));

    d->iconShowTagsBox = new QCheckBox(i18n("Show digiKam &tags"), iconTextGroup);
    QWhatsThis::add( d->iconShowTagsBox, i18n("<p>Set this option to show digiKam tags "
                                              "below image thumbnail."));

    d->iconShowRatingBox = new QCheckBox(i18n("Show digiKam &rating"), iconTextGroup);
    QWhatsThis::add( d->iconShowRatingBox, i18n("<p>Set this option to show digiKam rating "
                                                "below image thumbnail."));

    d->iconShowResolutionBox = new QCheckBox(i18n("Show ima&ge dimensions (warning: slow)"), iconTextGroup);
    QWhatsThis::add( d->iconShowResolutionBox, i18n("<p>Set this option to show picture size in pixels "
                                                    "below image thumbnail."));

    layout->addWidget(iconTextGroup);

    // --------------------------------------------------------

    QVGroupBox *interfaceOptionsGroup = new QVGroupBox(i18n("Interface Options"), parent);
    interfaceOptionsGroup->setColumnLayout(0, Qt::Vertical );
    interfaceOptionsGroup->layout()->setMargin(KDialog::marginHint());
    QGridLayout* ifaceSettingsLayout = new QGridLayout(interfaceOptionsGroup->layout(), 2, 4, KDialog::spacingHint());

    d->iconTreeThumbLabel = new QLabel(i18n("Sidebar thumbnail size:"), interfaceOptionsGroup);
    d->iconTreeThumbSize = new QComboBox(false, interfaceOptionsGroup);
    d->iconTreeThumbSize->insertItem("16");
    d->iconTreeThumbSize->insertItem("22");
    d->iconTreeThumbSize->insertItem("32");
    d->iconTreeThumbSize->insertItem("48");
    QToolTip::add( d->iconTreeThumbSize, i18n("<p>Set this option to configure the size "
                                              "in pixels of the thumbnails in digiKam's sidebars. "
                                              "This option will take effect when you restart "
                                              "digiKam."));
    ifaceSettingsLayout->addMultiCellWidget(d->iconTreeThumbLabel, 0, 0, 0, 0);
    ifaceSettingsLayout->addMultiCellWidget(d->iconTreeThumbSize, 0, 0, 1, 1);

    QLabel *rightClickLabel     = new QLabel(i18n("Thumbnail click action:"), interfaceOptionsGroup);
    d->rightClickActionComboBox = new QComboBox(false, interfaceOptionsGroup);
    d->rightClickActionComboBox->insertItem(i18n("Show embedded preview"), AlbumSettings::ShowPreview);
    d->rightClickActionComboBox->insertItem(i18n("Start image editor"), AlbumSettings::StartEditor);
    QToolTip::add( d->rightClickActionComboBox, i18n("<p>Select here the right action to do when you "
                                                     "right click with mouse button on thumbnail."));
    ifaceSettingsLayout->addMultiCellWidget(rightClickLabel, 1 ,1, 0, 0);
    ifaceSettingsLayout->addMultiCellWidget(d->rightClickActionComboBox, 1, 1, 1, 4);

    d->previewLoadFullImageSize = new QCheckBox(i18n("Embedded preview load full image size"), interfaceOptionsGroup);
    QWhatsThis::add( d->previewLoadFullImageSize, i18n("<p>Set this option to load full image size "
                     "with embedded preview instead a reduced one. Because this option will take more time "
                     "to load image, use it only if you have a fast computer."));
    ifaceSettingsLayout->addMultiCellWidget(d->previewLoadFullImageSize, 2, 2, 0, 4);

    layout->addWidget(interfaceOptionsGroup);

    // --------------------------------------------------------

    layout->addStretch();

    readSettings();
    adjustSize();
}

SetupGeneral::~SetupGeneral()
{
    delete d;
}

void SetupGeneral::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    settings->setAlbumLibraryPath(d->albumPathEdit->url());

    settings->setDefaultTreeIconSize(d->iconTreeThumbSize->currentText().toInt());
    settings->setIconShowName(d->iconShowNameBox->isChecked());
    settings->setIconShowTags(d->iconShowTagsBox->isChecked());
    settings->setIconShowSize(d->iconShowSizeBox->isChecked());
    settings->setIconShowDate(d->iconShowDateBox->isChecked());
    settings->setIconShowModDate(d->iconShowModDateBox->isChecked());
    settings->setIconShowResolution(d->iconShowResolutionBox->isChecked());
    settings->setIconShowComments(d->iconShowCommentsBox->isChecked());
    settings->setIconShowRating(d->iconShowRatingBox->isChecked());

    settings->setItemRightClickAction((AlbumSettings::ItemRightClickAction)
                                      d->rightClickActionComboBox->currentItem());

    settings->setPreviewLoadFullImageSize(d->previewLoadFullImageSize->isChecked());
    settings->saveSettings();
}

void SetupGeneral::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    d->albumPathEdit->setURL(settings->getAlbumLibraryPath());

    if (settings->getDefaultTreeIconSize() == 16)
        d->iconTreeThumbSize->setCurrentItem(0);
    else if (settings->getDefaultTreeIconSize() == 22)
        d->iconTreeThumbSize->setCurrentItem(1);
    else if (settings->getDefaultTreeIconSize() == 32)
        d->iconTreeThumbSize->setCurrentItem(2);
    else 
        d->iconTreeThumbSize->setCurrentItem(3);
    
    d->iconShowNameBox->setChecked(settings->getIconShowName());
    d->iconShowTagsBox->setChecked(settings->getIconShowTags());
    d->iconShowSizeBox->setChecked(settings->getIconShowSize());
    d->iconShowDateBox->setChecked(settings->getIconShowDate());
    d->iconShowModDateBox->setChecked(settings->getIconShowModDate());
    d->iconShowResolutionBox->setChecked(settings->getIconShowResolution());
    d->iconShowCommentsBox->setChecked(settings->getIconShowComments());
    d->iconShowRatingBox->setChecked(settings->getIconShowRating());

    d->rightClickActionComboBox->setCurrentItem((int)settings->getItemRightClickAction());

    d->previewLoadFullImageSize->setChecked(settings->getPreviewLoadFullImageSize());
}

void SetupGeneral::slotChangeAlbumPath(const QString &result)
{
    if (KURL(result).equals(KURL(QDir::homeDirPath()), true)) 
    {
        KMessageBox::sorry(0, i18n("Sorry; cannot use home directory as album library."));
        return;
    }

    QFileInfo targetPath(result);

    if (!result.isEmpty() && !targetPath.isWritable()) 
    {
        KMessageBox::information(0, i18n("No write access for this path.\n"
                                         "Warning: the comment and tag features will not work."));
    }
}

void SetupGeneral::slotPathEdited(const QString& newPath)
{
    if (newPath.isEmpty()) 
    {
       d->mainDialog->enableButtonOK(false);
       return;
    }

    if (!newPath.startsWith("/")) 
    {
        d->albumPathEdit->setURL(QDir::homeDirPath() + '/' + newPath);
    }

    QFileInfo targetPath(newPath);
    QDir dir(newPath);
    d->mainDialog->enableButtonOK(dir.exists() && dir.path() != QDir::homeDirPath());
}

}  // namespace Digikam

