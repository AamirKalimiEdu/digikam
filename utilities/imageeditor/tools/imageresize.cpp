/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-07
 * Description : a tool to resize a picture
 *
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

// C++ include.

#include <cmath>

// Qt includes.

#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QCheckBox>
#include <QComboBox>
#include <QTabWidget>
#include <QTimer>
#include <QEvent>
#include <QPixmap>
#include <QBrush>
#include <QFile>
#include <QImage>
#include <QCustomEvent>
#include <QCloseEvent>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QProgressBar>

// KDE includes.

#include <kseparator.h>
#include <kcursor.h>
#include <kurllabel.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <KToolInvocation>

// Digikam includes.

#include "dimg.h"
#include "ddebug.h"
#include "imageiface.h"
#include "dimgthreadedfilter.h"
#include "greycstorationiface.h"
#include "greycstorationwidget.h"
#include "greycstorationsettings.h"

// Local includes.

#include "imageresize.h"
#include "imageresize.moc"

namespace Digikam
{

class ImageResizePriv
{
public:

    enum RunningMode
    {
        NoneRendering=0,
        FinalRendering
    };

    ImageResizePriv()
    {
        currentRenderingMode = NoneRendering;
        parent               = 0;
        preserveRatioBox     = 0;
        useGreycstorationBox = 0;
        mainTab              = 0;
        wInput               = 0;
        hInput               = 0;
        wpInput              = 0;
        hpInput              = 0;
        progressBar          = 0;
        greycstorationIface  = 0;
        settingsWidget       = 0;
    }

    int                   currentRenderingMode;
    int                   orgWidth;
    int                   orgHeight;
    int                   prevW; 
    int                   prevH; 

    double                prevWP;
    double                prevHP;

    QWidget              *parent;

    QCheckBox            *preserveRatioBox;
    QCheckBox            *useGreycstorationBox;

    QTabWidget           *mainTab;

    KIntNumInput         *wInput;
    KIntNumInput         *hInput;

    KDoubleNumInput      *wpInput;
    KDoubleNumInput      *hpInput;

    QProgressBar         *progressBar;

