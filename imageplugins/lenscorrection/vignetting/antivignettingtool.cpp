/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-25
 * Description : a digiKam image plugin to reduce
 *               vignetting on an image.
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "antivignettingtool.moc"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kseparator.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "antivignetting.h"
#include "bcgfilter.h"
#include "daboutdata.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"
#include "version.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamAntiVignettingImagesPlugin
{

class AntiVignettingToolPriv
{
public:

    AntiVignettingToolPriv() :
        configGroupName("antivignetting Tool"),
        configDensityAdjustmentEntry("DensityAdjustment"),
        configPowerAdjustmentEntry("PowerAdjustment"),
        configRadiusAdjustmentEntry("RadiusAdjustment"),
        configBrightnessAdjustmentEntry("BrightnessAdjustment"),
        configContrastAdjustmentEntry("ContrastAdjustment"),
        configGammaAdjustmentEntry("GammaAdjustment"),
        configAddVignettingEntry("AddVignetting"),

        maskPreviewLabel(0),
        brightnessInput(0),
        contrastInput(0),
        gammaInput(0),
        densityInput(0),
        powerInput(0),
        radiusInput(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString       configGroupName;
    const QString       configDensityAdjustmentEntry;
    const QString       configPowerAdjustmentEntry;
    const QString       configRadiusAdjustmentEntry;
    const QString       configBrightnessAdjustmentEntry;
    const QString       configContrastAdjustmentEntry;
    const QString       configGammaAdjustmentEntry;
    const QString       configAddVignettingEntry;

    QLabel*             maskPreviewLabel;

    RIntNumInput*       brightnessInput;
    RIntNumInput*       contrastInput;

    RDoubleNumInput*    gammaInput;
    RDoubleNumInput*    densityInput;
    RDoubleNumInput*    powerInput;
    RDoubleNumInput*    radiusInput;
    RDoubleNumInput*    xOffsetInput;
    RDoubleNumInput*    yOffsetInput;

    QCheckBox*          addVignettingCheck;

    ImageGuideWidget*   previewWidget;
    EditorToolSettings* gboxSettings;
};

AntiVignettingTool::AntiVignettingTool(QObject* parent)
                  : EditorToolThreaded(parent),
                    d(new AntiVignettingToolPriv)
{
    setObjectName("antivignetting");
    setToolName(i18n("Vignetting Correction"));
    setToolIcon(SmallIcon("antivignetting"));

    d->previewWidget = new ImageGuideWidget(0, false, ImageGuideWidget::HVGuideMode);
    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;

    // -------------------------------------------------------------

    d->addVignettingCheck = new QCheckBox(i18n("Add vignetting"));
    d->addVignettingCheck->setWhatsThis(i18n("This option add vignetting to the image instead for removing it."
                                             "Use it for creative effects."));
    d->addVignettingCheck->setChecked(false);

    // -------------------------------------------------------------

    d->maskPreviewLabel = new QLabel();
    d->maskPreviewLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->maskPreviewLabel->setPixmap(QPixmap(120, 120));
    d->maskPreviewLabel->setWhatsThis(i18n("You can see here a thumbnail preview of the anti-vignetting "
                                           "mask applied to the image."));

    // -------------------------------------------------------------

    QLabel *label1  = new QLabel(i18n("Amount:"));
    d->densityInput = new RDoubleNumInput();
    d->densityInput->setDecimals(1);
    d->densityInput->input()->setRange(1.0, 20.0, 0.1, true);
    d->densityInput->setDefaultValue(2.0);
    d->densityInput->setWhatsThis(i18n("This value controls the degree of intensity attenuation "
                                       "by the filter at its point of maximum density."));

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Feather:"));
    d->powerInput  = new RDoubleNumInput();
    d->powerInput->setDecimals(1);
    d->powerInput->input()->setRange(0.1, 2.0, 0.1, true);
    d->powerInput->setDefaultValue(1.0);
    d->powerInput->setWhatsThis(i18n("This value is used as the exponent controlling the "
                                     "fall-off in density from the center of the filter to the periphery."));

    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Radius:"));
    d->radiusInput = new RDoubleNumInput();
    d->radiusInput->setDecimals(1);
    d->radiusInput->input()->setRange(0.1, 1.5, 0.05, true);
    d->radiusInput->setDefaultValue(1.0);
    d->radiusInput->setWhatsThis(i18n("This value is the radius of the center filter. It is a "
                                      "multiple of the half-diagonal measure of the image, at which "
                                      "the density of the filter falls to zero."));

    // -------------------------------------------------------------

    QLabel *label4 = new QLabel(i18n("X offset:"));
    d->xOffsetInput = new RDoubleNumInput();
    d->xOffsetInput->setDecimals(0);
    d->xOffsetInput->input()->setRange(-100, 100, 1, true);
    d->xOffsetInput->setDefaultValue(0);
    d->xOffsetInput->setWhatsThis(i18n("X offset "));

    // -------------------------------------------------------------

    QLabel *label5 = new QLabel(i18n("Y offset:"));
    d->yOffsetInput = new RDoubleNumInput();
    d->yOffsetInput->setDecimals(0);
    d->yOffsetInput->input()->setRange(-100, 100, 1, true);
    d->yOffsetInput->setDefaultValue(0);
    d->yOffsetInput->setWhatsThis(i18n("Y offset "));

    KSeparator *line = new KSeparator (Qt::Horizontal);

    // -------------------------------------------------------------

    QLabel *label6     = new QLabel(i18n("Brightness:"));
    d->brightnessInput = new RIntNumInput();
    d->brightnessInput->setRange(0, 100, 1);
    d->brightnessInput->setSliderEnabled(true);
    d->brightnessInput->setDefaultValue(0);
    d->brightnessInput->setWhatsThis(i18n("Set here the brightness re-adjustment of the target image."));

    // -------------------------------------------------------------

    QLabel *label7   = new QLabel(i18n("Contrast:"));
    d->contrastInput = new RIntNumInput();
    d->contrastInput->setRange(0, 100, 1);
    d->contrastInput->setSliderEnabled(true);
    d->contrastInput->setDefaultValue(0);
    d->contrastInput->setWhatsThis(i18n("Set here the contrast re-adjustment of the target image."));

    // -------------------------------------------------------------

    QLabel *label8 = new QLabel(i18n("Gamma:"));
    d->gammaInput  = new RDoubleNumInput();
    d->gammaInput->setDecimals(2);
    d->gammaInput->input()->setRange(0.1, 3.0, 0.01, true);
    d->gammaInput->setDefaultValue(1.0);
    d->gammaInput->setWhatsThis(i18n("Set here the gamma re-adjustment of the target image."));

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->addWidget(d->maskPreviewLabel,    0, 0, 1, 3);
    mainLayout->addWidget(label1,                 1, 0, 1, 3);
    mainLayout->addWidget(d->densityInput,        2, 0, 1, 3);
    mainLayout->addWidget(label2,                 3, 0, 1, 3);
    mainLayout->addWidget(d->powerInput,          4, 0, 1, 3);
    mainLayout->addWidget(label3,                 5, 0, 1, 3);
    mainLayout->addWidget(d->radiusInput,         6, 0, 1, 3);
    mainLayout->addWidget(label4,                 7, 0, 1, 3);
    mainLayout->addWidget(d->xOffsetInput,        8, 0, 1, 3);
    mainLayout->addWidget(label5,                 9, 0, 1, 3);
    mainLayout->addWidget(d->yOffsetInput,        10, 0, 1, 3);
    mainLayout->addWidget(line,                   11, 0, 1, 3);
    mainLayout->addWidget(label6,                 12, 0, 1, 3);
    mainLayout->addWidget(d->brightnessInput,     13, 0, 1, 3);
    mainLayout->addWidget(label7,                 14, 0, 1, 3);
    mainLayout->addWidget(d->contrastInput,       15, 0, 1, 3);
    mainLayout->addWidget(label8,                 16, 0, 1, 3);
    mainLayout->addWidget(d->gammaInput,          17, 0, 1, 3);
    mainLayout->addWidget(d->addVignettingCheck,  18, 0, 1, 3);
    mainLayout->setRowStretch(19, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->densityInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->powerInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->radiusInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->brightnessInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(d->contrastInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    connect(d->gammaInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->xOffsetInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->yOffsetInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));

    connect(d->addVignettingCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotTimer()));
}

AntiVignettingTool::~AntiVignettingTool()
{
    delete d;
}

void AntiVignettingTool::renderingFinished()
{
    enableSettings(true);
}

void AntiVignettingTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    blockWidgetSignals(true);

    d->densityInput->setValue(group.readEntry(d->configDensityAdjustmentEntry,       d->densityInput->defaultValue()));
    d->powerInput->setValue(group.readEntry(d->configPowerAdjustmentEntry,           d->powerInput->defaultValue()));
    d->radiusInput->setValue(group.readEntry(d->configRadiusAdjustmentEntry,         d->radiusInput->defaultValue()));
    d->brightnessInput->setValue(group.readEntry(d->configBrightnessAdjustmentEntry, d->brightnessInput->defaultValue()));
    d->contrastInput->setValue(group.readEntry(d->configContrastAdjustmentEntry,     d->contrastInput->defaultValue()));
    d->gammaInput->setValue(group.readEntry(d->configGammaAdjustmentEntry,           d->gammaInput->defaultValue()));
    d->addVignettingCheck->setChecked(group.readEntry(d->configAddVignettingEntry,   false));
    blockWidgetSignals(false);

    slotEffect();
}

void AntiVignettingTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configDensityAdjustmentEntry,    d->densityInput->value());
    group.writeEntry(d->configPowerAdjustmentEntry,      d->powerInput->value());
    group.writeEntry(d->configRadiusAdjustmentEntry,     d->radiusInput->value());
    group.writeEntry(d->configBrightnessAdjustmentEntry, d->brightnessInput->value());
    group.writeEntry(d->configContrastAdjustmentEntry,   d->contrastInput->value());
    group.writeEntry(d->configGammaAdjustmentEntry,      d->gammaInput->value());
    group.writeEntry(d->configAddVignettingEntry,        d->addVignettingCheck->isChecked());

    group.sync();
}

