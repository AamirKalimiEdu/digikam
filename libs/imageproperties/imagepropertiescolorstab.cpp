/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : a tab to display colors information of images
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

// C++ includes.

#include <cmath>

// Qt includes.

#include <QButtonGroup>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QToolButton>

// KDE includes.

#include <kdebug.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// Local includes.

#include "dimg.h"
#include "imagehistogram.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "colorgradientwidget.h"
#include "sharedloadsavethread.h"
#include "iccprofilewidget.h"
#include "cietonguewidget.h"
#include "imagepropertiescolorstab.h"
#include "imagepropertiescolorstab.moc"

namespace Digikam
{

class ImagePropertiesColorsTabPriv
{
public:

    enum MetadataTab
    {
        HISTOGRAM=0,
        ICCPROFILE
    };

    ImagePropertiesColorsTabPriv()
    {
        regionBox            = 0;
        imageLoaderThread    = 0;
        regionBG             = 0;
        minInterv            = 0;
        maxInterv            = 0;
        labelMeanValue       = 0;
        labelPixelsValue     = 0;
        labelStdDevValue     = 0;
        labelCountValue      = 0;
        labelMedianValue     = 0;
        labelPercentileValue = 0;
        labelColorDepth      = 0;
        labelAlphaChannel    = 0;
        iccProfileWidget     = 0;
        imageLoaderThread    = 0;
        histogramBox         = 0;
    }

    bool                   blinkFlag;

    QWidget               *regionBox;

    QButtonGroup          *regionBG;

    QSpinBox              *minInterv;
    QSpinBox              *maxInterv;

    QLabel                *labelMeanValue;
    QLabel                *labelPixelsValue;
    QLabel                *labelStdDevValue;
    QLabel                *labelCountValue;
    QLabel                *labelMedianValue;
    QLabel                *labelPercentileValue;
    QLabel                *labelColorDepth;
    QLabel                *labelAlphaChannel;

    QString                currentFilePath;
    LoadingDescription     currentLoadingDescription;

    QRect                  selectionArea;

    QByteArray             embedded_profile;

    DImg                   image;
    DImg                   imageSelection;

    ICCProfileWidget      *iccProfileWidget;
    SharedLoadSaveThread  *imageLoaderThread;

    HistogramBox          *histogramBox;
};

ImagePropertiesColorsTab::ImagePropertiesColorsTab(QWidget* parent)
                        : KTabWidget(parent)
{
    d = new ImagePropertiesColorsTabPriv;

    // Histogram tab area -----------------------------------------------------

    QWidget* histogramPage = new QWidget(this);
    QGridLayout *topLayout = new QGridLayout(histogramPage);

    // -------------------------------------------------------------

    d->regionBox       = new QWidget(histogramPage);
    QHBoxLayout *hlay2 = new QHBoxLayout(d->regionBox);
    d->regionBG        = new QButtonGroup(d->regionBox);
    d->regionBG->setExclusive(true);
    d->regionBox->hide();
    d->regionBox->setWhatsThis( i18n("<p>Select from which region the histogram will be computed here:</p>"
                                     "<p><b>Full Image</b>: Compute histogram using the full image.<br/>"
                                     "<b>Selection</b>: Compute histogram using the current image "
                                     "selection.</p>"));

    QPushButton *fullImageButton = new QPushButton(d->regionBox);
    fullImageButton->setToolTip( i18n( "Full Image" ) );
    fullImageButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/image-full.png")));
    fullImageButton->setCheckable(true);
    d->regionBG->addButton(fullImageButton, HistogramWidget::FullImageHistogram);

    QPushButton *SelectionImageButton = new QPushButton(d->regionBox);
    SelectionImageButton->setToolTip( i18n( "Selection" ) );
    SelectionImageButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/image-selection.png")));
    SelectionImageButton->setCheckable(true);
    d->regionBG->addButton(SelectionImageButton, HistogramWidget::ImageSelectionHistogram);

    hlay2->setMargin(0);
    hlay2->setSpacing(0);
    hlay2->addWidget(fullImageButton);
    hlay2->addWidget(SelectionImageButton);

    // -------------------------------------------------------------

    KVBox *histoBox    = new KVBox(histogramPage);
    d->histogramBox    = new HistogramBox(histoBox, HistogramBox::LRGBAC, true);

    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);

