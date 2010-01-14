/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-26
 * Description : a digiKam image editor plugin to restore
 *               a photograph
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "restorationtool.moc"

// Qt includes

#include <QFile>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QPixmap>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>

// Local includes

#include "editortoolsettings.h"
#include "greycstorationiface.h"
#include "greycstorationsettings.h"
#include "greycstorationwidget.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "version.h"

using namespace Digikam;

namespace DigikamRestorationImagesPlugin
{

class RestorationToolPriv
{
public:

    RestorationToolPriv() :
        configGroupName("restoration Tool"),
        configPresetEntry("Preset"),
        configFastApproxEntry("FastApprox"),
        configInterpolationEntry("Interpolation"),
        configAmplitudeEntry("Amplitude"),
        configSharpnessEntry("Sharpness"),
        configAnisotropyEntry("Anisotropy"),
        configAlphaEntry("Alpha"),
        configSigmaEntry("Sigma"),
        configGaussPrecEntry("GaussPrec"),
        configDlEntry("Dl"), configDaEntry("Da"),
        configIterationEntry("Iteration"),
        configTileEntry("Tile"),
        configBTileEntry("BTile"),

        mainTab(0),
        restorationTypeCB(0),
        settingsWidget(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString         configGroupName;
    const QString         configPresetEntry;
    const QString         configFastApproxEntry;
    const QString         configInterpolationEntry;
    const QString         configAmplitudeEntry;
    const QString         configSharpnessEntry;
    const QString         configAnisotropyEntry;
    const QString         configAlphaEntry;
    const QString         configSigmaEntry;
    const QString         configGaussPrecEntry;
    const QString         configDlEntry;
    const QString         configDaEntry;
    const QString         configIterationEntry;
    const QString         configTileEntry;
    const QString         configBTileEntry;

    KTabWidget*           mainTab;

    KComboBox*            restorationTypeCB;

    GreycstorationWidget* settingsWidget;
    ImageRegionWidget*    previewWidget;
    EditorToolSettings*   gboxSettings;
};

RestorationTool::RestorationTool(QObject* parent)
               : EditorToolThreaded(parent),
                 d(new RestorationToolPriv)
{
    setObjectName("restoration");
    setToolName(i18n("Restoration"));
    setToolIcon(SmallIcon("restoration"));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Try);

    QGridLayout* gridSettings = new QGridLayout(d->gboxSettings->plainPage());
    d->mainTab = new KTabWidget( d->gboxSettings->plainPage() );

    QWidget* firstPage = new QWidget( d->mainTab );
    QGridLayout* grid  = new QGridLayout(firstPage);
    d->mainTab->addTab( firstPage, i18n("Preset") );

    KUrlLabel *cimgLogoLabel = new KUrlLabel(firstPage);
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setUrl("http://cimg.sourceforge.net");
    cimgLogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-cimg.png")));
    cimgLogoLabel->setToolTip( i18n("Visit CImg library website"));

    QLabel *typeLabel   = new QLabel(i18n("Filtering type:"), firstPage);
    typeLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    d->restorationTypeCB = new KComboBox(firstPage);
    d->restorationTypeCB->addItem( i18nc("no restoration preset", "None") );
    d->restorationTypeCB->addItem( i18n("Reduce Uniform Noise") );
    d->restorationTypeCB->addItem( i18n("Reduce JPEG Artifacts") );
    d->restorationTypeCB->addItem( i18n("Reduce Texturing") );
    d->restorationTypeCB->setWhatsThis(i18n("<p>Select the filter preset to use for photograph restoration here:</p>"
                                            "<p><b>None</b>: Most common values. Puts settings to default.<br/>"
                                            "<b>Reduce Uniform Noise</b>: reduce small image artifacts such as sensor noise.<br/>"
                                            "<b>Reduce JPEG Artifacts</b>: reduce large image artifacts, such as a JPEG compression mosaic.<br/>"
                                            "<b>Reduce Texturing</b>: reduce image artifacts, such as paper texture, or Moire patterns "
                                            "on scanned images.</p>"));