    GreycstorationIface  *greycstorationIface;
    GreycstorationWidget *settingsWidget;
};

ImageResize::ImageResize(QWidget* parent)
           : KDialog(parent)
{
    d = new ImageResizePriv;
    d->parent = parent;

    setDefaultButton(Ok);
    setButtons(Help|Default|User2|User3|Ok|Cancel);
    setCaption(i18n("Resize Image"));
    setModal(true);
    setButtonText(User2,i18n("&Save As..."));
    setButtonText(User3,i18n("&Load..."));
    setHelp("resizetool.anchor", "digikam");
    setButtonWhatsThis ( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User2, i18n("<p>Save all filter parameters to settings text file.") );
    enableButton(Ok, false);

    ImageIface iface(0, 0);
    d->orgWidth    = iface.originalWidth();
    d->orgHeight   = iface.originalHeight();
    d->prevW       = d->orgWidth;
    d->prevH       = d->orgHeight;
    d->prevWP      = 100.0;
    d->prevHP      = 100.0;

    // -------------------------------------------------------------

    QWidget *page = new QWidget(this);
    setMainWidget(page);
    QVBoxLayout *vlay  = new QVBoxLayout();
    vlay->setSpacing(spacingHint());
    page->setLayout(vlay);

    d->mainTab         = new QTabWidget( page );
    QWidget* firstPage = new QWidget( d->mainTab );
    QGridLayout* grid  = new QGridLayout();
    grid->setMargin(spacingHint());
    firstPage->setLayout(grid);

    d->mainTab->addTab( firstPage, i18n("New Size") );

    QLabel *label1 = new QLabel(i18n("Width:"), firstPage);
    d->wInput      = new KIntNumInput(firstPage);
    d->wInput->setRange(1, qMax(d->orgWidth * 10, 9999), 1, true);
    d->wInput->setObjectName("d->wInput");
    d->wInput->setWhatsThis( i18n("<p>Set here the new image width in pixels."));

    QLabel *label2 = new QLabel(i18n("Height:"), firstPage);
    d->hInput      = new KIntNumInput(firstPage);
    d->hInput->setRange(1, qMax(d->orgHeight * 10, 9999), 1, true);
    d->hInput->setObjectName("d->hInput");
    d->hInput->setWhatsThis( i18n("<p>Set here the new image height in pixels."));

    QLabel *label3 = new QLabel(i18n("Width (%):"), firstPage);
    d->wpInput     = new KDoubleNumInput(firstPage);
    d->wpInput->setRange(1.0, 999.0, 1.0, true);
    d->wpInput->setObjectName("d->wpInput");
    d->wpInput->setWhatsThis( i18n("<p>Set here the new image width in percents."));

    QLabel *label4 = new QLabel(i18n("Height (%):"), firstPage);
    d->hpInput     = new KDoubleNumInput(firstPage);
    d->hpInput->setRange(1.0, 999.0, 1.0, true);
    d->hpInput->setObjectName("d->hpInput");
    d->hpInput->setWhatsThis( i18n("<p>Set here the new image height in percents."));

    d->preserveRatioBox = new QCheckBox(i18n("Maintain aspect ratio"), firstPage);
    d->preserveRatioBox->setWhatsThis( i18n("<p>Enable this option to maintain aspect "
                                            "ratio with new image sizes."));

    KUrlLabel *cimgLogoLabel = new KUrlLabel(firstPage);
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setUrl("http://cimg.sourceforge.net");
    cimgLogoLabel->setPixmap( QPixmap( KStandardDirs::locate("data", "digikam/data/logo-cimg.png" ) ));
    cimgLogoLabel->setToolTip( i18n("Visit CImg library website"));

    d->useGreycstorationBox = new QCheckBox(i18n("Restore photograph (slow)"), firstPage);
    d->useGreycstorationBox->setWhatsThis( i18n("<p>Enable this option to restore photograph content. "
                                                "Warning: this process can take a while."));

    d->progressBar = new QProgressBar(firstPage);
    d->progressBar->setValue(0);
    d->progressBar->setMaximum(100);
    d->progressBar->setWhatsThis( i18n("<p>This is the current progress when you use Restoration mode."));

    grid->addMultiCellWidget(d->preserveRatioBox, 0, 0, 0, 2);
    grid->addMultiCellWidget(label1, 1, 1, 0, 0);
    grid->addMultiCellWidget(d->wInput, 1, 1, 1, 2);
    grid->addMultiCellWidget(label2, 2, 2, 0, 0);
    grid->addMultiCellWidget(d->hInput, 2, 2, 1, 2);
    grid->addMultiCellWidget(label3, 3, 3, 0, 0);
    grid->addMultiCellWidget(d->wpInput, 3, 3, 1, 2);
    grid->addMultiCellWidget(label4, 4, 4, 0, 0);
    grid->addMultiCellWidget(d->hpInput, 4, 4, 1, 2);
    grid->addMultiCellWidget(new KSeparator(firstPage), 5, 5, 0, 2);
    grid->addMultiCellWidget(cimgLogoLabel, 6, 7, 0, 0);
    grid->addMultiCellWidget(d->useGreycstorationBox, 6, 6, 1, 2);
    grid->addMultiCellWidget(d->progressBar, 7, 7, 1, 2);
    grid->setRowStretch(8, 10);

    // -------------------------------------------------------------

    d->settingsWidget = new GreycstorationWidget(d->mainTab);
    vlay->addWidget(d->mainTab);

    // -------------------------------------------------------------

    adjustSize();
    //disableResize();
    QTimer::singleShot(0, this, SLOT(readUserSettings()));

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(processCImgUrl(const QString&)));

    connect(d->wInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotValuesChanged()));

    connect(d->hInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotValuesChanged()));

    connect(d->wpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->hpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->useGreycstorationBox, SIGNAL(toggled(bool)),
             this, SLOT(slotRestorationToggled(bool)) );
}

ImageResize::~ImageResize()
{
    if (d->greycstorationIface)
       delete d->greycstorationIface;

    delete d;
}

void ImageResize::slotRestorationToggled(bool b)
{
    d->settingsWidget->setEnabled(b);
    d->progressBar->setEnabled(b);
    enableButton(User2, b);
    enableButton(User3, b);
}

void ImageResize::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("resize Tool Dialog");

    GreycstorationSettings settings;
    settings.fastApprox = group.readEntry("FastApprox", true);
    settings.interp     = group.readEntry("Interpolation",
                          (int)GreycstorationSettings::NearestNeighbor);
    settings.amplitude  = group.readEntry("Amplitude", 20.0);
    settings.sharpness  = group.readEntry("Sharpness", 0.2);
    settings.anisotropy = group.readEntry("Anisotropy", 0.9);
    settings.alpha      = group.readEntry("Alpha", 0.1);
    settings.sigma      = group.readEntry("Sigma", 1.5);
    settings.gaussPrec  = group.readEntry("GaussPrec", 2.0);
    settings.dl         = group.readEntry("Dl", 0.8);
    settings.da         = group.readEntry("Da", 30.0);
    settings.nbIter     = group.readEntry("Iteration", 3);
    settings.tile       = group.readEntry("Tile", 512);
    settings.btile      = group.readEntry("BTile", 4);
    d->settingsWidget->setSettings(settings);
    d->useGreycstorationBox->setChecked(group.readEntry("RestorePhotograph", false));
    slotRestorationToggled(d->useGreycstorationBox->isChecked());

    d->preserveRatioBox->blockSignals(true);
    d->wInput->blockSignals(true);
    d->hInput->blockSignals(true);
    d->wpInput->blockSignals(true);
    d->hpInput->blockSignals(true);
    d->preserveRatioBox->setChecked(true);
    d->wInput->setValue(d->orgWidth);
    d->hInput->setValue(d->orgHeight);
    d->wpInput->setValue(100);
    d->hpInput->setValue(100);
    d->preserveRatioBox->blockSignals(false);
    d->wInput->blockSignals(false);
    d->hInput->blockSignals(false);
    d->wpInput->blockSignals(false);
    d->hpInput->blockSignals(false);
}