    // -------------------------------------------------------------

    QHBoxLayout *hlay3 = new QHBoxLayout();
    QLabel *label3     = new QLabel(i18n("Range:"), histogramPage);
    label3->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->minInterv = new QSpinBox(histogramPage);
    d->minInterv->setRange(0, 255);
    d->minInterv->setSingleStep(1);
    d->minInterv->setValue(0);
    d->minInterv->setWhatsThis( i18n("Select the minimal intensity "
                                     "value of the histogram selection here."));
    d->maxInterv = new QSpinBox(histogramPage);
    d->minInterv->setRange(0, 255);
    d->minInterv->setSingleStep(1);
    d->maxInterv->setValue(255);
    d->minInterv->setWhatsThis( i18n("Select the maximal intensity value "
                                     "of the histogram selection here."));
    hlay3->addWidget(label3);
    hlay3->addWidget(d->minInterv);
    hlay3->addWidget(d->maxInterv);
    hlay2->setSpacing(KDialog::spacingHint());
    hlay2->setMargin(0);

    // -------------------------------------------------------------

    QGroupBox *gbox = new QGroupBox(i18n("Statistics"), histogramPage);
    gbox->setWhatsThis( i18n("Here you can see the statistical results calculated from the "
                             "selected histogram part. These values are available for all "
                             "channels."));
    QGridLayout* grid = new QGridLayout(gbox);

    QLabel *label5 = new QLabel(i18n("Pixels:"), gbox);
    label5->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->labelPixelsValue = new QLabel(gbox);
    d->labelPixelsValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLabel *label7 = new QLabel(i18n("Count:"), gbox);
    label7->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->labelCountValue = new QLabel(gbox);
    d->labelCountValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLabel *label4 = new QLabel(i18n("Mean:"), gbox);
    label4->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->labelMeanValue = new QLabel(gbox);
    d->labelMeanValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLabel *label6 = new QLabel(i18n("Std. deviation:"), gbox);
    label6->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->labelStdDevValue = new QLabel(gbox);
    d->labelStdDevValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLabel *label8 = new QLabel(i18n("Median:"), gbox);
    label8->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->labelMedianValue = new QLabel(gbox);
    d->labelMedianValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLabel *label9 = new QLabel(i18n("Percentile:"), gbox);
    label9->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->labelPercentileValue = new QLabel(gbox);
    d->labelPercentileValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    grid->addWidget(label5,                  0, 0, 1, 1);
    grid->addWidget(d->labelMeanValue,       0, 1, 1, 1);
    grid->addWidget(label7,                  1, 0, 1, 1);
    grid->addWidget(d->labelPixelsValue,     1, 1, 1, 1);
    grid->addWidget(label4,                  2, 0, 1, 1);
    grid->addWidget(d->labelStdDevValue,     2, 1, 1, 1);
    grid->addWidget(label6,                  3, 0, 1, 1);
    grid->addWidget(d->labelCountValue,      3, 1, 1, 1);
    grid->addWidget(label8,                  4, 0, 1, 1);
    grid->addWidget(d->labelMedianValue,     4, 1, 1, 1);
    grid->addWidget(label9,                  5, 0, 1, 1);
    grid->addWidget(d->labelPercentileValue, 5, 1, 1, 1);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(0);

    // -------------------------------------------------------------

    QWidget *gbox2     = new QWidget(histogramPage);
    QGridLayout* grid2 = new QGridLayout(gbox2);

