/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-19
 * Description : a options group to set renaming files
 *               operations during camera downloading
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#include <QDateTime>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>

// KDE includes.

#include <khbox.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kdialog.h>
#include <kinputdialog.h>

// Local includes.

#include "renamecustomizer.h"
#include "renamecustomizer.moc"

namespace Digikam
{

class RenameCustomizerPriv
{
public:

    enum DateFormatOptions
    {
        DigikamStandard = 0,
        IsoDateFormat,
        TextDateFormat,
        LocalDateFormat,
        Advanced
    };

    RenameCustomizerPriv()
    {
        renameDefault         = 0;
        renameCustom          = 0;
        renameDefaultBox      = 0;
        renameCustomBox       = 0;
        renameDefaultCase     = 0;
        renameDefaultCaseType = 0;
        addDateTimeBox        = 0;
        addCameraNameBox      = 0;
        addSeqNumberBox       = 0;
        changedTimer          = 0;
        renameCustomPrefix    = 0;
        renameCustomSuffix    = 0;
        startIndexLabel       = 0;
        startIndexInput       = 0;
        focusedWidget         = 0;
        dateTimeButton        = 0;
        dateTimeLabel         = 0;
        dateTimeFormat        = 0;
}

    QWidget      *focusedWidget;

    QString       cameraTitle;

    QButtonGroup *buttonGroup;

    QRadioButton *renameDefault;
    QRadioButton *renameCustom;

    QLabel       *renameDefaultCase;
    QLabel       *startIndexLabel;
    QLabel       *dateTimeLabel;

    QComboBox    *renameDefaultCaseType;
    QComboBox    *dateTimeFormat;

    QCheckBox    *addDateTimeBox;
    QCheckBox    *addCameraNameBox;
    QCheckBox    *addSeqNumberBox;

    QPushButton  *dateTimeButton;

    QString       dateTimeFormatString;

    QTimer       *changedTimer;

    KVBox        *renameDefaultBox;
    KVBox        *renameCustomBox;

    KLineEdit    *renameCustomPrefix;
    KLineEdit    *renameCustomSuffix;

    KIntNumInput *startIndexInput;
};

RenameCustomizer::RenameCustomizer(QWidget* parent, const QString& cameraTitle)
                : KVBox(parent)
{
    d = new RenameCustomizerPriv;
    d->changedTimer = new QTimer();
    d->cameraTitle  = cameraTitle;
    d->buttonGroup  = new QButtonGroup(this);

    setAttribute(Qt::WA_DeleteOnClose);

    d->buttonGroup->setExclusive(true);

    QGridLayout* mainLayout = new QGridLayout(this);

    // ----------------------------------------------------------------

    d->renameDefault = new QRadioButton(i18n("Camera filenames"), this);
    d->buttonGroup->addButton(d->renameDefault);
    d->renameDefault->setWhatsThis( i18n("<p>Turn on this option to use camera "
                                         "provided image filenames without modifications."));

    d->renameDefaultBox = new KVBox(this);
    d->renameDefaultBox->setMargin(0);
    d->renameDefaultBox->setSpacing(KDialog::spacingHint());

    d->renameDefaultCase = new QLabel( i18n("Change case to:"), d->renameDefaultBox );
    d->renameDefaultCase->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );

    d->renameDefaultCaseType = new QComboBox( d->renameDefaultBox );
    d->renameDefaultCaseType->insertItem(0, i18n("Leave as Is"));
    d->renameDefaultCaseType->insertItem(1, i18n("Upper"));
    d->renameDefaultCaseType->insertItem(2, i18n("Lower"));
    d->renameDefaultCaseType->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    d->renameDefaultCaseType->setWhatsThis( i18n("<p>Set the method to use to change the case "
                                                 "of image filenames."));
                                           