void AntiVignettingTool::slotResetSettings()
{
    blockWidgetSignals(true);

    d->densityInput->slotReset();
    d->powerInput->slotReset();
    d->radiusInput->slotReset();
    d->brightnessInput->slotReset();
    d->contrastInput->slotReset();
    d->gammaInput->slotReset();
    d->xOffsetInput->slotReset();
    d->yOffsetInput->slotReset();

    blockWidgetSignals(false);

    slotEffect();
}

void AntiVignettingTool::enableSettings(bool b)
{
    d->densityInput->setEnabled(b);
    d->powerInput->setEnabled(b);
    d->radiusInput->setEnabled(b);
    d->brightnessInput->setEnabled(b);
    d->contrastInput->setEnabled(b);
    d->gammaInput->setEnabled(b);
    d->xOffsetInput->setEnabled(b);
    d->yOffsetInput->setEnabled(b);
    d->addVignettingCheck->setEnabled(b);
}

void AntiVignettingTool::prepareEffect()
{
    enableSettings(false);

    double dens           = d->densityInput->value();
    double power          = d->powerInput->value();
    double rad            = d->radiusInput->value();
    bool   addvignetting  = d->addVignettingCheck->isChecked();
    double xoffset        = d->xOffsetInput->value();
    double yoffset        = d->yOffsetInput->value();

    ImageIface* iface = d->previewWidget->imageIface();
    int orgWidth               = iface->originalWidth();
    int orgHeight              = iface->originalHeight();
    int previewWidth           = iface->previewWidth();
    int previewHeight          = iface->previewHeight();
    DImg imTemp                = iface->getOriginalImg()->smoothScale(previewWidth, previewHeight, Qt::KeepAspectRatio);
    QSize ps(orgWidth, orgHeight);
    ps.scale(QSize(120, 120), Qt::KeepAspectRatio);

    // Compute preview mask.
    DImg preview(ps.width(), ps.height(), false);
    memset(preview.bits(), 255, preview.numBytes());
    AntiVignetting maskPreview(&preview, 0, dens, power, rad, xoffset, yoffset, false, addvignetting);
    maskPreview.startFilterDirectly();       // Run filter without to use multithreading.
    QPixmap pix = maskPreview.getTargetImage().convertToPixmap();
    QPainter pt(&pix);
    pt.setPen(QPen(Qt::black, 1));
    pt.drawRect(0, 0, pix.width(), pix.height());
    pt.end();
    d->maskPreviewLabel->setPixmap(pix);

    setFilter(dynamic_cast<DImgThreadedFilter*>(
                       new AntiVignetting(&imTemp, this, dens, power, rad, xoffset, yoffset, true, addvignetting)));
}

