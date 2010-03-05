/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-20
 * Description : a digiKam image plugin to add a border
 *               around an image.
 *
 * Copyright 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "bordertool.moc"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>

// KDE includes

#include <kapplication.h>
#include <kcolorbutton.h>
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
#include <libkdcraw/rcombobox.h>

// Local includes

#include "borderfilter.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"

using namespace KDcrawIface;

namespace DigikamBorderImagesPlugin
{

class BorderToolPriv
{
public:

    BorderToolPriv() :
        configGroupName("border Tool"),
        configBorderTypeEntry("Border Type"),
        configBorderPercentEntry("Border Percent"),
        configBorderWidthEntry("Border Width"),
        configPreserveAspectRatioEntry("Preserve Aspect Ratio"),
        configSolidColorEntry("Solid Color"),
        configNiepceBorderColorEntry("Niepce Border Color"),
        configNiepceLineColorEntry("Niepce Line Color"),
        configBevelUpperLeftColorEntry("Bevel Upper Left Color"),
        configBevelLowerRightColorEntry("Bevel Lower Right Color"),
        configDecorativeFirstColorEntry("Decorative First Color"),
        configDecorativeSecondColorEntry("Decorative Second Color"),

        preserveAspectRatio(0),
        labelBackground(0),
        labelBorderPercent(0),
        labelBorderWidth(0),
        labelForeground(0),
        firstColorButton(0),
        secondColorButton(0),
        gboxSettings(0),
        previewWidget(0),
        borderType(0),
        borderPercent(0),
        borderWidth(0)
        {}

    const QString       configGroupName;
    const QString       configBorderTypeEntry;
    const QString       configBorderPercentEntry;
    const QString       configBorderWidthEntry;
    const QString       configPreserveAspectRatioEntry;
    const QString       configSolidColorEntry;
    const QString       configNiepceBorderColorEntry;
    const QString       configNiepceLineColorEntry;
    const QString       configBevelUpperLeftColorEntry;
    const QString       configBevelLowerRightColorEntry;
    const QString       configDecorativeFirstColorEntry;
    const QString       configDecorativeSecondColorEntry;

    QCheckBox*          preserveAspectRatio;

    QColor              bevelLowerRightColor;
    QColor              bevelUpperLeftColor;
    QColor              decorativeFirstColor;
    QColor              decorativeSecondColor;
    QColor              niepceBorderColor;
    QColor              niepceLineColor;
    QColor              solidColor;

    QLabel*             labelBackground;
    QLabel*             labelBorderPercent;
    QLabel*             labelBorderWidth;
    QLabel*             labelForeground;

    KColorButton*       firstColorButton;
    KColorButton*       secondColorButton;

    EditorToolSettings* gboxSettings;
    ImageGuideWidget*   previewWidget;

    RComboBox*          borderType;
    RIntNumInput*       borderPercent;
    RIntNumInput*       borderWidth;
};

BorderTool::BorderTool(QObject* parent)
          : EditorToolThreaded(parent),
            d(new BorderToolPriv)
{
    setObjectName("border");
    setToolName(i18n("Add Border"));
    setToolIcon(SmallIcon("bordertool"));

    d->previewWidget = new ImageGuideWidget(0, false, ImageGuideWidget::HVGuideMode);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::UnSplitPreviewModes);
    
    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;

    // -------------------------------------------------------------

    QLabel *label1 = new QLabel(i18n("Type:"));
    d->borderType  = new RComboBox( );
    d->borderType->addItem(i18nc("solid border type", "Solid"));
    // NOTE: Niepce is a real name. This is the first guy in the world to have built a camera.
    d->borderType->addItem("Niepce");
    d->borderType->addItem(i18nc("beveled border type", "Beveled"));
    d->borderType->addItem(i18n("Decorative Pine"));
    d->borderType->addItem(i18n("Decorative Wood"));
    d->borderType->addItem(i18n("Decorative Paper"));
    d->borderType->addItem(i18n("Decorative Parquet"));
    d->borderType->addItem(i18n("Decorative Ice"));
    d->borderType->addItem(i18n("Decorative Leaf"));
    d->borderType->addItem(i18n("Decorative Marble"));
    d->borderType->addItem(i18n("Decorative Rain"));
    d->borderType->addItem(i18n("Decorative Craters"));
    d->borderType->addItem(i18n("Decorative Dried"));
    d->borderType->addItem(i18n("Decorative Pink"));
    d->borderType->addItem(i18n("Decorative Stone"));
    d->borderType->addItem(i18n("Decorative Chalk"));
    d->borderType->addItem(i18n("Decorative Granite"));
    d->borderType->addItem(i18n("Decorative Rock"));
    d->borderType->addItem(i18n("Decorative Wall"));
    d->borderType->setDefaultIndex(BorderFilter::SolidBorder);
    d->borderType->setWhatsThis( i18n("Select the border type to add around the image here."));

