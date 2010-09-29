/* ==================-==========================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "lensfuncameraselector.moc"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QWidget>

// KDE includes

#include <kcombobox.h>
#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

using namespace KDcrawIface;

namespace Digikam
{

class LensFunCameraSelector::LensFunCameraSelectorPriv
{
public:

    LensFunCameraSelectorPriv()
        : configUseMetadata("UseMetadata")
    {
        metadataUsage        = 0;
        make                 = 0;
        model                = 0;
        focal                = 0;
        aperture             = 0;
        distance             = 0;
        iface                = 0;
        metadataResult       = 0;
        passiveMetadataUsage = false;
    }

    bool             passiveMetadataUsage;

    QCheckBox*       metadataUsage;
    QLabel*          metadataResult;

    const QString    configUseMetadata;

    RComboBox*       make;
    RComboBox*       model;
    RComboBox*       lens;

    RDoubleNumInput* focal;
    RDoubleNumInput* aperture;
    RDoubleNumInput* distance;

    DMetadata        metadata;

    LensFunIface*    iface;
};

LensFunCameraSelector::LensFunCameraSelector(QWidget* parent)
                     : QWidget(parent), d(new LensFunCameraSelectorPriv)
{
    d->iface           = new LensFunIface();

    QGridLayout* grid  = new QGridLayout(this);
    d->metadataUsage   = new QCheckBox(i18n("Use Metadata"), this);
    d->metadataResult  = new QLabel(this);

    QLabel* makeLabel  = new QLabel(i18nc("camera make",  "Make:"),  this);
    d->make            = new RComboBox(this);
    d->make->setDefaultIndex(0);

    QLabel* modelLabel = new QLabel(i18nc("camera model", "Model:"), this);
    d->model           = new RComboBox(this);
    d->model->setDefaultIndex(0);

    QLabel* lensLabel  = new QLabel(i18nc("camera lens",  "Lens:"),  this);
    d->lens            = new RComboBox(this);
    d->lens->setDefaultIndex(0);

    d->metadataUsage->setEnabled(false);
    d->metadataUsage->setCheckState(Qt::Unchecked);
    d->metadataUsage->setWhatsThis(i18n("Set this option to try to guess the right camera/lens settings "
                                   "from the image metadata (as Exif or XMP)."));

    QLabel* focalLabel = new QLabel(i18n("Focal Length:"), this);
    QLabel* aperLabel  = new QLabel(i18n("Aperture:"), this);
    QLabel* distLabel  = new QLabel(i18n("Subject Distance:"), this);

    d->focal = new RDoubleNumInput(this);
    d->focal->setDecimals(1);
    d->focal->input()->setRange(1.0, 10000.0, 0.01, true);
    d->focal->setDefaultValue(1.0);

    d->aperture = new RDoubleNumInput(this);
    d->aperture->setDecimals(1);
    d->aperture->input()->setRange(1.1, 256.0, 0.1, true);
    d->aperture->setDefaultValue(1.1);

    d->distance = new RDoubleNumInput(this);
    d->distance->setDecimals(1);
    d->distance->input()->setRange(0.0, 10000.0, 0.1, true);
    d->distance->setDefaultValue(0.0);

    grid->addWidget(d->metadataUsage,  0,  0, 1, 3);
    grid->addWidget(d->metadataResult, 1,  0, 1, 3);
    grid->addWidget(makeLabel,         2,  0, 1, 3);
    grid->addWidget(d->make,           3,  0, 1, 3);
    grid->addWidget(modelLabel,        4,  0, 1, 3);
    grid->addWidget(d->model,          5,  0, 1, 3);
    grid->addWidget(lensLabel,         6,  0, 1, 3);
    grid->addWidget(d->lens,           7,  0, 1, 3);
    grid->addWidget(focalLabel,        8,  0, 1, 1);
    grid->addWidget(d->focal,          8,  1, 1, 2);
    grid->addWidget(aperLabel,         9,  0, 1, 1);
    grid->addWidget(d->aperture,       9,  1, 1, 2);
    grid->addWidget(distLabel,         10, 0, 1, 1);
    grid->addWidget(d->distance,       10, 1, 1, 2);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    connect(d->metadataUsage, SIGNAL(toggled(bool)),
            this, SLOT(slotUseMetadata(bool)));

    connect(d->make, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotMakeSelected()));

    connect(d->model, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotModelSelected()));

    connect(d->lens, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotLensSelected()));

    connect(d->focal, SIGNAL(valueChanged(double)),
            this, SLOT(slotFocalChanged()));

    connect(d->aperture, SIGNAL(valueChanged(double)),
            this, SLOT(slotApertureChanged()));

    connect(d->distance, SIGNAL(valueChanged(double)),
            this, SLOT(slotDistanceChanged()));
}

LensFunCameraSelector::~LensFunCameraSelector()
{
    delete d->iface;
    delete d;
}

LensFunIface* LensFunCameraSelector::iface() const
{
    return d->iface;
}

LensFunContainer LensFunCameraSelector::settings()
{
    // Update settings in LensFun interface
    blockSignals(true);
    slotModelSelected();
    slotLensSelected();
    slotFocalChanged();
    slotApertureChanged();
    slotDistanceChanged();
    blockSignals(false);
    return d->iface->settings();
}

void LensFunCameraSelector::setSettings(const LensFunContainer& settings)
{
    blockSignals(true);
    d->iface->setSettings(settings);
    refreshSettingsView();
    blockSignals(false);
}

void LensFunCameraSelector::readSettings(KConfigGroup& group)
{
    setUseMetadata(group.readEntry(d->configUseMetadata, true));
    slotUseMetadata(useMetadata());
}

void LensFunCameraSelector::writeSettings(KConfigGroup& group)
{
    group.writeEntry(d->configUseMetadata, useMetadata());
}

void LensFunCameraSelector::findFromMetadata(const DMetadata& meta)
{
    d->metadata = meta;
    findFromMetadata();
}

void LensFunCameraSelector::enableUseMetadata(bool b)
{
    d->metadataUsage->setEnabled(b);
}

void LensFunCameraSelector::setUseMetadata(bool b)
{
    d->metadataUsage->setChecked(b);
}

bool LensFunCameraSelector::useMetadata() const
{
      return (d->metadataUsage->isChecked());
}

void LensFunCameraSelector::setPassiveMetadataUsage(bool b)
{
    d->passiveMetadataUsage = b;
}

void LensFunCameraSelector::slotUseMetadata(bool b)
{
    d->metadataResult->clear();

    if (b)
    {
        if (d->passiveMetadataUsage)
        {
            d->make->setEnabled(false);
            d->model->setEnabled(false);
            d->lens->setEnabled(false);
            d->focal->setEnabled(false);
            d->aperture->setEnabled(false);
            d->distance->setEnabled(false);
            emit signalLensSettingsChanged();
        }
        else
        {
            LensFunIface::MetadataMatch ret = findFromMetadata();
            switch (ret)
            {
                case LensFunIface::MetadataNoMatch:
                    d->metadataResult->setText(i18n("No match found"));
                    d->metadataResult->setStyleSheet(QString("QLabel {color: red;}"));
                    break;
                case LensFunIface::MetadataPartialMatch:
                    d->metadataResult->setText(i18n("Partial match found"));
                    d->metadataResult->setStyleSheet(QString("QLabel {color: orange;}"));
                    break;
                default:
                    d->metadataResult->setText(i18n("Exact match found"));
                    d->metadataResult->setStyleSheet(QString("QLabel {color: green;}"));
                    break;
            }
            emit signalLensSettingsChanged();
        }
    }
    else
    {
        d->make->setEnabled(true);
        d->model->setEnabled(true);
        d->lens->setEnabled(true);
        d->focal->setEnabled(true);
        d->aperture->setEnabled(true);
        d->distance->setEnabled(true);
        slotMakeSelected();
    }
}

LensFunIface::MetadataMatch LensFunCameraSelector::findFromMetadata()
{
    if (d->metadata.isEmpty())
    {
        d->metadataUsage->setCheckState(Qt::Unchecked);
        enableUseMetadata(false);
    }
    else
    {
        d->metadataUsage->setCheckState(Qt::Checked);
        enableUseMetadata(true);
    }

    LensFunIface::MetadataMatch ret = d->iface->findFromMetadata(d->metadata);
    refreshSettingsView();
    return ret;
}

void LensFunCameraSelector::refreshSettingsView()
{
    int makerIdx = -1;

    if (d->iface->usedCamera())
    {
        makerIdx = d->make->combo()->findText(d->iface->settings().cameraMake);
        kDebug() << "makerIdx: " << makerIdx << " (" << d->iface->settings().cameraMake << ")";
    }

    if (makerIdx >= 0)
    {
        d->make->setCurrentIndex(makerIdx);
        d->make->setEnabled(d->passiveMetadataUsage);
        updateDeviceCombos();
    }

    // ------------------------------------------------------------------------------------------------

    int modelIdx = -1;

    if (d->iface->usedCamera())
    {
        modelIdx = d->model->combo()->findText(d->iface->settings().cameraModel);
        kDebug() << "modelIdx: " << modelIdx << " (" << d->iface->settings().cameraModel << ")";
    }

    if (modelIdx >= 0)
    {
        d->model->setCurrentIndex(modelIdx);
        d->model->setEnabled(d->passiveMetadataUsage);
        updateLensCombo();
    }

    // ------------------------------------------------------------------------------------------------

    int lensIdx = -1;

    if (d->iface->usedLens())
    {
        lensIdx = d->lens->combo()->findText(d->iface->settings().lensModel);
        kDebug() << "lensIdx: " << lensIdx << " (" << d->iface->settings().lensModel << ")";
    }

    if (lensIdx >= 0)
    {
        // found lens model directly, best case :)
        d->lens->setCurrentIndex(lensIdx);
        d->lens->setEnabled(d->passiveMetadataUsage);
    }

    // ------------------------------------------------------------------------------------------------

    if (d->iface->settings().focalLength != -1.0)
    {
        d->focal->setValue(d->iface->settings().focalLength);
        d->focal->setEnabled(d->passiveMetadataUsage);
    }

    if (d->iface->settings().aperture != -1.0)
    {
        d->aperture->setValue(d->iface->settings().aperture);
        d->aperture->setEnabled(d->passiveMetadataUsage);
    }

    if (d->iface->settings().subjectDistance != -1.0)
    {
        d->distance->setValue(d->iface->settings().subjectDistance);
        d->distance->setEnabled(d->passiveMetadataUsage);
    }
}

void LensFunCameraSelector::updateDeviceCombos()
{
    d->make->blockSignals(true);
    d->model->blockSignals(true);

    const lfCamera* const* it = d->iface->lensFunCameras();

    // reset box
    d->model->combo()->clear();

    bool firstRun = false;
    if ( d->make->combo()->count() == 0 )
       firstRun = true;

    while ( *it )
    {
       if ( firstRun )
       {
           // Maker DB does not change, so we fill it only once.
           if ( (*it)->Maker )
           {
                QString t( (*it)->Maker );
                if ( d->make->combo()->findText( t, Qt::MatchExactly ) < 0 )
                    d->make->addItem( t );
           }
       }

       // Fill models for current selected maker
       if ( (*it)->Model && (*it)->Maker == d->make->combo()->currentText() )
       {
            LensFunIface::DevicePtr dev = *it;
            QVariant b                  = qVariantFromValue(dev);
            d->model->combo()->addItem( dev->Model, b );
       }

       ++it;
    }
    d->make->combo()->model()->sort(0, Qt::AscendingOrder);
    d->model->combo()->model()->sort(0, Qt::AscendingOrder);

    d->make->blockSignals(false);
    d->model->blockSignals(false);
}

void LensFunCameraSelector::updateLensCombo()
{
    d->lens->blockSignals(true);
    d->lens->combo()->clear();

    QVariant v = d->model->combo()->itemData( d->model->currentIndex() );
    if (!v.isValid() || v.isNull())
    {
        kDebug() << "Invalid variant value for device!";
        return;
    }
    kDebug() << "variant: " << v;

    LensFunIface::DevicePtr dev = v.value<LensFunIface::DevicePtr>();
    if (!dev)
    {
        kDebug() << "Device is null!";
        return;
    }

    kDebug() << "dev: " << dev->Maker << " :: " << dev->Model;

    d->lens->blockSignals(true);
    const lfLens** lenses     = d->iface->lensFunDataBase()->FindLenses( dev, NULL, NULL );
    LensFunContainer settings = d->iface->settings();
    settings.cropFactor       = dev ? dev->CropFactor : -1.0;
    d->iface->setSettings(settings);

    while (lenses && *lenses)
    {
        LensFunIface::LensPtr lens = *lenses;
        QVariant b                 = qVariantFromValue(lens);
        d->lens->combo()->addItem(lens->Model, b);
        ++lenses;
    }
    d->lens->combo()->model()->sort(0, Qt::AscendingOrder);
    d->lens->blockSignals(false);
}

void LensFunCameraSelector::slotMakeSelected()
{
    updateDeviceCombos();
    slotModelSelected();

    // Fill Lens list for current Maker & Model and fire signalLensSettingsChanged()
    updateLensCombo();
    slotLensSelected();
}

void LensFunCameraSelector::slotModelSelected()
{
    QVariant v = d->model->combo()->itemData( d->model->currentIndex() );
    d->iface->setUsedCamera(d->metadataUsage->isChecked() && d->passiveMetadataUsage ? 0 :
                            v.value<LensFunIface::DevicePtr>());
    emit signalLensSettingsChanged();
}

void LensFunCameraSelector::slotLensSelected()
{
    QVariant v = d->lens->combo()->itemData( d->lens->currentIndex() );
    d->iface->setUsedLens(d->metadataUsage->isChecked() && d->passiveMetadataUsage ? 0 :
                          v.value<LensFunIface::LensPtr>());

    LensFunContainer settings = d->iface->settings();

    if (d->iface->usedLens() &&
        settings.cropFactor <= 0.0) // this should not happen
        settings.cropFactor = d->iface->usedLens()->CropFactor;
    else 
        settings.cropFactor = -1.0;

    d->iface->setSettings(settings);
    emit signalLensSettingsChanged();
}

void LensFunCameraSelector::slotFocalChanged()
{
    LensFunContainer settings = d->iface->settings();
    settings.focalLength = d->metadataUsage->isChecked() && d->passiveMetadataUsage ? -1.0 :
                           d->focal->value();
    d->iface->setSettings(settings);
    emit signalLensSettingsChanged();
}

void LensFunCameraSelector::slotApertureChanged()
{
    LensFunContainer settings = d->iface->settings();
    settings.aperture = d->metadataUsage->isChecked() && d->passiveMetadataUsage ? -1.0 :
                        d->aperture->value();
    d->iface->setSettings(settings);
    emit signalLensSettingsChanged();
}

void LensFunCameraSelector::slotDistanceChanged()
{
    LensFunContainer settings = d->iface->settings();
    settings.subjectDistance  = d->metadataUsage->isChecked() && d->passiveMetadataUsage ? -1.0 :
                                d->distance->value();
    d->iface->setSettings(settings);
    emit signalLensSettingsChanged();
}

}  // namespace Digikam