void AntiVignettingTool::prepareFinal()
{
    enableSettings(false);

    double dens          = d->densityInput->value();
    double power         = d->powerInput->value();
    double rad           = d->radiusInput->value();
    bool   addvignetting = d->addVignettingCheck->isChecked();
    double xoffset       = d->xOffsetInput->value();
    double yoffset       = d->yOffsetInput->value();

    ImageIface iface(0, 0);

    setFilter(dynamic_cast<DImgThreadedFilter*>(
                       new AntiVignetting(iface.getOriginalImg(), this, dens, power, rad, xoffset, yoffset, true, addvignetting)));
}

void AntiVignettingTool::putPreviewData()
{
    ImageIface* iface = d->previewWidget->imageIface();
    DImg imDest       = filter()->getTargetImage();

    // Adjust Image BCG.

    BCGContainer settings;
    settings.brightness = (double)(d->brightnessInput->value() / 250.0);
    settings.contrast   = (double)(d->contrastInput->value()   / 100.0) + 1.00;
    settings.gamma      = d->gammaInput->value();

    BCGFilter bcg(&imDest, 0L, settings);
    bcg.startFilterDirectly();

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(), iface->previewHeight())).bits());
    d->previewWidget->updatePreview();
}

void AntiVignettingTool::putFinalData()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    ImageIface* iface = d->previewWidget->imageIface();
    DImg finalImage   = filter()->getTargetImage();

    BCGContainer settings;
    settings.brightness = (double)(d->brightnessInput->value() / 250.0);
    settings.contrast   = (double)(d->contrastInput->value()   / 100.0) + 1.00;
    settings.gamma      = d->gammaInput->value();

    BCGFilter bcg(&finalImage, 0L, settings);
    bcg.startFilterDirectly();

    iface->putOriginalImage(i18n("Vignetting Correction"), finalImage.bits());
    kapp->restoreOverrideCursor();
}

void AntiVignettingTool::blockWidgetSignals(bool b)
{
    d->densityInput->blockSignals(b);
    d->powerInput->blockSignals(b);
    d->radiusInput->blockSignals(b);
    d->brightnessInput->blockSignals(b);
    d->contrastInput->blockSignals(b);
    d->gammaInput->blockSignals(b);
    d->xOffsetInput->blockSignals(b);
    d->yOffsetInput->blockSignals(b);
    d->addVignettingCheck->blockSignals(b);
}

}  // namespace DigikamAntiVignettingImagesPlugin