    KSeparator *line1 = new KSeparator(Qt::Horizontal);

    // -------------------------------------------------------------------

    d->preserveAspectRatio = new QCheckBox();
    d->preserveAspectRatio->setText(i18n("Preserve Aspect Ratio"));
    d->preserveAspectRatio->setWhatsThis(i18n("Enable this option if you want to preserve the aspect "
                                              "ratio of image. If enabled, the border width will be "
                                              "a percentage of the image size, else the border width will be "
                                              "in pixels."));

    d->labelBorderPercent  = new QLabel(i18n("Width (%):"));
    d->borderPercent       = new RIntNumInput();
    d->borderPercent->setRange(1, 50, 1);
    d->borderPercent->setSliderEnabled(true);
    d->borderPercent->setDefaultValue(10);
    d->borderPercent->setWhatsThis( i18n("Set here the border width as a percentage of the image size."));

    d->labelBorderWidth = new QLabel(i18n("Width (pixels):"));
    d->borderWidth      = new RIntNumInput();
    d->borderWidth->setRange(1, 1000, 1);
    d->borderWidth->setSliderEnabled(true);
    d->borderWidth->setDefaultValue(100);
    d->borderWidth->setWhatsThis(i18n("Set here the border width in pixels to add around the image."));

    ImageIface iface(0, 0);
    int w = iface.originalWidth();
    int h = iface.originalHeight();

    if (w > h)
        d->borderWidth->setRange(1, h/2, 1);
    else
        d->borderWidth->setRange(1, w/2, 1);

    KSeparator* line2 = new KSeparator(Qt::Horizontal);

    // -------------------------------------------------------------------

    d->labelForeground   = new QLabel();
    d->firstColorButton  = new KColorButton( QColor( 192, 192, 192 ) );
    d->labelBackground   = new QLabel();
    d->secondColorButton = new KColorButton( QColor( 128, 128, 128 ) );

    // -------------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->addWidget(label1,                  0, 0, 1, 3);
    mainLayout->addWidget(d->borderType,           1, 0, 1, 3);
    mainLayout->addWidget(line1,                   2, 0, 1, 3);
    mainLayout->addWidget(d->preserveAspectRatio,  3, 0, 1, 3);
    mainLayout->addWidget(d->labelBorderPercent,   4, 0, 1, 3);
    mainLayout->addWidget(d->borderPercent,        5, 0, 1, 3);
    mainLayout->addWidget(d->labelBorderWidth,     6, 0, 1, 3);
    mainLayout->addWidget(d->borderWidth,          7, 0, 1, 3);
    mainLayout->addWidget(line2,                   8, 0, 1, 3);
    mainLayout->addWidget(d->labelForeground,      9, 0, 1, 1);
    mainLayout->addWidget(d->firstColorButton,     9, 1, 1, 2);
    mainLayout->addWidget(d->labelBackground,     10, 0, 1, 1);
    mainLayout->addWidget(d->secondColorButton,   10, 1, 1, 2);
    mainLayout->setRowStretch(11, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->preserveAspectRatio, SIGNAL(toggled(bool)),
            this, SLOT(slotPreserveAspectRatioToggled(bool)));

    connect(d->borderType, SIGNAL(activated(int)),
            this, SLOT(slotBorderTypeChanged(int)));

    connect(d->borderPercent, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->borderWidth, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->firstColorButton, SIGNAL(changed(const QColor&)),
            this, SLOT(slotColorForegroundChanged(const QColor&)));