    grid->addWidget(cimgLogoLabel,        0, 1, 1, 1);
    grid->addWidget(typeLabel,            1, 0, 1, 1);
    grid->addWidget(d->restorationTypeCB, 1, 1, 1, 1);
    grid->setRowStretch(1, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(0);

    // -------------------------------------------------------------

    d->settingsWidget = new GreycstorationWidget( d->mainTab );
    gridSettings->addWidget(d->mainTab,                               0, 1, 1, 1);
    gridSettings->addWidget(new QLabel(d->gboxSettings->plainPage()), 1, 1, 1, 1);
    gridSettings->setMargin(d->gboxSettings->spacingHint());
    gridSettings->setSpacing(d->gboxSettings->spacingHint());
    gridSettings->setRowStretch(2, 10);

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);       
    init();

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(processCImgUrl(const QString&)));

    connect(d->restorationTypeCB, SIGNAL(activated(int)),
            this, SLOT(slotResetValues(int)));

    // -------------------------------------------------------------

    GreycstorationSettings defaults;
    defaults.setRestorationDefaultSettings();
    d->settingsWidget->setDefaultSettings(defaults);
}

RestorationTool::~RestorationTool()
{
    delete d;
}

void RestorationTool::renderingFinished()
{
    d->mainTab->setEnabled(true);
    toolView()->setEnabled(true);
}

void RestorationTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    GreycstorationSettings settings;
    GreycstorationSettings defaults;
    defaults.setRestorationDefaultSettings();

    settings.fastApprox = group.readEntry(d->configFastApproxEntry,    defaults.fastApprox);
    settings.interp     = group.readEntry(d->configInterpolationEntry, defaults.interp);
    settings.amplitude  = group.readEntry(d->configAmplitudeEntry,     (double)defaults.amplitude);
    settings.sharpness  = group.readEntry(d->configSharpnessEntry,     (double)defaults.sharpness);
    settings.anisotropy = group.readEntry(d->configAnisotropyEntry,    (double)defaults.anisotropy);
    settings.alpha      = group.readEntry(d->configAlphaEntry,         (double)defaults.alpha);
    settings.sigma      = group.readEntry(d->configSigmaEntry,         (double)defaults.sigma);
    settings.gaussPrec  = group.readEntry(d->configGaussPrecEntry,     (double)defaults.gaussPrec);
    settings.dl         = group.readEntry(d->configDlEntry,            (double)defaults.dl);
    settings.da         = group.readEntry(d->configDaEntry,            (double)defaults.da);
    settings.nbIter     = group.readEntry(d->configIterationEntry,     defaults.nbIter);
    settings.tile       = group.readEntry(d->configTileEntry,          defaults.tile);
    settings.btile      = group.readEntry(d->configBTileEntry,         defaults.btile);
    d->settingsWidget->setSettings(settings);

    int p = group.readEntry(d->configPresetEntry, (int)NoPreset);
    d->restorationTypeCB->setCurrentIndex(p);
    if (p == NoPreset)
        d->settingsWidget->setEnabled(true);
    else
        d->settingsWidget->setEnabled(false);
}

void RestorationTool::writeSettings()
{
    GreycstorationSettings settings = d->settingsWidget->getSettings();
    KSharedConfig::Ptr config       = KGlobal::config();
    KConfigGroup group              = config->group(d->configGroupName);

    group.writeEntry(d->configPresetEntry,        d->restorationTypeCB->currentIndex());
    group.writeEntry(d->configFastApproxEntry,    settings.fastApprox);
    group.writeEntry(d->configInterpolationEntry, settings.interp);
    group.writeEntry(d->configAmplitudeEntry,     (double)settings.amplitude);
    group.writeEntry(d->configSharpnessEntry,     (double)settings.sharpness);
    group.writeEntry(d->configAnisotropyEntry,    (double)settings.anisotropy);
    group.writeEntry(d->configAlphaEntry,         (double)settings.alpha);
    group.writeEntry(d->configSigmaEntry,         (double)settings.sigma);
    group.writeEntry(d->configGaussPrecEntry,     (double)settings.gaussPrec);
    group.writeEntry(d->configDlEntry,            (double)settings.dl);
    group.writeEntry(d->configDaEntry,            (double)settings.da);
    group.writeEntry(d->configIterationEntry,     settings.nbIter);
    group.writeEntry(d->configTileEntry,          settings.tile);
    group.writeEntry(d->configBTileEntry,         settings.btile);
    group.sync();
}