    QLabel *label11     = new QLabel(i18n("Color depth:"), gbox2);
    label11->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->labelColorDepth  = new QLabel(gbox2);
    d->labelColorDepth->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLabel *label12     = new QLabel(i18n("Alpha Channel:"), gbox2);
    label12->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->labelAlphaChannel = new QLabel(gbox2);
    d->labelAlphaChannel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    grid2->addWidget(label11,              0, 0, 1, 1);
    grid2->addWidget(d->labelColorDepth,   0, 1, 1, 1);
    grid2->addWidget(label12,              1, 0, 1, 1);
    grid2->addWidget(d->labelAlphaChannel, 1, 1, 1, 1);
    grid2->setMargin(0);
    grid2->setSpacing(0);

    // -------------------------------------------------------------

    topLayout->addWidget(d->regionBox, 0, 3, 1, 1);
    topLayout->addWidget(histoBox,     1, 0, 2, 4);
    topLayout->addLayout(hlay3,        3, 0, 1, 4);
    topLayout->addWidget(gbox,         4, 0, 1, 4);
    topLayout->addWidget(gbox2,        5, 0, 1, 4);
    topLayout->setRowStretch(6, 10);
    topLayout->setColumnStretch(2, 10);
    topLayout->setMargin(KDialog::spacingHint());
    topLayout->setSpacing(KDialog::spacingHint());

    insertTab(ImagePropertiesColorsTabPriv::HISTOGRAM, histogramPage, i18n("Histogram"));

    // ICC Profiles tab area ---------------------------------------

    d->iccProfileWidget = new ICCProfileWidget(this);
    insertTab(ImagePropertiesColorsTabPriv::ICCPROFILE, d->iccProfileWidget, i18n("ICC profile"));

    // -- read config ---------------------------------------------------------

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QString("Image Properties SideBar"));

    setCurrentIndex(group.readEntry("ImagePropertiesColors Tab",
                    (int)ImagePropertiesColorsTabPriv::HISTOGRAM));
    d->iccProfileWidget->setMode(group.readEntry("ICC Level", (int)ICCProfileWidget::SIMPLE));
    d->iccProfileWidget->setCurrentItemByKey(group.readEntry("Current ICC Item", QString()));

    d->histogramBox->setChannel(group.readEntry("Histogram Channel",
                                (int)HistogramBox::LuminosityChannel));
    d->histogramBox->setScale(group.readEntry("Histogram Scale",
                                (int)HistogramBox::Logarithmic));
    d->histogramBox->setColorsChannel(group.readEntry("Histogram Color", 0));

    d->regionBG->button(group.readEntry("Histogram Rendering",
                                        (int)HistogramWidget::FullImageHistogram))->setChecked(true);

    // -------------------------------------------------------------

    connect(d->regionBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotRenderingChanged(int)));

    // -------------------------------------------------------------
    // histogramBox connections

    connect(d->histogramBox->histogram(), SIGNAL(signalIntervalChanged( int, int )),
            this, SLOT(slotUpdateInterval(int, int)));

    connect(d->histogramBox->histogram(), SIGNAL(signalMaximumValueChanged( int )),
            this, SLOT(slotUpdateIntervRange(int)));

    connect(d->histogramBox->histogram(), SIGNAL(signalHistogramComputationDone(bool)),
            this, SLOT(slotRefreshOptions(bool)));

    connect(d->histogramBox->histogram(), SIGNAL(signalHistogramComputationFailed(void)),
            this, SLOT(slotHistogramComputationFailed(void)));

    connect(d->histogramBox, SIGNAL(signalChannelChanged()),
            this, SLOT(slotChannelChanged()));

    connect(d->histogramBox, SIGNAL(signalScaleChanged()),
            this, SLOT(slotScaleChanged()));

    connect(d->histogramBox, SIGNAL(signalColorsChanged()),
            this, SLOT(slotColorsChanged()));

    // -------------------------------------------------------------

    connect(d->minInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotMinValueChanged(int)));

    connect(d->maxInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotMaxValueChanged(int)));
}