    connect(d->secondColorButton, SIGNAL(changed(const QColor&)),
            this, SLOT(slotColorBackgroundChanged(const QColor&)));
}

BorderTool::~BorderTool()
{
    delete d;
}

void BorderTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    blockWidgetSignals(true);

    d->borderType->setCurrentIndex(group.readEntry(d->configBorderTypeEntry, d->borderType->defaultIndex()));
    d->borderPercent->setValue(group.readEntry(d->configBorderPercentEntry,  d->borderPercent->defaultValue()));
    d->borderWidth->setValue(group.readEntry(d->configBorderWidthEntry,      d->borderWidth->defaultValue()));
    d->preserveAspectRatio->setChecked(group.readEntry(d->configPreserveAspectRatioEntry, true));

    QColor black(0, 0, 0);
    QColor white(255, 255, 255);
    QColor gray1(192, 192, 192);
    QColor gray2(128, 128, 128);

    d->solidColor            = group.readEntry(d->configSolidColorEntry,            black);
    d->niepceBorderColor     = group.readEntry(d->configNiepceBorderColorEntry,     white);
    d->niepceLineColor       = group.readEntry(d->configNiepceLineColorEntry,       black);
    d->bevelUpperLeftColor   = group.readEntry(  d->configBevelUpperLeftColorEntry, gray1);
    d->bevelLowerRightColor  = group.readEntry( d->configBevelLowerRightColorEntry, gray2);
    d->decorativeFirstColor  = group.readEntry( d->configDecorativeFirstColorEntry, black);
    d->decorativeSecondColor = group.readEntry(d->configDecorativeSecondColorEntry, black);

    blockWidgetSignals(false);

    slotBorderTypeChanged(d->borderType->currentIndex());
}

void BorderTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configBorderTypeEntry,            d->borderType->currentIndex());
    group.writeEntry(d->configBorderPercentEntry,         d->borderPercent->value());
    group.writeEntry(d->configBorderWidthEntry,           d->borderWidth->value());
    group.writeEntry(d->configPreserveAspectRatioEntry,   d->preserveAspectRatio->isChecked());
    group.writeEntry(d->configSolidColorEntry,            d->solidColor);
    group.writeEntry(d->configNiepceBorderColorEntry,     d->niepceBorderColor);
    group.writeEntry(d->configNiepceLineColorEntry,       d->niepceLineColor);
    group.writeEntry(d->configBevelUpperLeftColorEntry,   d->bevelUpperLeftColor);
    group.writeEntry(d->configBevelLowerRightColorEntry,  d->bevelLowerRightColor);
    group.writeEntry(d->configDecorativeFirstColorEntry,  d->decorativeFirstColor);
    group.writeEntry(d->configDecorativeSecondColorEntry, d->decorativeSecondColor);

    group.sync();
}

void BorderTool::slotResetSettings()
{
    blockWidgetSignals(true);

    d->borderType->slotReset();
    d->borderPercent->slotReset();
    d->borderWidth->slotReset();
    d->preserveAspectRatio->setChecked(true);

    d->solidColor            = QColor(0, 0, 0);
    d->niepceBorderColor     = QColor(255, 255, 255);
    d->niepceLineColor       = QColor(0, 0, 0);
    d->bevelUpperLeftColor   = QColor(192, 192, 192);
    d->bevelLowerRightColor  = QColor(128, 128, 128);
    d->decorativeFirstColor  = QColor(0, 0, 0);
    d->decorativeSecondColor = QColor(0, 0, 0);

    blockWidgetSignals(false);

    slotBorderTypeChanged(BorderFilter::SolidBorder);
}

void BorderTool::renderingFinished()
{
    d->preserveAspectRatio->setEnabled(true);
    d->borderType->setEnabled(true);
    d->borderPercent->setEnabled(true);
    d->borderWidth->setEnabled(true);
    d->firstColorButton->setEnabled(true);
    d->secondColorButton->setEnabled(true);
    toggleBorderSlider(d->preserveAspectRatio->isChecked());
}

