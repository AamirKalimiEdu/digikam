/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-01-02
 * Description : collection setup tab.
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

#include <QButtonGroup>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDir>
#include <QGridLayout>
#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kpagedialog.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <klistwidget.h>

// // Local includes.

#include "thumbnailsize.h"
#include "albumsettings.h"
#include "setupcollections.h"
#include "setupcollections.moc"

namespace Digikam
{

class SetupCollectionsPriv
{
public:

    SetupCollectionsPriv()
    {
        albumCollectionBox  = 0;
        addCollectionButton = 0;
        delCollectionButton = 0;
    }

    KListWidget *albumCollectionBox;

    QPushButton *addCollectionButton;
    QPushButton *delCollectionButton;
};

SetupCollections::SetupCollections(QWidget* parent )
                : QWidget(parent)
{
    d = new SetupCollectionsPriv;
    QVBoxLayout *mainLayout            = new QVBoxLayout(this);
    QGridLayout *collectionGroupLayout = new QGridLayout( this );
    collectionGroupLayout->setSpacing( KDialog::spacingHint() );
    collectionGroupLayout->setAlignment( Qt::AlignTop );

   // --------------------------------------------------------

   d->albumCollectionBox = new KListWidget(this);
   d->albumCollectionBox->setWhatsThis( i18n("<p>You can add or remove Album "
                                             "collection types here to improve how "
                                             "your Albums are sorted in digiKam."));

   d->albumCollectionBox->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

   collectionGroupLayout->addWidget( d->albumCollectionBox, 0, 4, 0, 0 );

   d->addCollectionButton = new QPushButton( i18n("&Add..."), this);
   collectionGroupLayout->addWidget( d->addCollectionButton, 0, 1);

   d->delCollectionButton = new QPushButton( i18n("&Delete"), this);
   collectionGroupLayout->addWidget( d->delCollectionButton, 1, 1);
   d->delCollectionButton->setEnabled(false);

   connect(d->albumCollectionBox, SIGNAL(selectionChanged()),
           this, SLOT(slotCollectionSelectionChanged()));

   connect(d->addCollectionButton, SIGNAL(clicked()),
           this, SLOT(slotAddCollection()));

   connect(d->delCollectionButton, SIGNAL(clicked()),
           this, SLOT(slotDelCollection()));

   QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum,
                                          QSizePolicy::Expanding );
   collectionGroupLayout->addItem( spacer, 4, 1 );

   mainLayout->addWidget(this);

   // --------------------------------------------------------

   readSettings();
   adjustSize();
}

SetupCollections::~SetupCollections()
{
    delete d;
}

void SetupCollections::applySettings()
{
    AlbumSettings* settings = AlbumSettings::componentData();

    if (!settings) return;

    QStringList collectionList;

    for (int i = 0 ; i < d->albumCollectionBox->count(); i++)
    {
        QListWidgetItem *item = d->albumCollectionBox->item(i);
        collectionList.append(item->text());
    }

    settings->setAlbumCollectionNames(collectionList);
    settings->saveSettings();
}

void SetupCollections::readSettings()
{
    AlbumSettings* settings = AlbumSettings::componentData();
    if (!settings) return;

    d->albumCollectionBox->insertItems(0, settings->getAlbumCollectionNames());
}

void SetupCollections::slotCollectionSelectionChanged()
{
    if (d->albumCollectionBox->currentItem())
        d->delCollectionButton->setEnabled(true);
    else
        d->delCollectionButton->setEnabled(false);
}

void SetupCollections::slotAddCollection()
{
    bool ok;

    QString newCollection = KInputDialog::getText(i18n("New Collection Name"),
                                                  i18n("Enter new collection name:"),
                                                  QString(), &ok, this);

    if (!ok) return;

    bool found = false;
    for (int i = 0 ; i < d->albumCollectionBox->count(); i++)
    {
        QListWidgetItem *item = d->albumCollectionBox->item(i);
        if (newCollection == item->text()) 
        {
            found = true;
            break;
        }
    }

    if (!found)
        d->albumCollectionBox->insertItem(d->albumCollectionBox->count(), newCollection);
}

void SetupCollections::slotDelCollection()
{
    QListWidgetItem *item = d->albumCollectionBox->currentItem();
    if (!item) return;
    d->albumCollectionBox->takeItem(d->albumCollectionBox->row(item));
    delete item;
}

}  // namespace Digikam

