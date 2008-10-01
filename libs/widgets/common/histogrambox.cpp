/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-30
 * Description : a widget to display an image histogram and its control widgets
 *
 * Copyright (C) 2008 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QPair>
#include <QString>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

// KDE includes.

#include <kcombobox.h>
#include <kicon.h>
#include <klocale.h>

// Local includes.

#include "colorgradientwidget.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "histogrambox.moc"

namespace Digikam
{

class HistogramBoxPriv
{
public:

    HistogramBoxPriv()
    {
        channelCB       = 0;
        colorsCB        = 0;
        scaleBG         = 0;
        histogramWidget = 0;
        hGradient       = 0;
    }

    QButtonGroup        *scaleBG;

    QToolButton         *linHistoButton;
    QToolButton         *logHistoButton;

    KComboBox           *channelCB;
    KComboBox           *colorsCB;

    ColorGradientWidget *hGradient;

    HistogramWidget     *histogramWidget;
};

HistogramBox::HistogramBox(QWidget *parent, int histogramType, bool selectMode)
            : QWidget(parent)
{
    d = new HistogramBoxPriv;

    QGridLayout* gridLayout = new QGridLayout(this);
    QLabel *channelLabel    = new QLabel(i18n("Channel:"), this);
    channelLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    d->channelCB = new KComboBox(this);

    // all possible channels for histogram widget are defined in this map
    QMap<int, QPair<QString, QString> > channelDescMap;

    // this string will contain the WhatsThis message for the channelCB
    QString channelCBDescr(i18n("<p>Select the histogram channel to display:</p>"));

    // those pairs hold the combobox text and WhatsThis description for each channel item
    typedef QPair<QString, QString> ChannelPair;
    ChannelPair luminosityPair (i18n("Luminosity"), i18n("<b>Luminosity</b>: display the image's luminosity values."));
    ChannelPair redPair        (i18n("Red"),        i18n("<b>Red</b>: display the red image-channel values."));
    ChannelPair greenPair      (i18n("Green"),      i18n("<b>Green</b>: display the green image-channel values."));
    ChannelPair bluePair       (i18n("Blue"),       i18n("<b>Blue</b>: display the blue image-channel values."));
    ChannelPair colorsPair     (i18n("Colors"),     i18n("<b>Colors</b>: Display all color channel values at the same time."));
    ChannelPair alphaPair      (i18n("Alpha"),      i18n("<b>Alpha</b>: display the alpha image-channel values. "
                                                         "This channel corresponds to the transparency value and "
                                                         "is supported by some image formats, such as PNG or TIF."));

    channelDescMap.insert(LuminosityChannel, luminosityPair);
    channelDescMap.insert(RedChannel, redPair);
    channelDescMap.insert(GreenChannel, greenPair);
    channelDescMap.insert(BlueChannel, bluePair);
    channelDescMap.insert(ColorChannels, colorsPair);
    channelDescMap.insert(AlphaChannel, alphaPair);

    switch (histogramType)
    {
        case RGB:
            d->channelCB->addItem(channelDescMap[RedChannel].first, QVariant(RedChannel));
            d->channelCB->addItem(channelDescMap[GreenChannel].first, QVariant(GreenChannel));
            d->channelCB->addItem(channelDescMap[BlueChannel].first, QVariant(BlueChannel));
            channelCBDescr.append("<p>");
            channelCBDescr.append(channelDescMap[RedChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[GreenChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[BlueChannel].second);
            channelCBDescr.append("</p>");
            break;

        case RGBA:
            d->channelCB->addItem(channelDescMap[RedChannel].first, QVariant(RedChannel));
            d->channelCB->addItem(channelDescMap[GreenChannel].first, QVariant(GreenChannel));
            d->channelCB->addItem(channelDescMap[BlueChannel].first, QVariant(BlueChannel));
            d->channelCB->addItem(channelDescMap[AlphaChannel].first, QVariant(AlphaChannel));
            channelCBDescr.append("<p>");
            channelCBDescr.append(channelDescMap[RedChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[GreenChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[BlueChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[AlphaChannel].second);
            channelCBDescr.append("</p>");
            break;

        case LRGB:
            d->channelCB->addItem(channelDescMap[LuminosityChannel].first, QVariant(LuminosityChannel));
            d->channelCB->addItem(channelDescMap[RedChannel].first, QVariant(RedChannel));
            d->channelCB->addItem(channelDescMap[GreenChannel].first, QVariant(GreenChannel));
            d->channelCB->addItem(channelDescMap[BlueChannel].first, QVariant(BlueChannel));
            channelCBDescr.append("<p>");
            channelCBDescr.append(channelDescMap[LuminosityChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[RedChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[GreenChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[BlueChannel].second);
            channelCBDescr.append("</p>");
            break;

        case LRGBA:
            d->channelCB->addItem(channelDescMap[LuminosityChannel].first, QVariant(LuminosityChannel));
            d->channelCB->addItem(channelDescMap[RedChannel].first, QVariant(RedChannel));
            d->channelCB->addItem(channelDescMap[GreenChannel].first, QVariant(GreenChannel));
            d->channelCB->addItem(channelDescMap[BlueChannel].first, QVariant(BlueChannel));
            d->channelCB->addItem(channelDescMap[AlphaChannel].first, QVariant(AlphaChannel));
            channelCBDescr.append("<p>");
            channelCBDescr.append(channelDescMap[LuminosityChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[RedChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[GreenChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[BlueChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[AlphaChannel].second);
            channelCBDescr.append("</p>");
            break;

        case LRGBC:
            d->channelCB->addItem(channelDescMap[LuminosityChannel].first, QVariant(LuminosityChannel));
            d->channelCB->addItem(channelDescMap[RedChannel].first, QVariant(RedChannel));
            d->channelCB->addItem(channelDescMap[GreenChannel].first, QVariant(GreenChannel));
            d->channelCB->addItem(channelDescMap[BlueChannel].first, QVariant(BlueChannel));
            d->channelCB->addItem(channelDescMap[ColorChannels].first, QVariant(ColorChannels));
            channelCBDescr.append("<p>");
            channelCBDescr.append(channelDescMap[LuminosityChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[RedChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[GreenChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[BlueChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[ColorChannels].second);
            channelCBDescr.append("</p>");
            break;

        case LRGBAC:
            d->channelCB->addItem(channelDescMap[LuminosityChannel].first, QVariant(LuminosityChannel));
            d->channelCB->addItem(channelDescMap[RedChannel].first, QVariant(RedChannel));
            d->channelCB->addItem(channelDescMap[GreenChannel].first, QVariant(GreenChannel));
            d->channelCB->addItem(channelDescMap[BlueChannel].first, QVariant(BlueChannel));
            d->channelCB->addItem(channelDescMap[AlphaChannel].first, QVariant(AlphaChannel));
            d->channelCB->addItem(channelDescMap[ColorChannels].first, QVariant(ColorChannels));
            channelCBDescr.append("<p>");
            channelCBDescr.append(channelDescMap[LuminosityChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[RedChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[GreenChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[BlueChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[AlphaChannel].second).append("<br/>");
            channelCBDescr.append(channelDescMap[ColorChannels].second);
            channelCBDescr.append("</p>");
            break;

        default:
            break;
    }

    d->channelCB->setWhatsThis(channelCBDescr);

    QWidget *scaleBox = new QWidget(this);
    QHBoxLayout *hlay = new QHBoxLayout(scaleBox);
    d->scaleBG        = new QButtonGroup(scaleBox);
    scaleBox->setWhatsThis(i18n("<p>Select the histogram scale.</p>"
                                "<p>If the image's maximal counts are small, you can use the <b>linear</b> scale.</p>"
                                "<p><b>Logarithmic</b> scale can be used when the maximal counts are big; "
                                "if it is used, all values (small and large) will be visible on the graph.</p>"));

    d->linHistoButton = new QToolButton(scaleBox);
    d->linHistoButton->setToolTip(i18n("Linear"));
    d->linHistoButton->setIcon(KIcon("view-object-histogram-linear"));
    d->linHistoButton->setCheckable(true);
    d->scaleBG->addButton(d->linHistoButton, HistogramWidget::LinScaleHistogram);

    d->logHistoButton = new QToolButton(scaleBox);
    d->logHistoButton->setToolTip(i18n("Logarithmic"));
    d->logHistoButton->setIcon(KIcon("view-object-histogram-logarithmic"));
    d->logHistoButton->setCheckable(true);
    d->scaleBG->addButton(d->logHistoButton, HistogramWidget::LogScaleHistogram);

    hlay->setMargin(0);
    hlay->setSpacing(0);
    hlay->addWidget(d->linHistoButton);
    hlay->addWidget(d->logHistoButton);

    d->scaleBG->setExclusive(true);
    d->logHistoButton->setChecked(true);

    QLabel *colorsLabel = new QLabel(i18n("Colors:"), this);
    colorsLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    d->colorsCB = new KComboBox(this);
    d->colorsCB->addItem(i18n("Red"));
    d->colorsCB->addItem(i18n("Green"));
    d->colorsCB->addItem(i18n("Blue"));
    d->colorsCB->setEnabled(false);
    d->colorsCB->setWhatsThis(i18n("<p>Select the main color displayed with Colors Channel mode here:</p>"
                                   "<p><b>Red</b>: Draw the red image channel in the foreground.<br/>"
                                   "<b>Green</b>: Draw the green image channel in the foreground.<br/>"
                                   "<b>Blue</b>: Draw the blue image channel in the foreground.</p>"));
    colorsLabel->hide();
    d->colorsCB->hide();

    QWidget *histoBox = new QWidget;
    QVBoxLayout *histoBoxLayout = new QVBoxLayout;

    d->histogramWidget = new HistogramWidget(256, 140, histoBox, selectMode, true, true);
    d->histogramWidget->setWhatsThis(i18n("Here you can see the target preview image histogram drawing "
                                          "of the selected image channel. This one is re-computed at any "
                                          "settings changes."));

    d->hGradient = new ColorGradientWidget(ColorGradientWidget::Horizontal, 10, histoBox);
    d->hGradient->setColors(QColor("black"), QColor("white"));

    histoBoxLayout->addWidget(d->histogramWidget);
    histoBoxLayout->addWidget(d->hGradient);
    histoBoxLayout->setSpacing(1);
    histoBoxLayout->setMargin(0);
    histoBox->setLayout(histoBoxLayout);

    gridLayout->addWidget(channelLabel,     0, 0, 1, 1);
    gridLayout->addWidget(d->channelCB,     0, 1, 1, 1);
    gridLayout->addWidget(scaleBox,         0, 3, 1, 2);
    gridLayout->addWidget(colorsLabel,      1, 0, 1, 1);
    gridLayout->addWidget(d->colorsCB,      1, 1, 1, 1);
    gridLayout->addWidget(histoBox,         2, 0, 1, 5);
    gridLayout->setColumnStretch(2, 10);
    gridLayout->setSpacing(5);
    gridLayout->setMargin(0);

    switch (histogramType)
    {
        case LRGBC:
        case LRGBAC:
            colorsLabel->show();
            d->colorsCB->show();
            break;
        default:
            break;
    }

    // ---------------------------------------------------------------

    connect(d->channelCB, SIGNAL(activated(int)),
            this, SIGNAL(signalChannelChanged()));

    connect(d->colorsCB, SIGNAL(activated(int)),
            this, SIGNAL(signalColorsChanged()));

    connect(d->scaleBG, SIGNAL(buttonReleased(int)),
            this, SIGNAL(signalScaleChanged()));

    connect(d->channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged()));

    connect(d->colorsCB, SIGNAL(activated(int)),
            this, SLOT(slotColorsChanged()));

    connect(d->scaleBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotScaleChanged()));
}

HistogramBox::~HistogramBox()
{
    histogram()->stopHistogramComputation();
    delete d;
}

void HistogramBox::setGradientVisible(bool visible)
{
    d->hGradient->setVisible(visible);
}

void HistogramBox::setGradientColors(const QColor& from, const QColor& to)
{
    d->hGradient->setColors(from, to);
}

int HistogramBox::channel() const
{
    int index = d->channelCB->currentIndex();
    return d->channelCB->itemData(index).toInt();
}

void HistogramBox::setChannel(int channel)
{
    int id = d->channelCB->findData(QVariant(channel));
    d->channelCB->setCurrentIndex(id);
    slotChannelChanged();
    emit signalChannelChanged();
}

void HistogramBox::setChannelEnabled(bool enabled)
{
    d->channelCB->setEnabled(enabled);
}

void HistogramBox::setColorsEnabled(bool enabled)
{
    d->colorsCB->setEnabled(enabled);
}

int HistogramBox::colorsChannel() const
{
    return d->colorsCB->currentIndex();
}

void HistogramBox::setColorsChannel(int color)
{
    d->colorsCB->setCurrentIndex(color);
    slotColorsChanged();
    emit signalColorsChanged();
}

int HistogramBox::scale() const
{
    return d->scaleBG->checkedId();
}

void HistogramBox::setScale(int scale)
{
    d->scaleBG->button(scale)->setChecked(true);
    slotScaleChanged();
    emit signalScaleChanged();
}

HistogramWidget* HistogramBox::histogram() const
{
    return d->histogramWidget;
}

void HistogramBox::slotChannelChanged()
{
    switch (channel())
    {
        case LuminosityChannel:
            d->histogramWidget->m_channelType = HistogramWidget::ValueHistogram;
            setGradientColors(QColor("black"), QColor("white"));
            setColorsEnabled(false);
            break;

        case RedChannel:
            d->histogramWidget->m_channelType = HistogramWidget::RedChannelHistogram;
            setGradientColors(QColor("black"), QColor("red"));
            setColorsEnabled(false);
            break;

        case GreenChannel:
            d->histogramWidget->m_channelType = HistogramWidget::GreenChannelHistogram;
            setGradientColors(QColor("black"), QColor("green"));
            setColorsEnabled(false);
            break;

        case BlueChannel:
            d->histogramWidget->m_channelType = HistogramWidget::BlueChannelHistogram;
            setGradientColors(QColor("black"), QColor("blue"));
            setColorsEnabled(false);
            break;

        case AlphaChannel:
            d->histogramWidget->m_channelType = HistogramWidget::AlphaChannelHistogram;
            setGradientColors(QColor("black"), QColor("white"));
            setColorsEnabled(false);
            break;

        case ColorChannels:
            d->histogramWidget->m_channelType = HistogramWidget::ColorChannelsHistogram;
            setGradientColors(QColor("black"), QColor("white"));
            setColorsEnabled(true);
            break;
    }

    d->histogramWidget->repaint();
}

void HistogramBox::slotScaleChanged()
{
    d->histogramWidget->m_scaleType = scale();
    d->histogramWidget->repaint();
}

void HistogramBox::slotColorsChanged()
{
    switch (colorsChannel())
    {
        case HistogramWidget::GreenColor:
            d->histogramWidget->m_colorType = HistogramWidget::GreenColor;
            break;

        case HistogramWidget::BlueColor:
            d->histogramWidget->m_colorType = HistogramWidget::BlueColor;
            break;

        default: // Red.
            d->histogramWidget->m_colorType = HistogramWidget::RedColor;
            break;
    }

    d->histogramWidget->repaint();
}

} // namspace Digikam
