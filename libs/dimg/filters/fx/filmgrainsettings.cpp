/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-10
 * Description : Film Grain settings view.
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "filmgrainsettings.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>

// KDE includes

#include <kdebug.h>
#include <kurl.h>
#include <kdialog.h>
#include <klocale.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "rexpanderbox.h"

using namespace KDcrawIface;

namespace Digikam
{

class FilmGrainSettingsPriv
{
public:

    FilmGrainSettingsPriv() :
        configSensitivityLumAdjustmentEntry("SensitivityLumAdjustment"),
        configShadowsLumAdjustmentEntry("ShadowsLumAdjustment"),
        configMidtonesLumAdjustmentEntry("MidtonesLumAdjustment"),
        configHighlightsLumAdjustmentEntry("HighlightsLumAdjustment"),
        configSensitivityChromaAdjustmentEntry("SensitivityChromaAdjustment"),
        configShadowsChromaAdjustmentEntry("ShadowsChromaAdjustment"),
        configMidtonesChromaAdjustmentEntry("MidtonesChromaAdjustment"),
        configHighlightsChromaAdjustmentEntry("HighlightsChromaAdjustment"),
        sensibilityLumInput(0),
        shadowsLumInput(0),
        midtonesLumInput(0),
        highlightsLumInput(0),
        sensibilityChromaInput(0),
        shadowsChromaInput(0),
        midtonesChromaInput(0),
        highlightsChromaInput(0)
        {}

    const QString configSensitivityLumAdjustmentEntry;
    const QString configShadowsLumAdjustmentEntry;
    const QString configMidtonesLumAdjustmentEntry;
    const QString configHighlightsLumAdjustmentEntry;
    const QString configSensitivityChromaAdjustmentEntry;
    const QString configShadowsChromaAdjustmentEntry;
    const QString configMidtonesChromaAdjustmentEntry;
    const QString configHighlightsChromaAdjustmentEntry;
    
    RIntNumInput* sensibilityLumInput;
    RIntNumInput* shadowsLumInput; 
    RIntNumInput* midtonesLumInput;
    RIntNumInput* highlightsLumInput;
    RIntNumInput* sensibilityChromaInput;
    RIntNumInput* shadowsChromaInput; 
    RIntNumInput* midtonesChromaInput;
    RIntNumInput* highlightsChromaInput;
    