void ImageResize::writeUserSettings()
{
    GreycstorationSettings settings = d->settingsWidget->getSettings();
    KConfigGroup group = KGlobal::config()->group("resize Tool Dialog");
    group.writeEntry("FastApprox", settings.fastApprox);
    group.writeEntry("Interpolation", settings.interp);
    group.writeEntry("Amplitude", (double)settings.amplitude);
    group.writeEntry("Sharpness", (double)settings.sharpness);
    group.writeEntry("Anisotropy", (double)settings.anisotropy);
    group.writeEntry("Alpha", (double)settings.alpha);
    group.writeEntry("Sigma", (double)settings.sigma);
    group.writeEntry("GaussPrec", (double)settings.gaussPrec);
    group.writeEntry("Dl", (double)settings.dl);
    group.writeEntry("Da", (double)settings.da);
    group.writeEntry("Iteration", settings.nbIter);
    group.writeEntry("Tile", settings.tile);
    group.writeEntry("BTile", settings.btile);
    group.writeEntry("RestorePhotograph", d->useGreycstorationBox->isChecked());
    group.sync();
}

void ImageResize::slotDefault()
{
    GreycstorationSettings settings;
    settings.setResizeDefaultSettings();
    d->settingsWidget->setSettings(settings);
    d->useGreycstorationBox->setChecked(false);
    slotRestorationToggled(d->useGreycstorationBox->isChecked());

    d->preserveRatioBox->blockSignals(true);
    d->wInput->blockSignals(true);
    d->hInput->blockSignals(true);
    d->wpInput->blockSignals(true);
    d->hpInput->blockSignals(true);
    d->preserveRatioBox->setChecked(true);
    d->wInput->setValue(d->orgWidth);
    d->hInput->setValue(d->orgHeight);
    d->wpInput->setValue(100.0);
    d->hpInput->setValue(100.0);
    d->preserveRatioBox->blockSignals(false);
    d->wInput->blockSignals(false);
    d->hInput->blockSignals(false);
    d->wpInput->blockSignals(false);
    d->hpInput->blockSignals(false);
}

void ImageResize::slotValuesChanged()
{
    enableButton(Ok, true);
    d->wInput->blockSignals(true);
    d->hInput->blockSignals(true);
    d->wpInput->blockSignals(true);
    d->hpInput->blockSignals(true);

    QString s(sender()->objectName());

    if (s == "d->wInput")
    {
        double val = d->wInput->value();
        double wp  = val/(double)(d->orgWidth) * 100.0;
        d->wpInput->setValue(wp);

        if (d->preserveRatioBox->isChecked())
        {
            d->hpInput->setValue(wp);
            int h = (int)(wp*d->orgHeight/100);
            d->hInput->setValue(h);
        }
    }
    else if (s == "d->hInput")
    {
        double val = d->hInput->value();
        double hp  = val/(double)(d->orgHeight) * 100.0;
        d->hpInput->setValue(hp);

        if (d->preserveRatioBox->isChecked())
        {
            d->wpInput->setValue(hp);
            int w = (int)(hp*d->orgWidth/100);
            d->wInput->setValue(w);
        }
    }
    else if (s == "d->wpInput")
    {
        double val = d->wpInput->value();
        int w      = (int)(val*d->orgWidth/100);
        d->wInput->setValue(w);

        if (d->preserveRatioBox->isChecked())
        {
            d->hpInput->setValue(val);
            int h = (int)(val*d->orgHeight/100);
            d->hInput->setValue(h);
        }
    }
    else if (s == "d->hpInput")
    {
        double val = d->hpInput->value();
        int h      = (int)(val*d->orgHeight/100);
        d->hInput->setValue(h);

        if (d->preserveRatioBox->isChecked())
        {
            d->wpInput->setValue(val);
            int w = (int)(val*d->orgWidth/100);
            d->wInput->setValue(w);
        }
    }

    d->prevW  = d->wInput->value();
    d->prevH  = d->hInput->value();
    d->prevWP = d->wpInput->value();
    d->prevHP = d->hpInput->value();

    d->wInput->blockSignals(false);
    d->hInput->blockSignals(false);
    d->wpInput->blockSignals(false);
    d->hpInput->blockSignals(false);
}

