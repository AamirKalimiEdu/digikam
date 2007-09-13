/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2006-05-15
 * Description : a dialog to see preview ICC color correction 
 *               before to apply color profile.
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

// Qt includes.

#include <QLabel>
#include <QFrame>
#include <QString>
#include <QFileInfo>
#include <QPushButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kseparator.h>

// Local includes.

#include "ddebug.h"
#include "dimg.h"
#include "icctransform.h"
#include "iccprofileinfodlg.h"
#include "colorcorrectiondlg.h"
#include "colorcorrectiondlg.moc"

namespace Digikam
{

ColorCorrectionDlg::ColorCorrectionDlg(QWidget* parent, DImg *preview, 
                                       IccTransform *iccTrans, const QString& file)
                  : KDialog(parent)
{
    setButtons(Help|Ok|Apply|Cancel);
    setDefaultButton(Ok);
    setModal(true);
    m_iccTrans = iccTrans;
    m_parent   = parent;
    setHelp("iccprofile.anchor", "digikam");
    setButtonText(Ok,        i18n("Convert"));
    setButtonToolTip(Ok,     i18n("Apply the default color workspace profile to the image"));
    setButtonText(Cancel,    i18n("Do Nothing"));
    setButtonToolTip(Cancel, i18n("Do not change the image"));
    setButtonText(Apply,     i18n("Assign"));
    setButtonToolTip(Apply,  i18n("Embed only the color workspace profile to the image "
                                  "without changing the image"));

    QFileInfo fi(file);
    setCaption(fi.fileName());
    
    QWidget *page     = new QWidget(this);
    setMainWidget(page);
    QGridLayout* grid = new QGridLayout(page);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());
    page->setLayout(grid);        

    QLabel *originalTitle         = new QLabel(i18n("Original Image:"), page);
    QLabel *previewOriginal       = new QLabel(page);
    QLabel *targetTitle           = new QLabel(i18n("Corrected Image:"), page);
    QLabel *previewTarget         = new QLabel(page);
    QLabel *logo                  = new QLabel(page);
    QLabel *message               = new QLabel(page);
    QLabel *currentProfileTitle   = new QLabel(i18n("Current workspace color profile:"), page);
    QLabel *currentProfileDesc    = new QLabel(QString("<b>%1</b>").arg(m_iccTrans->getOutpoutProfileDescriptor()), page);
    QPushButton *currentProfInfo  = new QPushButton(i18n("Info..."), page);
    QLabel *embeddedProfileTitle  = new QLabel(i18n("Embedded color profile:"), page);
    QLabel *embeddedProfileDesc   = new QLabel(QString("<b>%1</b>").arg(m_iccTrans->getEmbeddedProfileDescriptor()), page);
    QPushButton *embeddedProfInfo = new QPushButton(i18n("Info..."), page);
    KSeparator *line              = new KSeparator (Qt::Horizontal, page);
    
    if (m_iccTrans->embeddedProfile().isEmpty())
    {
        message->setText(i18n("<p>This image has not been assigned any color profile.</p>"
                              "<p>Do you want to convert it to your workspace color profile?</p>"));
                              
        line->hide();
        embeddedProfileTitle->hide();
        embeddedProfileDesc->hide();
        embeddedProfInfo->hide();
    }
    else
    {
        message->setText(i18n("<p>This image has been assigned a color profile that does not "
                              "match with your default workspace color profile.</p>"
                              "<p>Do you want to convert it to your workspace color profile?</p>"));
    }
    
    previewOriginal->setPixmap(preview->convertToPixmap());
    previewTarget->setPixmap(preview->convertToPixmap(m_iccTrans));
    KIconLoader* iconLoader = KIconLoader::global();
    logo->setPixmap(iconLoader->loadIcon("digikam", K3Icon::NoGroup, 128));    
    
    grid->addWidget(originalTitle, 0, 0, 0, 0);
    grid->addWidget(previewOriginal, 1, 1, 0, 0);
    grid->addWidget(targetTitle, 2, 2, 0, 0);
    grid->addWidget(previewTarget, 3, 3, 0, 0);
    
    QVBoxLayout *vlay = new QVBoxLayout();
    vlay->setSpacing( KDialog::spacingHint() );
    vlay->addWidget(logo);
    vlay->addWidget(message);
    
    vlay->addWidget(new KSeparator (Qt::Horizontal, page));
    vlay->addWidget(currentProfileTitle);
    vlay->addWidget(currentProfileDesc);
    
    QHBoxLayout *hlay1 = new QHBoxLayout();
    hlay1->setSpacing( KDialog::spacingHint() );
    hlay1->addWidget(currentProfInfo);
    hlay1->addStretch();
    vlay->addLayout(hlay1);
    
    vlay->addWidget(line);
    vlay->addWidget(embeddedProfileTitle);
    vlay->addWidget(embeddedProfileDesc);    
    
    QHBoxLayout *hlay2 = new QHBoxLayout();
    hlay2->setSpacing( KDialog::spacingHint() );
    hlay2->addWidget(embeddedProfInfo);
    hlay2->addStretch();
    vlay->addLayout(hlay2);
    vlay->addStretch();
    
    grid->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                      QSizePolicy::Minimum, QSizePolicy::Expanding), 0, 3, 1, 1);
    grid->addLayout(vlay, 0, 3, 2, 2);
    
    setMainWidget(page);
    
    // --------------------------------------------------------------------
    
    connect(currentProfInfo, SIGNAL(clicked()),
            this, SLOT(slotCurrentProfInfo()) );
    
    connect(embeddedProfInfo, SIGNAL(clicked()),
            this, SLOT(slotEmbeddedProfInfo()) );
            
    connect(this, SIGNAL(applyClicked()), 
            this, SLOT(slotApplyClicked()));
}

ColorCorrectionDlg::~ColorCorrectionDlg()
{
}

void ColorCorrectionDlg::slotCurrentProfInfo()
{
    if (m_iccTrans->outputProfile().isEmpty())
        return;

    ICCProfileInfoDlg infoDlg(m_parent, QString(), m_iccTrans->outputProfile());
    infoDlg.exec();
}

void ColorCorrectionDlg::slotEmbeddedProfInfo()
{
    if (m_iccTrans->embeddedProfile().isEmpty())
        return;

    ICCProfileInfoDlg infoDlg(m_parent, QString(), m_iccTrans->embeddedProfile());
    infoDlg.exec();
}

void ColorCorrectionDlg::slotApplyClicked()
{
    DDebug() << "colorcorrectiondlg: Apply pressed" << endl;
    done(-1);
}

}  // NameSpace Digikam