ImagePropertiesColorsTab::~ImagePropertiesColorsTab()
{
    // If there is a currently histogram computation when dialog is closed,
    // stop it before the d->image data are deleted automatically!
    d->histogramBox->histogram()->stopHistogramComputation();

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Image Properties SideBar"));
    group.writeEntry("ImagePropertiesColors Tab", currentIndex());
    group.writeEntry("Histogram Channel", d->histogramBox->channel());
    group.writeEntry("Histogram Scale", d->histogramBox->scale());
    group.writeEntry("Histogram Color", d->histogramBox->colorsChannel());
    group.writeEntry("Histogram Rendering", d->regionBG->checkedId());
    group.writeEntry("ICC Level", d->iccProfileWidget->getMode());
    group.writeEntry("Current ICC Item", d->iccProfileWidget->getCurrentItemKey());
    config->sync();

    if (d->imageLoaderThread)
       delete d->imageLoaderThread;

    delete d;
}

void ImagePropertiesColorsTab::setData(const KUrl& url, const QRect &selectionArea,
                                       DImg *img)
{
    // We might be getting duplicate events from AlbumIconView,
    // which will cause all sorts of duplicate work.
    // More importantly, while the loading thread can handle this pretty well,
    // this will completely mess up the timing of progress info in the histogram widget.
    // So filter here, before the stopHistogramComputation!
    // But do not filter if current path is null, as it would not disable the widget on first run.
    if (!img && !d->currentFilePath.isNull() && url.path() == d->currentFilePath)
        return;

    // This is necessary to stop computation because d->image.bits() is currently used by
    // threaded histogram algorithm.
    d->histogramBox->histogram()->stopHistogramComputation();

    d->currentFilePath = QString();
    d->currentLoadingDescription = LoadingDescription();
    d->iccProfileWidget->loadFromURL(KUrl());

    // Clear information.
    d->labelMeanValue->clear();
    d->labelPixelsValue->clear();
    d->labelStdDevValue->clear();
    d->labelCountValue->clear();
    d->labelMedianValue->clear();
    d->labelPercentileValue->clear();
    d->labelColorDepth->clear();
    d->labelAlphaChannel->clear();

    if (url.isEmpty())
    {
       setEnabled(false);
       return;
    }

    d->selectionArea = selectionArea;
    d->image.reset();
    setEnabled(true);

    if (!img)
    {
        loadImageFromUrl(url);
    }
    else
    {
        d->image = img->copy();

        if ( !d->image.isNull() )
        {
            getICCData();

            // If a selection area is done in Image Editor and if the current image is the same
            // in Image Editor, then compute too the histogram for this selection.
            if (d->selectionArea.isValid())
            {
                d->imageSelection = d->image.copy(d->selectionArea);
                d->histogramBox->histogram()->updateData(d->image.bits(), d->image.width(), d->image.height(),
                                               d->image.sixteenBit(), d->imageSelection.bits(),
                                               d->imageSelection.width(), d->imageSelection.height());
                d->regionBox->show();
                updateInformation();
            }
            else
            {
                d->histogramBox->histogram()->updateData(d->image.bits(), d->image.width(),
                                               d->image.height(), d->image.sixteenBit());
                d->regionBox->hide();
                updateInformation();
            }
        }
        else
        {
            d->histogramBox->histogram()->setLoadingFailed();
            d->iccProfileWidget->setLoadingFailed();
            slotHistogramComputationFailed();
        }
    }
}