    QHBoxLayout* boxLayout1 = new QHBoxLayout(d->renameDefaultBox);
    boxLayout1->setMargin(KDialog::spacingHint());
    boxLayout1->setSpacing(KDialog::spacingHint());
    boxLayout1->addSpacing( 10 );
    boxLayout1->addWidget( d->renameDefaultCase );
    boxLayout1->addWidget( d->renameDefaultCaseType );

    // -------------------------------------------------------------

    d->renameCustom = new QRadioButton(i18n("Customize"), this);
    d->buttonGroup->addButton(d->renameCustom);
    d->renameCustom->setWhatsThis( i18n("<p>Turn on this option to customize image filenames "
                                        "during download."));

    d->renameCustomBox = new KVBox(this);
    d->renameCustomBox->setMargin(0);
    d->renameCustomBox->setSpacing(KDialog::spacingHint());

    QGridLayout* renameCustomBoxLayout = new QGridLayout(d->renameCustomBox);

    QLabel* prefixLabel   = new QLabel(i18n("Prefix:"), d->renameCustomBox);
    d->renameCustomPrefix = new KLineEdit(d->renameCustomBox);
    d->focusedWidget = d->renameCustomPrefix;
    d->renameCustomPrefix->setWhatsThis( i18n("<p>Set the prefix which will be prepended to "
                                              "image filenames."));

    QLabel* suffixLabel   = new QLabel(i18n("Suffix:"), d->renameCustomBox);
    d->renameCustomSuffix = new KLineEdit(d->renameCustomBox);
    d->renameCustomSuffix->setWhatsThis( i18n("<p>Set the suffix which will be postpended to "
                                              "image filenames."));

    d->addDateTimeBox = new QCheckBox( i18n("Add Date && Time"), d->renameCustomBox );
    d->addDateTimeBox->setWhatsThis( i18n("<p>Set this option to add the camera provided date and time."));

    QWidget *dateTimeWidget = new QWidget(d->renameCustomBox);
    d->dateTimeLabel        = new QLabel(i18n("Date format:"), dateTimeWidget);
    d->dateTimeFormat       = new QComboBox(dateTimeWidget);
    d->dateTimeFormat->insertItem(RenameCustomizerPriv::DigikamStandard, i18n("Standard"));
    d->dateTimeFormat->insertItem(RenameCustomizerPriv::IsoDateFormat,   i18n("ISO"));
    d->dateTimeFormat->insertItem(RenameCustomizerPriv::TextDateFormat,  i18n("Full Text"));
    d->dateTimeFormat->insertItem(RenameCustomizerPriv::LocalDateFormat, i18n("Local Settings"));
    d->dateTimeFormat->insertItem(RenameCustomizerPriv::Advanced,        i18n("Advanced..."));
    d->dateTimeFormat->setWhatsThis( i18n("<p>Select your preferred date format used to "
                    "create new albums. The options available are:</p>"
                    "<p><b>Standard</b>: the date format that has been used as a standard by digiKam. "
                    "E.g.: <i>20060824T142618</i></p>"
                    "<p/><b>ISO</b>: the date format is in accordance with ISO 8601 "
                    "(YYYY-MM-DD). E.g.: <i>2006-08-24T14:26:18</i></p>"
                    "<p><b>Full Text</b>: the date format is in a user-readable string. "
                    "E.g.: <i>Thu Aug 24 14:26:18 2006</i></p>"
                    "<p><b>Local Settings</b>: the date format depending on KDE control panel settings.</p>"
                    "<p><b>Advanced:</b> allows to specify a custom date format.</p>"));

