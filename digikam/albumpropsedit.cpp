/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2003-03-09
 * Description : Album properties dialog.
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005 by Tom Albers <tomalbers@kde.nl>
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

// Qt includes.

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QFrame>
#include <QPushButton>
#include <QGridLayout>
#include <QTextEdit>

// KDE includes.

#include <kdatepicker.h>
#include <ktextedit.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kcursor.h>
#include <kdeversion.h>
#include <kinputdialog.h>
#include <khbox.h>

// Local includes.

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "databaseaccess.h"
#include "albumpropsedit.h"
#include "albumpropsedit.moc"

namespace Digikam
{

class AlbumPropsEditPriv
{

public:

    AlbumPropsEditPriv()
    {
        titleEdit          = 0;
        collectionCombo    = 0;
        commentsEdit       = 0;
        datePicker         = 0;
        album              = 0;
    }

    QStringList     albumCollections;

    QComboBox      *collectionCombo;

    KLineEdit      *titleEdit;

    KTextEdit      *commentsEdit;

    KDatePicker    *datePicker;

    PAlbum         *album;
};

AlbumPropsEdit::AlbumPropsEdit(PAlbum* album, bool create)
              : KDialog( 0L )
{
    setCaption(create ? i18n("New Album") : i18n("Edit Album"));
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setModal(true);
    setHelp("albumpropsedit.anchor", "digikam");

    d = new AlbumPropsEditPriv;
    d->album = album;

    QWidget *page = new QWidget(this);
    setMainWidget(page);

    QGridLayout *topLayout = new QGridLayout(page);

    QLabel *topLabel = new QLabel( page );
    if (create)
    {
        topLabel->setText(i18n("<qt><b>Create new Album in \"<i>%1</i>\"</b></qt>", album->title()));
    }
    else
    {
        topLabel->setText(i18n("<qt><b>\"<i>%1</i>\" Album Properties</b></qt>", album->title()));
    }
    topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine);

    // --------------------------------------------------------

    QFrame *topLine = new QFrame(page);
    topLine->setFrameShape(QFrame::HLine);
    topLine->setFrameShadow(QFrame::Sunken);

    // --------------------------------------------------------

    QLabel *titleLabel = new QLabel(page);
    titleLabel->setText(i18n("&Title:"));

    d->titleEdit = new KLineEdit(page);
    titleLabel->setBuddy(d->titleEdit);

    QLabel *collectionLabel = new QLabel(page);
    collectionLabel->setText(i18n("Co&llection:"));

    d->collectionCombo = new QComboBox(page);
    d->collectionCombo->setEditable(true);
    collectionLabel->setBuddy(d->collectionCombo);

    QLabel *commentsLabel = new QLabel(page);
    commentsLabel->setText(i18n("Co&mments:"));

    d->commentsEdit = new KTextEdit(page);
    commentsLabel->setBuddy(d->commentsEdit);
    d->commentsEdit->setCheckSpellingEnabled(true);
    d->commentsEdit->setWordWrapMode(QTextOption::WordWrap);

    QLabel *dateLabel = new QLabel(page);
    dateLabel->setText(i18n("Album &date:"));

    d->datePicker = new KDatePicker(page);
    dateLabel->setBuddy(d->datePicker);

    KHBox *buttonRow = new KHBox(page);
    QPushButton *dateLowButton = new QPushButton(i18nc("Selects the date of the oldest image",
                                                 "&Oldest"), buttonRow);
    QPushButton *dateAvgButton = new QPushButton(i18nc("Calculates the average date",
                                                 "&Average"), buttonRow);
    QPushButton *dateHighButton = new QPushButton(i18nc("Selects the date of the newest image",
                                                  "Newest"), buttonRow);
        
    setTabOrder(d->titleEdit, d->collectionCombo);
    setTabOrder(d->collectionCombo, d->commentsEdit);
    setTabOrder(d->commentsEdit, d->datePicker);
    d->commentsEdit->setTabChangesFocus(true);
    d->titleEdit->selectAll();
    d->titleEdit->setFocus();

    // --------------------------------------------------------

    topLayout->addWidget( topLabel, 0, 0, 1, 2 );
    topLayout->addWidget( topLine, 1, 0, 1, 2 );
    topLayout->addWidget( titleLabel, 2, 0, 1, 1);
    topLayout->addWidget( d->titleEdit, 2, 1, 1, 1);
    topLayout->addWidget( collectionLabel, 3, 0, 1, 1);
    topLayout->addWidget( d->collectionCombo, 3, 1, 1, 1);
    topLayout->addWidget( commentsLabel, 4, 0, 1, 1, Qt::AlignLeft|Qt::AlignTop );
    topLayout->addWidget( d->commentsEdit, 4, 1, 1, 1);
    topLayout->addWidget( dateLabel, 5, 0, 1, 1, Qt::AlignLeft|Qt::AlignTop );
    topLayout->addWidget( d->datePicker, 5, 1, 1, 1);
    topLayout->addWidget( buttonRow, 6, 1, 1, 1);
    topLayout->setMargin(0);
    topLayout->setSpacing(KDialog::spacingHint());