void BorderTool::slotColorForegroundChanged(const QColor& color)
{
    switch (d->borderType->currentIndex())
    {
        case BorderFilter::SolidBorder:
            d->solidColor = color;
            break;

        case BorderFilter::NiepceBorder:
            d->niepceBorderColor = color;
            break;

        case BorderFilter::BeveledBorder:
            d->bevelUpperLeftColor = color;
            break;

        case BorderFilter::PineBorder:
        case BorderFilter::WoodBorder:
        case BorderFilter::PaperBorder:
        case BorderFilter::ParqueBorder:
        case BorderFilter::IceBorder:
        case BorderFilter::LeafBorder:
        case BorderFilter::MarbleBorder:
        case BorderFilter::RainBorder:
        case BorderFilter::CratersBorder:
        case BorderFilter::DriedBorder:
        case BorderFilter::PinkBorder:
        case BorderFilter::StoneBorder:
        case BorderFilter::ChalkBorder:
        case BorderFilter::GraniteBorder:
        case BorderFilter::RockBorder:
        case BorderFilter::WallBorder:
            d->decorativeFirstColor = color;
            break;
    }

    slotEffect();
}

void BorderTool::slotColorBackgroundChanged(const QColor& color)
{
    switch (d->borderType->currentIndex())
    {
        case BorderFilter::SolidBorder:
            d->solidColor = color;
            break;

        case BorderFilter::NiepceBorder:
            d->niepceLineColor = color;
            break;

        case BorderFilter::BeveledBorder:
            d->bevelLowerRightColor = color;
            break;

        case BorderFilter::PineBorder:
        case BorderFilter::WoodBorder:
        case BorderFilter::PaperBorder:
        case BorderFilter::ParqueBorder:
        case BorderFilter::IceBorder:
        case BorderFilter::LeafBorder:
        case BorderFilter::MarbleBorder:
        case BorderFilter::RainBorder:
        case BorderFilter::CratersBorder:
        case BorderFilter::DriedBorder:
        case BorderFilter::PinkBorder:
        case BorderFilter::StoneBorder:
        case BorderFilter::ChalkBorder:
        case BorderFilter::GraniteBorder:
        case BorderFilter::RockBorder:
        case BorderFilter::WallBorder:
            d->decorativeSecondColor = color;
            break;
    }

    slotEffect();
}

void BorderTool::slotBorderTypeChanged(int borderType)
{
    d->labelForeground->setText(i18nc("first color for border effect", "First:"));
    d->labelBackground->setText(i18nc("second color for border effect", "Second:"));
    d->firstColorButton->setWhatsThis(i18n("Set here the foreground color of the border."));
    d->secondColorButton->setWhatsThis(i18n("Set here the Background color of the border."));
    d->firstColorButton->setEnabled(true);
    d->secondColorButton->setEnabled(true);
    d->labelForeground->setEnabled(true);
    d->labelBackground->setEnabled(true);
    d->borderPercent->setEnabled(true);

    switch (borderType)
    {
        case BorderFilter::SolidBorder:
            d->firstColorButton->setColor(d->solidColor);
            d->secondColorButton->setEnabled(false);
            d->labelBackground->setEnabled(false);
            break;

        case BorderFilter::NiepceBorder:
            d->firstColorButton->setWhatsThis(i18n("Set here the color of the main border."));
            d->secondColorButton->setWhatsThis(i18n("Set here the color of the line."));
            d->firstColorButton->setColor(d->niepceBorderColor);
            d->secondColorButton->setColor(d->niepceLineColor);
            break;

        case BorderFilter::BeveledBorder:
            d->firstColorButton->setWhatsThis(i18n("Set here the color of the upper left area."));
            d->secondColorButton->setWhatsThis(i18n("Set here the color of the lower right area."));
            d->firstColorButton->setColor(d->bevelUpperLeftColor);
            d->secondColorButton->setColor(d->bevelLowerRightColor);
            break;

        case BorderFilter::PineBorder:
        case BorderFilter::WoodBorder:
        case BorderFilter::PaperBorder:
        case BorderFilter::ParqueBorder:
        case BorderFilter::IceBorder:
        case BorderFilter::LeafBorder:
        case BorderFilter::MarbleBorder:
        case BorderFilter::RainBorder:
        case BorderFilter::CratersBorder:
        case BorderFilter::DriedBorder:
        case BorderFilter::PinkBorder:
        case BorderFilter::StoneBorder:
        case BorderFilter::ChalkBorder:
        case BorderFilter::GraniteBorder:
        case BorderFilter::RockBorder:
        case BorderFilter::WallBorder:
            d->firstColorButton->setWhatsThis(i18n("Set here the color of the first line."));
            d->secondColorButton->setWhatsThis(i18n("Set here the color of the second line."));
            d->firstColorButton->setColor(d->decorativeFirstColor);
            d->secondColorButton->setColor(d->decorativeSecondColor);
            break;
    }

    slotEffect();
}