    d->dateTimeButton = new QPushButton(SmallIcon("configure"), QString(), dateTimeWidget);
    QSizePolicy policy = d->dateTimeButton->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Maximum);
    d->dateTimeButton->setSizePolicy(policy);

    QHBoxLayout *boxLayout2 = new QHBoxLayout(dateTimeWidget);
    boxLayout2->addWidget(d->dateTimeLabel);
    boxLayout2->addWidget(d->dateTimeFormat);
    boxLayout2->addWidget(d->dateTimeButton);
    boxLayout2->setMargin(KDialog::spacingHint());
    boxLayout2->setSpacing(KDialog::spacingHint());

    d->addCameraNameBox = new QCheckBox( i18n("Add Camera Name"), d->renameCustomBox );
    d->addCameraNameBox->setWhatsThis( i18n("<p>Set this option to add the camera name."));

    d->addSeqNumberBox = new QCheckBox( i18n("Add Sequence Number"), d->renameCustomBox );
    d->addSeqNumberBox->setWhatsThis( i18n("<p>Set this option to add a sequence number "
                                           "starting with the index set below."));

    d->startIndexLabel = new QLabel( i18n("Start Index:"), d->renameCustomBox );
    d->startIndexInput = new KIntNumInput(1, d->renameCustomBox);
    d->startIndexInput->setRange(1, 900000, 1, false);
    d->startIndexInput->setWhatsThis( i18n("<p>Set the starting index value used to rename picture "
                                           "files with a sequence number."));

    renameCustomBoxLayout->addWidget(prefixLabel, 0, 1, 1, 1);
    renameCustomBoxLayout->addWidget(d->renameCustomPrefix, 0, 2, 1, 1);
    renameCustomBoxLayout->addWidget(suffixLabel, 1, 1, 1, 1);
    renameCustomBoxLayout->addWidget(d->renameCustomSuffix, 1, 2, 1, 1);
    renameCustomBoxLayout->addWidget(d->addDateTimeBox, 2, 1, 1, 2);
    renameCustomBoxLayout->addWidget(dateTimeWidget, 3, 1, 1, 2);
    renameCustomBoxLayout->addWidget(d->addCameraNameBox, 4, 1, 1, 2);
    renameCustomBoxLayout->addWidget(d->addSeqNumberBox, 5, 1, 1, 2);
    renameCustomBoxLayout->addWidget(d->startIndexLabel, 6, 1, 1, 1);
    renameCustomBoxLayout->addWidget(d->startIndexInput, 6, 2, 1, 1);
    renameCustomBoxLayout->setColumnMinimumWidth(0, 10);
    renameCustomBoxLayout->setMargin(KDialog::spacingHint());
    renameCustomBoxLayout->setSpacing(KDialog::spacingHint());

    // ----------------------------------------------------------------------

    mainLayout->addWidget(d->renameDefaultBox, 1, 0, 1, 2 );
    mainLayout->addWidget(d->renameCustom, 2, 0, 1, 2 );
    mainLayout->addWidget(d->renameDefault, 0, 0, 1, 2 );
    mainLayout->addWidget(d->renameCustomBox, 3, 0, 1, 2 );
    mainLayout->setRowStretch(4, 10);
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setSpacing(KDialog::spacingHint());

    // -- setup connections -------------------------------------------------

    connect(this, SIGNAL(buttonClicked(int)),
            this, SLOT(slotRadioButtonClicked(int)));
            
    connect(d->renameCustomPrefix, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->renameCustomSuffix, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->addDateTimeBox, SIGNAL(toggled(bool)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->addCameraNameBox, SIGNAL(toggled(bool)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->addSeqNumberBox, SIGNAL(toggled(bool)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->renameDefaultCaseType, SIGNAL(activated(const QString&)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->startIndexInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotRenameOptionsChanged()));

    connect(d->changedTimer, SIGNAL(timeout()),
            this, SIGNAL(signalChanged()));

    connect(d->dateTimeButton, SIGNAL(clicked()),
            this, SLOT(slotDateTimeButtonClicked()));

    connect(d->dateTimeFormat, SIGNAL(activated(int)),
            this, SLOT(slotDateTimeFormatChanged(int)));

    connect(d->addDateTimeBox, SIGNAL(toggled(bool)),
            this, SLOT(slotDateTimeBoxToggled(bool)));

    // -- initial values ---------------------------------------------------

    readSettings();

    // signal to this not yet connected when readSettings is called? Don't know
    slotDateTimeBoxToggled(d->addDateTimeBox->isChecked());
}

RenameCustomizer::~RenameCustomizer()
{
    delete d->changedTimer;
    saveSettings();
    delete d;
}

bool RenameCustomizer::useDefault() const
{
    return d->renameDefault->isChecked();
}

int RenameCustomizer::startIndex() const
{
    return d->startIndexInput->value();
}

QString RenameCustomizer::newName(const QDateTime &dateTime, int index, const QString &extension) const
{
    if (d->renameDefault->isChecked())
        return QString();
    else
    {
        QString name(d->renameCustomPrefix->text());

        // use the "T" as a delimiter between date and time
        QString date;
        switch (d->dateTimeFormat->currentIndex())
        {
            case RenameCustomizerPriv::DigikamStandard:
                date = dateTime.toString("yyyyMMddThhmmss");
                break;
            case RenameCustomizerPriv::TextDateFormat:
                date = dateTime.toString(Qt::TextDate);
                break;
            case RenameCustomizerPriv::LocalDateFormat:
                date = dateTime.toString(Qt::LocalDate);
                break;
            case RenameCustomizerPriv::IsoDateFormat:
                date = dateTime.toString(Qt::ISODate);
                break;
            case RenameCustomizerPriv::Advanced:
                date = dateTime.toString(d->dateTimeFormatString);
                break;
         }

        // it seems that QString::number does not support padding with zeros
        QString seq;
        seq.sprintf("-%06d", index);

        if (d->addDateTimeBox->isChecked())
            name += date;

        if (d->addSeqNumberBox->isChecked())
            name += seq;

        if (d->addCameraNameBox->isChecked())
            name += QString("-%1").arg(d->cameraTitle.simplified().replace(" ", ""));

        name += d->renameCustomSuffix->text();
        name += extension;

        return name;
    }
}

RenameCustomizer::Case RenameCustomizer::changeCase() const
{
    RenameCustomizer::Case type = NONE;

    if (d->renameDefaultCaseType->currentIndex() == 1)
        type=UPPER;
    if (d->renameDefaultCaseType->currentIndex() == 2)
        type=LOWER;

    return type;
}

void RenameCustomizer::slotRadioButtonClicked(int)
{
    QRadioButton* btn = dynamic_cast<QRadioButton*>(d->buttonGroup->checkedButton());
    if (!btn)
        return;

    d->renameCustomBox->setEnabled( btn != d->renameDefault );
    d->renameDefaultBox->setEnabled( btn == d->renameDefault );
    slotRenameOptionsChanged();
}

void RenameCustomizer::slotRenameOptionsChanged()
{
    d->focusedWidget = focusWidget();

    if (d->addSeqNumberBox->isChecked())
    {
        d->startIndexInput->setEnabled(true);
        d->startIndexLabel->setEnabled(true);
    }
    else
    {
        d->startIndexInput->setEnabled(false);
        d->startIndexLabel->setEnabled(false);
    }
    
    d->changedTimer->setSingleShot(true);
    d->changedTimer->start(500);
}

void RenameCustomizer::slotDateTimeBoxToggled(bool on)
{
    d->dateTimeLabel->setEnabled(on);
    d->dateTimeFormat->setEnabled(on);
    d->dateTimeButton->setEnabled(on
            && d->dateTimeFormat->currentIndex() == RenameCustomizerPriv::Advanced);
    slotRenameOptionsChanged();
}

void RenameCustomizer::slotDateTimeFormatChanged(int index)
{
    if (index == RenameCustomizerPriv::Advanced)
    {
        d->dateTimeButton->setEnabled(true);
        //d->dateTimeButton->show();
        //slotDateTimeButtonClicked();
    }
    else
    {
        d->dateTimeButton->setEnabled(false);
        //d->dateTimeButton->hide();
    }
    slotRenameOptionsChanged();
}

void RenameCustomizer::slotDateTimeButtonClicked()
{
    bool ok;
    QString message = i18n("<qt><p>Enter the format for date and time.</p>"
                           "<p>Use <i>dd</i> for the day, "
                           "<i>MM</i> for the month, "
                           "<i>yyyy</i> for the year, "
                           "<i>hh</i> for the hour, "
                           "<i>mm</i> for the minute, "
                           "<i>ss</i> for the second.</p>"
                           "<p>Examples: <i>yyyyMMddThhmmss</i> "
                           "for 20060824T142418,<br>"
                           "<i>yyyy-MM-dd hh:mm:ss</i> "
                           "for 2006-08-24 14:24:18.</p></qt>");

    QString newFormat = KInputDialog::getText(i18n("Change Date and Time Format"),
                                              message,
                                              d->dateTimeFormatString, &ok, this);
    if (!ok)
        return;

    d->dateTimeFormatString = newFormat;
    slotRenameOptionsChanged();
}

void RenameCustomizer::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    
    KConfigGroup group = config->group("Camera Settings");
    bool def         = group.readEntry("Rename Use Default", true);
    bool addSeqNumb  = group.readEntry("Add Sequence Number", true);
    bool adddateTime = group.readEntry("Add Date Time", false);
    bool addCamName  = group.readEntry("Add Camera Name", false);
    int chcaseT      = group.readEntry("Case Type", (int)NONE);
    QString prefix   = group.readEntry("Rename Prefix", i18n("photo"));
    QString suffix   = group.readEntry("Rename Postfix", QString());
    int startIndex   = group.readEntry("Rename Start Index", 1);
    int dateTime     = group.readEntry("Date Time Format", (int)RenameCustomizerPriv::IsoDateFormat);
    QString format   = group.readEntry("Date Time Format String", "yyyyMMddThhmmss");

    if (def)
    {
        d->renameDefault->setChecked(true);
        d->renameCustom->setChecked(false);
        d->renameCustomBox->setEnabled(false);
        d->renameDefaultBox->setEnabled(true);
    }
    else
    {
        d->renameDefault->setChecked(false);
        d->renameCustom->setChecked(true);
        d->renameCustomBox->setEnabled(true);
        d->renameDefaultBox->setEnabled(false);
    }

    d->addDateTimeBox->setChecked(adddateTime);
    d->addCameraNameBox->setChecked(addCamName);
    d->addSeqNumberBox->setChecked(addSeqNumb);
    d->renameDefaultCaseType->setCurrentIndex(chcaseT);
    d->renameCustomPrefix->setText(prefix);
    d->renameCustomSuffix->setText(suffix);
    d->startIndexInput->setValue(startIndex);
    d->dateTimeFormat->setCurrentIndex(dateTime);
    d->dateTimeFormatString = format;
    slotRenameOptionsChanged();
}

void RenameCustomizer::saveSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group("Camera Settings");
    group.writeEntry("Rename Use Default", d->renameDefault->isChecked());
    group.writeEntry("Add Camera Name", d->addCameraNameBox->isChecked());
    group.writeEntry("Add Date Time", d->addDateTimeBox->isChecked());
    group.writeEntry("Add Sequence Number", d->addSeqNumberBox->isChecked());
    group.writeEntry("Case Type", d->renameDefaultCaseType->currentIndex());
    group.writeEntry("Rename Prefix", d->renameCustomPrefix->text());
    group.writeEntry("Rename Suffix", d->renameCustomSuffix->text());
    group.writeEntry("Rename Start Index", d->startIndexInput->value());
    group.writeEntry("Date Time Format", d->dateTimeFormat->currentIndex());
    group.writeEntry("Date Time Format String", d->dateTimeFormatString);
    config->sync();
}

void RenameCustomizer::restoreFocus()
{
    d->focusedWidget->setFocus();
}

}  // namespace Digikam