void ImagePropertiesColorsTab::loadImageFromUrl(const KUrl& url)
{
    // create thread on demand
    if (!d->imageLoaderThread)
    {
        d->imageLoaderThread = new SharedLoadSaveThread();

        connect(d->imageLoaderThread, SIGNAL(signalImageLoaded(const LoadingDescription &, const DImg&)),
                this, SLOT(slotLoadImageFromUrlComplete(const LoadingDescription &, const DImg&)));

        connect(d->imageLoaderThread, SIGNAL(signalMoreCompleteLoadingAvailable(const LoadingDescription &, const LoadingDescription &)),
                this, SLOT(slotMoreCompleteLoadingAvailable(const LoadingDescription &, const LoadingDescription &)));
    }

    LoadingDescription desc = LoadingDescription(url.path());

    if (DImg::fileFormat(desc.filePath) == DImg::RAW)
    {
        // use raw settings optimized for speed

        DRawDecoding rawDecodingSettings = DRawDecoding();
        rawDecodingSettings.optimizeTimeLoading();
        desc = LoadingDescription(desc.filePath, rawDecodingSettings);
    }

    if (d->currentLoadingDescription.equalsOrBetterThan(desc))
        return;

    d->currentFilePath = desc.filePath;
    d->currentLoadingDescription = desc;

    d->imageLoaderThread->load(d->currentLoadingDescription,
                               SharedLoadSaveThread::AccessModeRead,
                               SharedLoadSaveThread::LoadingPolicyFirstRemovePrevious);

    d->histogramBox->histogram()->setDataLoading();
    d->iccProfileWidget->setDataLoading();
}

void ImagePropertiesColorsTab::slotLoadImageFromUrlComplete(const LoadingDescription &loadingDescription, const DImg& img)
{
    // Discard any leftover messages from previous, possibly aborted loads
    if ( !loadingDescription.equalsOrBetterThan(d->currentLoadingDescription) )
        return;

    if ( !img.isNull() )
    {
        d->histogramBox->histogram()->updateData(img.bits(), img.width(), img.height(), img.sixteenBit());

        // As a safety precaution, this must be changed only after updateData is called,
        // which stops computation because d->image.bits() is currently used by threaded histogram algorithm.
        d->image = img;
        d->regionBox->hide();
        updateInformation();
        getICCData();
    }
    else
    {
        d->histogramBox->histogram()->setLoadingFailed();
        d->iccProfileWidget->setLoadingFailed();
        slotHistogramComputationFailed();
    }
}

void ImagePropertiesColorsTab::slotMoreCompleteLoadingAvailable(const LoadingDescription &oldLoadingDescription,
                                                                const LoadingDescription &newLoadingDescription)
{
    if (oldLoadingDescription == d->currentLoadingDescription &&
        newLoadingDescription.equalsOrBetterThan(d->currentLoadingDescription))
    {
        // Yes, we do want to stop our old time-optimized loading and chain to the current, more complete loading.
        // Even the time-optimized raw loading takes significant time, and we must avoid two dcraw instances running
        // at a time.
        d->currentLoadingDescription = newLoadingDescription;
        d->imageLoaderThread->load(newLoadingDescription,
                                   SharedLoadSaveThread::AccessModeRead,
                                   SharedLoadSaveThread::LoadingPolicyFirstRemovePrevious);
    }
}

void ImagePropertiesColorsTab::setSelection(const QRect &selectionArea)
{
    // This is necessary to stop computation because d->image.bits() is currently used by
    // threaded histogram algorithm.

    d->histogramBox->histogram()->stopHistogramComputation();
    d->selectionArea = selectionArea;

    if (d->selectionArea.isValid())
    {
        d->imageSelection = d->image.copy(d->selectionArea);
        d->histogramBox->histogram()->updateSelectionData(d->imageSelection.bits(), d->imageSelection.width(),
                                                d->imageSelection.height(), d->imageSelection.sixteenBit());
        d->regionBox->show();
    }
    else
    {
        d->regionBox->hide();
        slotRenderingChanged(HistogramWidget::FullImageHistogram);
    }
}

void ImagePropertiesColorsTab::slotRefreshOptions(bool /*sixteenBit*/)
{
    slotChannelChanged();
    slotScaleChanged();
    slotColorsChanged();

    if (d->selectionArea.isValid())
       slotRenderingChanged(d->regionBG->checkedId());
}

void ImagePropertiesColorsTab::slotHistogramComputationFailed()
{
    d->imageSelection.reset();
    d->image.reset();
}

void ImagePropertiesColorsTab::slotChannelChanged()
{
    updateStatistics();
}

void ImagePropertiesColorsTab::slotColorsChanged()
{
    updateStatistics();
}