void BorderTool::prepareEffect()
{
    d->borderType->setEnabled(false);
    d->borderPercent->setEnabled(false);
    d->borderWidth->setEnabled(false);
    d->firstColorButton->setEnabled(false);
    d->secondColorButton->setEnabled(false);
    d->preserveAspectRatio->setEnabled(false);

    ImageIface* iface = d->previewWidget->imageIface();
    int orgWidth      = iface->originalWidth();
    int orgHeight     = iface->originalHeight();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();
    bool sixteenBit   = iface->previewSixteenBit();
    uchar *data       = iface->getPreviewImage();
    DImg previewImage(w, h, sixteenBit,
                      iface->previewHasAlpha(), data);
    delete [] data;

    int borderType  = d->borderType->currentIndex();
    float ratio     = (float)w/(float)orgWidth;
    int borderWidth = (int)((float)d->borderWidth->value()*ratio);
    QString border  = getBorderPath( d->borderType->currentIndex() );

    if (d->preserveAspectRatio->isChecked())
    {
        setFilter(new BorderFilter(&previewImage, this, orgWidth, orgHeight,
                                   border, borderType, d->borderPercent->value()/100.0,
                                   DColor(d->solidColor, sixteenBit),
                                   DColor(d->niepceBorderColor, sixteenBit),
                                   DColor(d->niepceLineColor, sixteenBit),
                                   DColor(d->bevelUpperLeftColor, sixteenBit),
                                   DColor(d->bevelLowerRightColor, sixteenBit),
                                   DColor(d->decorativeFirstColor, sixteenBit),
                                   DColor(d->decorativeSecondColor, sixteenBit)));
    }
    else
    {
        setFilter(new BorderFilter(&previewImage, this, orgWidth, orgHeight,
                                   border, borderType, borderWidth,
                                   (int)(20.0*ratio), (int)(20.0*ratio), 3,
                                   DColor(d->solidColor, sixteenBit),
                                   DColor(d->niepceBorderColor, sixteenBit),
                                   DColor(d->niepceLineColor, sixteenBit),
                                   DColor(d->bevelUpperLeftColor, sixteenBit),
                                   DColor(d->bevelLowerRightColor, sixteenBit),
                                   DColor(d->decorativeFirstColor, sixteenBit),
                                   DColor(d->decorativeSecondColor, sixteenBit)));
    }
}

void BorderTool::prepareFinal()
{
    d->borderType->setEnabled(false);
    d->borderPercent->setEnabled(false);
    d->borderWidth->setEnabled(false);
    d->firstColorButton->setEnabled(false);
    d->secondColorButton->setEnabled(false);

    int borderType    = d->borderType->currentIndex();
    int borderWidth   = d->borderWidth->value();
    float borderRatio = d->borderPercent->value()/100.0f;
    QString border    = getBorderPath( d->borderType->currentIndex() );

    ImageIface iface(0, 0);
    int orgWidth    = iface.originalWidth();
    int orgHeight   = iface.originalHeight();
    bool sixteenBit = iface.previewSixteenBit();
    uchar *data     = iface.getOriginalImage();
    DImg orgImage(orgWidth, orgHeight, sixteenBit,
                           iface.originalHasAlpha(), data);
    delete [] data;

    if (d->preserveAspectRatio->isChecked())
    {
        setFilter(new BorderFilter(&orgImage, this, orgWidth, orgHeight,
                                   border, borderType, borderRatio,
                                   DColor(d->solidColor, sixteenBit),
                                   DColor(d->niepceBorderColor, sixteenBit),
                                   DColor(d->niepceLineColor, sixteenBit),
                                   DColor(d->bevelUpperLeftColor, sixteenBit),
                                   DColor(d->bevelLowerRightColor, sixteenBit),
                                   DColor(d->decorativeFirstColor, sixteenBit),
                                   DColor(d->decorativeSecondColor, sixteenBit)));
    }
    else
    {
        setFilter(new BorderFilter(&orgImage, this, orgWidth, orgHeight,
                                   border, borderType, borderWidth, 15, 15, 10,
                                   DColor(d->solidColor, sixteenBit),
                                   DColor(d->niepceBorderColor, sixteenBit),
                                   DColor(d->niepceLineColor, sixteenBit),
                                   DColor(d->bevelUpperLeftColor, sixteenBit),
                                   DColor(d->bevelLowerRightColor, sixteenBit),
                                   DColor(d->decorativeFirstColor, sixteenBit),
                                   DColor(d->decorativeSecondColor, sixteenBit)));
    }
}