void RestorationTool::slotResetValues(int i)
{
    if (i == NoPreset)
        d->settingsWidget->setEnabled(true);
    else
        d->settingsWidget->setEnabled(false);

    slotResetSettings();
}

void RestorationTool::slotResetSettings()
{
    GreycstorationSettings settings;
    settings.setRestorationDefaultSettings();

    switch(d->restorationTypeCB->currentIndex())
    {
        case ReduceUniformNoise:
        {
            settings.amplitude = 40.0;
            break;
        }

        case ReduceJPEGArtefacts:
        {
            settings.sharpness = 0.3F;
            settings.sigma     = 1.0;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }

        case ReduceTexturing:
        {
            settings.sharpness = 0.5F;
            settings.sigma     = 1.5;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }
    }

    d->settingsWidget->setSettings(settings);
}

void RestorationTool::processCImgUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

void RestorationTool::prepareEffect()
{
    d->mainTab->setEnabled(false);
    toolView()->setEnabled(false);
    
    DImg previewImage = d->previewWidget->getOriginalRegionImage();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new GreycstorationIface(&previewImage,
                                                d->settingsWidget->getSettings(), GreycstorationIface::Restore,
                                                0, 0, QImage(), this)));
}

void RestorationTool::prepareFinal()
{
    d->mainTab->setEnabled(false);
    toolView()->setEnabled(false);
    
    ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    DImg originalImage(iface.originalWidth(), iface.originalHeight(),
                       iface.originalSixteenBit(), iface.originalHasAlpha(), data);

    setFilter(dynamic_cast<DImgThreadedFilter*>(new GreycstorationIface(&originalImage,
                                                d->settingsWidget->getSettings(), GreycstorationIface::Restore,
                                                0, 0, QImage(), this)));

    delete [] data;
}

void RestorationTool::putPreviewData()
{
    DImg imDest = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(imDest);
}

void RestorationTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Restoration"), filter()->getTargetImage().bits());
}

void RestorationTool::slotLoadSettings()
{
    KUrl loadRestorationFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Photograph Restoration Settings File to Load")) );
    if ( loadRestorationFile.isEmpty() )
       return;

    QFile file(loadRestorationFile.toLocalFile());

    if ( file.open(QIODevice::ReadOnly) )
    {
        if (!d->settingsWidget->loadSettings(file, QString("# Photograph Restoration Configuration File V2")))
        {
           KMessageBox::error(kapp->activeWindow(),
                        i18n("\"%1\" is not a Photograph Restoration settings text file.",
                             loadRestorationFile.fileName()));
           file.close();
           return;
        }

        slotEffect();
    }
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Photograph Restoration text file."));

    file.close();
    d->restorationTypeCB->blockSignals(true);
    d->restorationTypeCB->setCurrentIndex((int)NoPreset);
    d->restorationTypeCB->blockSignals(false);
    d->settingsWidget->setEnabled(true);
}

void RestorationTool::slotSaveAsSettings()
{
    KUrl saveRestorationFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Photograph Restoration Settings File to Save")) );
    if ( saveRestorationFile.isEmpty() )
       return;

    QFile file(saveRestorationFile.toLocalFile());

    if ( file.open(QIODevice::WriteOnly) )
        d->settingsWidget->saveSettings(file, QString("# Photograph Restoration Configuration File V2"));
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Restoration text file."));

    file.close();
}

}  // namespace DigikamRestorationImagesPlugin