void ImageResize::slotCancel()
{
    if (d->currentRenderingMode != ImageResizePriv::NoneRendering)
    {
        d->greycstorationIface->stopComputation();
        d->parent->unsetCursor();
    }

    done(Cancel);
}

void ImageResize::processCImgUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

void ImageResize::closeEvent(QCloseEvent *e)
{
    if (d->currentRenderingMode != ImageResizePriv::NoneRendering)
    {
        d->greycstorationIface->stopComputation();
        d->parent->unsetCursor();
    }

    e->accept();
}

void ImageResize::slotOk()
{
    if (d->prevW  != d->wInput->value()  || d->prevH  != d->hInput->value() ||
        d->prevWP != d->wpInput->value() || d->prevHP != d->hpInput->value())
        slotValuesChanged();

    d->currentRenderingMode = ImageResizePriv::FinalRendering;
    d->mainTab->setCurrentIndex(0);
    d->settingsWidget->setEnabled(false);
    d->preserveRatioBox->setEnabled(false);
    d->useGreycstorationBox->setEnabled(false);
    d->wInput->setEnabled(false);
    d->hInput->setEnabled(false);
    d->wpInput->setEnabled(false);
    d->hpInput->setEnabled(false);
    enableButton(Ok, false);
    enableButton(Default, false);
    enableButton(User2, false);
    enableButton(User3, false);

    d->parent->setCursor( Qt::WaitCursor );
    if (d->useGreycstorationBox->isChecked())
    {
        d->progressBar->setValue(0);
        d->progressBar->setEnabled(true);
    }
    writeUserSettings();

    ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    DImg originalImage = DImg(iface.originalWidth(), iface.originalHeight(),
                              iface.originalSixteenBit(), iface.originalHasAlpha(), data);
    delete [] data;

    if (d->greycstorationIface)
    {
        delete d->greycstorationIface;
        d->greycstorationIface = 0;
    }

    int mode = d->useGreycstorationBox->isChecked() ? GreycstorationIface::Resize
                                                    : GreycstorationIface::SimpleResize;

    d->greycstorationIface = new GreycstorationIface(
                                    &originalImage,
                                    d->settingsWidget->getSettings(),
                                    mode,
                                    d->wInput->value(),
                                    d->hInput->value(),
                                    QImage(),
                                    this);
}

void ImageResize::customEvent(QCustomEvent *event)
{
    if (!event) return;

    GreycstorationIface::EventData *data = (GreycstorationIface::EventData*) event;

    if (!data) return;

    if (data->starting)           // Computation in progress !
    {
        d->progressBar->setValue(data->progress);
    }
    else
    {
        if (data->success)        // Computation Completed !
        {
            switch (d->currentRenderingMode)
            {
                case ImageResizePriv::FinalRendering:
                {
                    DDebug() << "Final resizing completed..." << endl;

                    ImageIface iface(0, 0);
                    DImg resizedImage = d->greycstorationIface->getTargetImage();

                    iface.putOriginalImage(i18n("Resize"), resizedImage.bits(),
                                           resizedImage.width(), resizedImage.height());
                    d->parent->unsetCursor();
                    accept();
                    break;
                }
            }
        }
        else                   // Computation Failed !
        {
            switch (d->currentRenderingMode)
            {
                case ImageResizePriv::FinalRendering:
                    break;
            }
        }
    }

    delete data;
}

void ImageResize::slotUser3()
{
    KUrl loadBlowupFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                       QString( "*" ), this,
                                       QString( i18n("Photograph Resizing Settings File to Load")) );
    if( loadBlowupFile.isEmpty() )
       return;

    QFile file(loadBlowupFile.path());

    if ( file.open(QIODevice::ReadOnly) )
    {
        if (!d->settingsWidget->loadSettings(file, QString("# Photograph Resizing Configuration File")))
        {
           KMessageBox::error(this, 
                        i18n("\"%1\" is not a Photograph Resizing settings text file.",
                        loadBlowupFile.fileName()));
           file.close();
           return;
        }
    }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Resizing text file."));

    file.close();
}

void ImageResize::slotUser2()
{
    KUrl saveBlowupFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                       QString( "*" ), this,
                                       QString( i18n("Photograph Resizing Settings File to Save")) );
    if( saveBlowupFile.isEmpty() )
       return;

    QFile file(saveBlowupFile.path());

    if ( file.open(QIODevice::WriteOnly) )
        d->settingsWidget->saveSettings(file, QString("# Photograph Resizing Configuration File"));
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Resizing text file."));

    file.close();
}

}  // NameSpace Digikam