void ImagePropertiesColorsTab::slotScaleChanged()
{
    updateStatistics();
}

void ImagePropertiesColorsTab::slotRenderingChanged(int rendering)
{
    d->histogramBox->histogram()->setRenderingType((HistogramWidget::HistogramRenderingType)rendering);
    updateStatistics();
}

void ImagePropertiesColorsTab::slotMinValueChanged(int min)
{
    // Called when user changes values of spin box.
    // Communicate the change to histogram widget.

    // make the one control "push" the other
    if (min == d->maxInterv->value()+1)
        d->maxInterv->setValue(min);
    d->maxInterv->setMinimum(min-1);
    d->histogramBox->histogram()->slotMinValueChanged(min);
    updateStatistics();
}

void ImagePropertiesColorsTab::slotMaxValueChanged(int max)
{
    if (max == d->minInterv->value()-1)
        d->minInterv->setValue(max);
    d->minInterv->setMaximum(max+1);
    d->histogramBox->histogram()->slotMaxValueChanged(max);
    updateStatistics();
}

void ImagePropertiesColorsTab::slotUpdateInterval(int min, int max)
{
    // Called when value is set from within histogram widget.
    // Block signals to prevent slotMinValueChanged and
    // slotMaxValueChanged being called.
    d->minInterv->blockSignals(true);
    d->minInterv->setMaximum(max+1);
    d->minInterv->setValue(min);
    d->minInterv->blockSignals(false);

    d->maxInterv->blockSignals(true);
    d->maxInterv->setMinimum(min-1);
    d->maxInterv->setValue(max);
    d->maxInterv->blockSignals(false);

    updateStatistics();
}

void ImagePropertiesColorsTab::slotUpdateIntervRange(int range)
{
    d->maxInterv->setMaximum( range );
}

void ImagePropertiesColorsTab::updateInformation()
{
    d->labelColorDepth->setText(d->image.sixteenBit() ? i18n("16 bits") : i18n("8 bits"));
    d->labelAlphaChannel->setText(d->image.hasAlpha() ? i18n("Yes") : i18n("No"));
}

void ImagePropertiesColorsTab::updateStatistics()
{
    if (!d->histogramBox->histogram()->m_imageHistogram)
        return;

    QString value;
    int min = d->minInterv->value();
    int max = d->maxInterv->value();
    int channel = d->histogramBox->channel();

    if ( channel == HistogramWidget::ColorChannelsHistogram )
        channel = d->histogramBox->colorsChannel()+1;

    double mean = d->histogramBox->histogram()->m_imageHistogram->getMean(channel, min, max);
    d->labelMeanValue->setText(value.setNum(mean, 'f', 1));

    double pixels = d->histogramBox->histogram()->m_imageHistogram->getPixels();
    d->labelPixelsValue->setText(value.setNum((float)pixels, 'f', 0));

    double stddev = d->histogramBox->histogram()->m_imageHistogram->getStdDev(channel, min, max);
    d->labelStdDevValue->setText(value.setNum(stddev, 'f', 1));

    double counts = d->histogramBox->histogram()->m_imageHistogram->getCount(channel, min, max);
    d->labelCountValue->setText(value.setNum((float)counts, 'f', 0));

    double median = d->histogramBox->histogram()->m_imageHistogram->getMedian(channel, min, max);
    d->labelMedianValue->setText(value.setNum(median, 'f', 1));

    double percentile = (pixels > 0 ? (100.0 * counts / pixels) : 0.0);
    d->labelPercentileValue->setText(value.setNum(percentile, 'f', 1));
}

void ImagePropertiesColorsTab::getICCData()
{
    if (d->image.getICCProfil().isNull())
    {
        d->iccProfileWidget->setLoadingFailed();
    }
    else
    {
        d->embedded_profile = d->image.getICCProfil();
        d->iccProfileWidget->loadFromProfileData(d->currentFilePath, d->embedded_profile);
    }
}

}  // namespace Digikam