void BorderTool::putPreviewData()
{
    ImageIface* iface = d->previewWidget->imageIface();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();

    DImg imTemp = filter()->getTargetImage().smoothScale(w, h, Qt::KeepAspectRatio);
    DImg imDest(w, h, filter()->getTargetImage().sixteenBit(),
                      filter()->getTargetImage().hasAlpha());

    imDest.fill(DColor(d->previewWidget->palette().color(QPalette::Background).rgb(),
                filter()->getTargetImage().sixteenBit()) );

    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->putPreviewImage(imDest.bits());
    d->previewWidget->updatePreview();
}

void BorderTool::putFinalData()
{
    ImageIface iface(0, 0);
    DImg targetImage = filter()->getTargetImage();
    iface.putOriginalImage(i18n("Add Border"), targetImage.bits(), targetImage.width(), targetImage.height()); 
}

QString BorderTool::getBorderPath(int border)
{
    QString pattern;

    switch (border)
       {
       case BorderFilter::PineBorder:
          pattern = "pine-pattern";
          break;

       case BorderFilter::WoodBorder:
          pattern = "wood-pattern";
          break;

       case BorderFilter::PaperBorder:
          pattern = "paper-pattern";
          break;

       case BorderFilter::ParqueBorder:
          pattern = "parque-pattern";
          break;

       case BorderFilter::IceBorder:
          pattern = "ice-pattern";
          break;

       case BorderFilter::LeafBorder:
          pattern = "leaf-pattern";
          break;

       case BorderFilter::MarbleBorder:
          pattern = "marble-pattern";
          break;

       case BorderFilter::RainBorder:
          pattern = "rain-pattern";
          break;

       case BorderFilter::CratersBorder:
          pattern = "craters-pattern";
          break;

       case BorderFilter::DriedBorder:
          pattern = "dried-pattern";
          break;

       case BorderFilter::PinkBorder:
          pattern = "pink-pattern";
          break;

       case BorderFilter::StoneBorder:
          pattern = "stone-pattern";
          break;

       case BorderFilter::ChalkBorder:
          pattern = "chalk-pattern";
          break;

       case BorderFilter::GraniteBorder:
          pattern = "granit-pattern";
          break;

       case BorderFilter::RockBorder:
          pattern = "rock-pattern";
          break;

       case BorderFilter::WallBorder:
          pattern = "wall-pattern";
          break;

       default:
          return QString();
       }

    return (KStandardDirs::locate("data", QString("digikam/data/") + pattern + QString(".png")));
}

void BorderTool::slotPreserveAspectRatioToggled(bool b)
{
    toggleBorderSlider(b);
    slotTimer();
}

void BorderTool::toggleBorderSlider(bool b)
{
    d->borderPercent->setEnabled(b);
    d->borderWidth->setEnabled(!b);
    d->labelBorderPercent->setEnabled(b);
    d->labelBorderWidth->setEnabled(!b);
}

void BorderTool::blockWidgetSignals(bool b)
{
    d->borderType->blockSignals(b);
    d->borderPercent->blockSignals(b);
    d->borderWidth->blockSignals(b);
    d->firstColorButton->blockSignals(b);
    d->secondColorButton->blockSignals(b);
    d->preserveAspectRatio->blockSignals(b);
}

}  // namespace DigikamBorderImagesPlugin