    RExpanderBox* expanderBox;
};

FilmGrainSettings::FilmGrainSettings(QWidget* parent)
                 : QWidget(parent),
                   d(new FilmGrainSettingsPriv)
{
    QGridLayout* grid = new QGridLayout(parent);
    QWidget* firstPage = new QWidget();
    QGridLayout* grid1 = new QGridLayout(firstPage);
    
    // -------------------------------------------------------------
    
    QLabel* label1         = new QLabel(i18n("Sensitivity (ISO):"), firstPage);
    d->sensibilityLumInput = new RIntNumInput(firstPage);
    d->sensibilityLumInput->setRange(800, 51200, 100);
    d->sensibilityLumInput->setSliderEnabled(true);
    d->sensibilityLumInput->setDefaultValue(2400);
    d->sensibilityLumInput->setWhatsThis(i18n("Set here the film ISO-sensitivity to use for "
                                              "simulating the film graininess."));
  
    // -------------------------------------------------------------

    QLabel* label2        = new QLabel(i18n("Shadows:"), firstPage);
    d->highlightsLumInput = new RIntNumInput(firstPage);
    d->highlightsLumInput->setRange(-100, 100, 1);
    d->highlightsLumInput->setSliderEnabled(true);
    d->highlightsLumInput->setDefaultValue(-100);
    d->highlightsLumInput->setWhatsThis(i18n("Set how much the filter affects highlights."));
    
    // -------------------------------------------------------------

    QLabel* label3      = new QLabel(i18n("Midtones:"), firstPage);
    d->midtonesLumInput = new RIntNumInput(firstPage);
    d->midtonesLumInput->setRange(-100, 100, 1);
    d->midtonesLumInput->setSliderEnabled(true);
    d->midtonesLumInput->setDefaultValue(0);
    d->midtonesLumInput->setWhatsThis(i18n("Set how much the filter affects midtones."));
    
 
    // -------------------------------------------------------------

    QLabel* label4     = new QLabel(i18n("Highlights:"), firstPage);
    d->shadowsLumInput = new RIntNumInput(firstPage);
    d->shadowsLumInput->setRange(-100, 100, 1);
    d->shadowsLumInput->setSliderEnabled(true);
    d->shadowsLumInput->setDefaultValue(-100);
    d->shadowsLumInput->setWhatsThis(i18n("Set how much the filter affects shadows."));
  
    grid1->addWidget(label1,                    1, 0, 1, 1);
    grid1->addWidget(d->sensibilityLumInput,    2, 0, 1, 1);
    grid1->addWidget(label2,                    3, 0, 1, 1);
    grid1->addWidget(d->shadowsLumInput,        4, 0, 1, 1);  
    grid1->addWidget(label3,                    5, 0, 1, 1);
    grid1->addWidget(d->midtonesLumInput,       6, 0, 1, 1); 
    grid1->addWidget(label4,                    7, 0, 1, 1); 
    grid1->addWidget(d->highlightsLumInput,     8, 0, 1, 1);
    grid1->setMargin(KDialog::spacingHint());
    grid1->setSpacing(KDialog::spacingHint());
    
    // -------------------------------------------------------------

    QWidget* secondPage = new QWidget();
    QGridLayout* grid2  = new QGridLayout( secondPage );

    QLabel* label5            = new QLabel(i18n("Sensitivity (ISO):"), secondPage);
    d->sensibilityChromaInput = new RIntNumInput(secondPage);
    d->sensibilityChromaInput->setRange(800, 51200, 100);
    d->sensibilityChromaInput->setSliderEnabled(true);
    d->sensibilityChromaInput->setDefaultValue(2400);
    d->sensibilityChromaInput->setWhatsThis(i18n("Set here the film ISO-sensitivity to use for "
                                                 "simulating the CCD noise."));
  
    // -------------------------------------------------------------

    QLabel* label6           = new QLabel(i18n("Shadows:"), secondPage);
    d->highlightsChromaInput = new RIntNumInput(secondPage);
    d->highlightsChromaInput->setRange(-100, 100, 1);
    d->highlightsChromaInput->setSliderEnabled(true);
    d->highlightsChromaInput->setDefaultValue(-100);
    d->highlightsChromaInput->setWhatsThis(i18n("Set how much the filter affects highlights."));
    
    // -------------------------------------------------------------
    
    QLabel* label7         = new QLabel(i18n("Midtones:"), secondPage);
    d->midtonesChromaInput = new RIntNumInput(secondPage);
    d->midtonesChromaInput->setRange(-100, 100, 1);
    d->midtonesChromaInput->setSliderEnabled(true);
    d->midtonesChromaInput->setDefaultValue(0);
    d->midtonesChromaInput->setWhatsThis(i18n("Set how much the filter affects midtones."));
    
 
    // -------------------------------------------------------------

    QLabel* label8        = new QLabel(i18n("Highlights:"), secondPage);
    d->shadowsChromaInput = new RIntNumInput(secondPage);
    d->shadowsChromaInput->setRange(-100, 100, 1);
    d->shadowsChromaInput->setSliderEnabled(true);
    d->shadowsChromaInput->setDefaultValue(-100);
    d->shadowsChromaInput->setWhatsThis(i18n("Set how much the filter affects shadows."));
      
    grid2->addWidget(label5,                    1, 0, 1, 1);
    grid2->addWidget(d->sensibilityChromaInput, 2, 0, 1, 1);
    grid2->addWidget(label6,                    3, 0, 1, 1);
    grid2->addWidget(d->shadowsChromaInput,     4, 0, 1, 1);  
    grid2->addWidget(label7,                    5, 0, 1, 1);
    grid2->addWidget(d->midtonesChromaInput,    6, 0, 1, 1); 
    grid2->addWidget(label8,                    7, 0, 1, 1); 
    grid2->addWidget(d->highlightsChromaInput,  8, 0, 1, 1);
    grid2->setMargin(KDialog::spacingHint());
    grid2->setSpacing(KDialog::spacingHint());    
    
    // -------------------------------------------------------------

    d->expanderBox = new RExpanderBox();
    d->expanderBox->setObjectName("Noise Expander");
    d->expanderBox->addItem(firstPage, SmallIcon("filmgrain"), i18n("Luminance noise"),
                            QString("LuminanceSettingsContainer"), true);
    d->expanderBox->addItem(secondPage, SmallIcon("camera-photo"), i18n("Chrominance noise"),
                            QString("ChrominanceSettingsContainer"), true);
    d->expanderBox->addStretch();

    grid->addWidget(d->expanderBox, 0, 0, 1, 1);
    grid->setRowStretch(0, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    connect(d->sensibilityLumInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
            
    connect(d->shadowsLumInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
            
    connect(d->midtonesLumInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
            
    connect(d->highlightsLumInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
            
    connect(d->sensibilityChromaInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
            
    connect(d->shadowsChromaInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
            
    connect(d->midtonesChromaInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
            
    connect(d->highlightsChromaInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

FilmGrainSettings::~FilmGrainSettings()
{
    delete d;
}

FilmGrainContainer FilmGrainSettings::settings() const
{
    FilmGrainContainer prm;
    prm.lum_sensibility    = d->sensibilityLumInput->value();
    prm.lum_shadows        = d->shadowsLumInput->value();
    prm.lum_midtones       = d->midtonesLumInput->value();
    prm.lum_highlights     = d->highlightsLumInput->value();
    prm.chroma_sensibility = d->sensibilityChromaInput->value();
    prm.chroma_shadows     = d->shadowsChromaInput->value(); 
    prm.chroma_midtones    = d->midtonesChromaInput->value();
    prm.chroma_highlights  = d->highlightsChromaInput->value();
    return prm;
}

void FilmGrainSettings::setSettings(const FilmGrainContainer& settings)
{
    blockSignals(true);

    d->sensibilityLumInput->setValue(settings.lum_sensibility);
    d->shadowsLumInput->setValue(settings.lum_shadows);
    d->midtonesLumInput->setValue(settings.lum_midtones);
    d->highlightsLumInput->setValue(settings.lum_highlights);
    d->sensibilityChromaInput->setValue(settings.chroma_sensibility);
    d->shadowsChromaInput->setValue(settings.chroma_shadows); 
    d->midtonesChromaInput->setValue(settings.chroma_midtones);
    d->highlightsChromaInput->setValue(settings.chroma_highlights);
    
    blockSignals(false);
}

void FilmGrainSettings::resetToDefault()
{
    blockSignals(true);
    d->sensibilityLumInput->slotReset();
    d->shadowsLumInput->slotReset();
    d->midtonesLumInput->slotReset();
    d->highlightsLumInput->slotReset();
    d->sensibilityChromaInput->slotReset();
    d->shadowsChromaInput->slotReset();
    d->midtonesChromaInput->slotReset();
    d->highlightsChromaInput->slotReset();
    blockSignals(false);
}

FilmGrainContainer FilmGrainSettings::defaultSettings() const
{
    FilmGrainContainer prm;
    prm.lum_sensibility    = d->sensibilityLumInput->defaultValue();
    prm.lum_shadows        = d->shadowsLumInput->defaultValue();
    prm.lum_midtones       = d->midtonesLumInput->defaultValue();
    prm.lum_highlights     = d->highlightsLumInput->defaultValue();
    prm.chroma_sensibility = d->sensibilityChromaInput->defaultValue();
    prm.chroma_shadows     = d->shadowsChromaInput->defaultValue(); 
    prm.chroma_midtones    = d->midtonesChromaInput->defaultValue();
    prm.chroma_highlights  = d->highlightsChromaInput->defaultValue();
    return prm;
}

void FilmGrainSettings::readSettings(KConfigGroup& group)
{
    FilmGrainContainer prm;
    FilmGrainContainer defaultPrm = defaultSettings();

    prm.lum_sensibility    = group.readEntry(d->configSensitivityLumAdjustmentEntry,    defaultPrm.lum_sensibility);
    prm.lum_shadows        = group.readEntry(d->configShadowsLumAdjustmentEntry,        defaultPrm.lum_shadows);
    prm.lum_midtones       = group.readEntry(d->configMidtonesLumAdjustmentEntry,       defaultPrm.lum_midtones);
    prm.lum_highlights     = group.readEntry(d->configHighlightsLumAdjustmentEntry,     defaultPrm.lum_highlights);
    prm.chroma_sensibility = group.readEntry(d->configSensitivityChromaAdjustmentEntry, defaultPrm.chroma_sensibility);
    prm.chroma_shadows     = group.readEntry(d->configShadowsChromaAdjustmentEntry,     defaultPrm.chroma_shadows);
    prm.chroma_midtones    = group.readEntry(d->configMidtonesChromaAdjustmentEntry,    defaultPrm.chroma_midtones);
    prm.chroma_highlights  = group.readEntry(d->configHighlightsChromaAdjustmentEntry,  defaultPrm.chroma_highlights);
                                                     
    setSettings(prm);
}

void FilmGrainSettings::writeSettings(KConfigGroup& group)
{
    FilmGrainContainer prm = settings();

    group.writeEntry(d->configSensitivityLumAdjustmentEntry,    prm.lum_sensibility);
    group.writeEntry(d->configShadowsLumAdjustmentEntry,        prm.lum_shadows);
    group.writeEntry(d->configMidtonesLumAdjustmentEntry,       prm.lum_midtones);
    group.writeEntry(d->configHighlightsLumAdjustmentEntry,     prm.lum_highlights);
    
    group.writeEntry(d->configSensitivityChromaAdjustmentEntry, prm.chroma_sensibility);
    group.writeEntry(d->configShadowsChromaAdjustmentEntry,     prm.chroma_shadows);
    group.writeEntry(d->configMidtonesChromaAdjustmentEntry,    prm.chroma_midtones);
    group.writeEntry(d->configHighlightsChromaAdjustmentEntry,  prm.chroma_highlights);
}

}  // namespace Digikam