    // Initialize ---------------------------------------------

    AlbumSettings *settings = AlbumSettings::componentData();
    if (settings)
    {
        d->collectionCombo->addItem( QString() );
        QStringList collections = settings->getAlbumCollectionNames();
        d->collectionCombo->addItems( collections );
        int collectionIndex = collections.indexOf( album->collection() );
        
        if ( collectionIndex != -1 )
        {
            // + 1 because of the empty item
            d->collectionCombo->setCurrentIndex(collectionIndex + 1);
        }
    }

    if (create)
    {
        d->titleEdit->setText( i18n("New Album") );
        d->datePicker->setDate( QDate::currentDate() );
    }
    else
    {
        d->titleEdit->setText( album->title() );
        d->commentsEdit->setText( album->caption() );
        d->datePicker->setDate( album->date() );
    }

    // -- slots connections -------------------------------------------

    connect(d->titleEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTitleChanged(const QString&)));
            
    connect(dateLowButton, SIGNAL( clicked() ),
            this, SLOT( slotDateLowButtonClicked()));
            
    connect(dateAvgButton, SIGNAL( clicked() ),
            this, SLOT( slotDateAverageButtonClicked()));
            
    connect(dateHighButton, SIGNAL( clicked() ),
            this, SLOT( slotDateHighButtonClicked()));
    
    adjustSize();
}

AlbumPropsEdit::~AlbumPropsEdit()
{
    delete d;
}

QString AlbumPropsEdit::title() const
{
    return d->titleEdit->text();
}

QString AlbumPropsEdit::comments() const
{
    return d->commentsEdit->document()->toPlainText();
}

QDate AlbumPropsEdit::date() const
{
    return d->datePicker->date();
}

QString AlbumPropsEdit::collection() const
{
    QString name = d->collectionCombo->currentText();

    if (name.isEmpty())
    {
        name = i18n( "Uncategorized Album" );
    }

    return name;
}

QStringList AlbumPropsEdit::albumCollections() const
{
    QStringList collections;
    AlbumSettings *settings = AlbumSettings::componentData();
    if (settings)
    {
        collections = settings->getAlbumCollectionNames();
    }

    QString currentCollection = d->collectionCombo->currentText();
    if ( collections.indexOf( currentCollection ) == -1 )
    {
        collections.append(currentCollection);
    }

    collections.sort();
    return collections;
}

bool AlbumPropsEdit::editProps(PAlbum *album, QString& title,
                               QString& comments, QDate& date, QString& collection,
                               QStringList& albumCollections)
{
    AlbumPropsEdit dlg(album);

    bool ok = dlg.exec() == QDialog::Accepted;

    title            = dlg.title();
    comments         = dlg.comments();
    date             = dlg.date();
    collection       = dlg.collection();
    albumCollections = dlg.albumCollections();

    return ok;
}

bool AlbumPropsEdit::createNew(PAlbum *parent, QString& title, QString& comments,
                               QDate& date, QString& collection, QStringList& albumCollections)
{
    AlbumPropsEdit dlg(parent, true);

    bool ok = dlg.exec() == QDialog::Accepted;

    title            = dlg.title();
    comments         = dlg.comments();
    date             = dlg.date();
    collection       = dlg.collection();
    albumCollections = dlg.albumCollections();

    return ok;
}

void AlbumPropsEdit::slotTitleChanged(const QString& newtitle)
{
    enableButtonOk(!newtitle.isEmpty());    
}

void AlbumPropsEdit::slotDateLowButtonClicked()
{
    setCursor( Qt::WaitCursor );

    QDate lowDate = DatabaseAccess().db()->getAlbumLowestDate( d->album->id() );

    setCursor( Qt::ArrowCursor );

    if ( lowDate.isValid() )
        d->datePicker->setDate( lowDate );
}

void AlbumPropsEdit::slotDateHighButtonClicked()
{
    setCursor( Qt::WaitCursor );

    QDate highDate = DatabaseAccess().db()->getAlbumHighestDate( d->album->id() );

    setCursor( Qt::ArrowCursor );

    if ( highDate.isValid() )
        d->datePicker->setDate( highDate );
}

void AlbumPropsEdit::slotDateAverageButtonClicked()
{
    setCursor( Qt::WaitCursor );

    QDate avDate = DatabaseAccess().db()->getAlbumAverageDate( d->album->id() );

    setCursor( Qt::ArrowCursor );

    if ( avDate.isValid() )
        d->datePicker->setDate( avDate );
    else
        KMessageBox::error( this,
                            i18n( "Could not calculate an average."),
                            i18n( "Could Not Calculate Average" ) );
}

}  // namespace Digikam
