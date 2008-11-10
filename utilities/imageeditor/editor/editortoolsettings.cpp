/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-21
 * Description : Editor tool settings template box
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "editortoolsettings.h"
#include "editortoolsettings.moc"

// Qt includes.

#include <QButtonGroup>
#include <QLabel>
#include <QLayout>
#include <QMap>
#include <QPair>
#include <QString>
#include <QToolButton>
#include <QVariant>

// KDE includes.

#include <kapplication.h>
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <khbox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kstandardguiitem.h>
#include <kvbox.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "colorgradientwidget.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imagepaniconwidget.h"

using namespace KDcrawIface;

namespace Digikam
{

class EditorToolSettingsPriv
{

public:

    EditorToolSettingsPriv()
    {
        okBtn           = 0;
        cancelBtn       = 0;
        tryBtn          = 0;
        defaultBtn      = 0;
        plainPage       = 0;
        btnBox1         = 0;
        btnBox2         = 0;
        saveAsBtn       = 0;
        loadBtn         = 0;
        guideBox        = 0;
        guideColorBt    = 0;
        guideSize       = 0;
        panIconView     = 0;
        channelCB       = 0;
        colorsCB        = 0;
        scaleBG         = 0;
        histogramBox    = 0;
        hGradient       = 0;
    }

    QButtonGroup        *scaleBG;

    QToolButton         *linHistoButton;
    QToolButton         *logHistoButton;

    QWidget             *plainPage;

    KHBox               *btnBox1;
    KHBox               *btnBox2;
    KHBox               *guideBox;

    KComboBox           *channelCB;
    KComboBox           *colorsCB;

    KPushButton         *okBtn;
    KPushButton         *cancelBtn;
    KPushButton         *tryBtn;
    KPushButton         *defaultBtn;
    KPushButton         *saveAsBtn;
    KPushButton         *loadBtn;

    KColorButton        *guideColorBt;

    ColorGradientWidget *hGradient;

    ImagePanIconWidget  *panIconView;

    HistogramBox        *histogramBox;

    RIntNumInput        *guideSize;
};

EditorToolSettings::EditorToolSettings(int buttonMask, int toolMask, int histogramType, QWidget *parent)
                  : QScrollArea(parent)
{
    d = new EditorToolSettingsPriv;

    setFrameStyle( QFrame::NoFrame );
    setWidgetResizable(true);

    QWidget *settingsArea = new QWidget(viewport());
    setWidget(settingsArea);

    // ---------------------------------------------------------------

    QGridLayout* gridSettings = new QGridLayout(settingsArea);

    d->plainPage    = new QWidget(settingsArea);
    d->guideBox     = new KHBox(settingsArea);
    d->btnBox1      = new KHBox(settingsArea);
    d->btnBox2      = new KHBox(settingsArea);
    d->histogramBox = new HistogramBox(settingsArea, histogramType);

    // ---------------------------------------------------------------

    QFrame *frame     = new QFrame(settingsArea);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* vlay = new QVBoxLayout(frame);
    d->panIconView    = new ImagePanIconWidget(360, 240, frame);
    d->panIconView->setWhatsThis(i18n("<p>Here you can see the original image panel "
                                      "which can help you to select the clip preview.</p>"
                                      "<p>Click and drag the mouse cursor in the "
                                      "red rectangle to change the clip focus.</p>"));
    vlay->addWidget(d->panIconView, 0, Qt::AlignCenter);
    vlay->setSpacing(0);
    vlay->setMargin(5);

    if (!(toolMask & PanIcon))
        frame->hide();

    // ---------------------------------------------------------------

    new QLabel(i18n("Guide:"), d->guideBox);
    QLabel *space4  = new QLabel(d->guideBox);
    d->guideColorBt = new KColorButton(QColor(Qt::red), d->guideBox);
    d->guideColorBt->setWhatsThis(i18n("Set here the color used to draw guides dashed-lines."));
    d->guideSize    = new RIntNumInput(d->guideBox);
    d->guideSize->setRange(1, 5, 1);
    d->guideSize->setSliderEnabled(true);
    d->guideSize->setDefaultValue(1);
    d->guideSize->setWhatsThis(i18n("Set here the width in pixels used to draw guides dashed-lines."));

    d->guideBox->setStretchFactor(space4, 10);
    d->guideBox->setSpacing(spacingHint());
    d->guideBox->setMargin(0);

    if (!(toolMask & ColorGuide))
        d->guideBox->hide();

    // ---------------------------------------------------------------

    d->defaultBtn = new KPushButton(d->btnBox1);
    d->defaultBtn->setGuiItem(KStandardGuiItem::defaults());
    d->defaultBtn->setIcon(KIcon(SmallIcon("document-revert")));
    d->defaultBtn->setToolTip(i18n("Reset all settings to their default values."));
    if (!(buttonMask & Default))
        d->defaultBtn->hide();

    QLabel *space2 = new QLabel(d->btnBox1);

    d->okBtn = new KPushButton(d->btnBox1);
    d->okBtn->setGuiItem(KStandardGuiItem::ok());
    if (!(buttonMask & Ok))
        d->okBtn->hide();

    d->cancelBtn = new KPushButton(d->btnBox1);
    d->cancelBtn->setGuiItem(KStandardGuiItem::cancel());
    if (!(buttonMask & Cancel))
        d->cancelBtn->hide();

    d->btnBox1->setStretchFactor(space2, 10);
    d->btnBox1->setSpacing(spacingHint());
    d->btnBox1->setMargin(0);

    if (!(buttonMask & Default) && !(buttonMask & Ok) && !(buttonMask & Cancel))
        d->btnBox1->hide();

    // ---------------------------------------------------------------

    d->loadBtn = new KPushButton(d->btnBox2);
    d->loadBtn->setGuiItem(KStandardGuiItem::open());
    d->loadBtn->setText(i18n("Load..."));
    d->loadBtn->setToolTip(i18n("Load all parameters from settings text file."));
    if (!(buttonMask & Load))
        d->loadBtn->hide();

    d->saveAsBtn = new KPushButton(d->btnBox2);
    d->saveAsBtn->setGuiItem(KStandardGuiItem::saveAs());
    d->saveAsBtn->setToolTip(i18n("Save all parameters to settings text file."));
    if (!(buttonMask & SaveAs))
        d->saveAsBtn->hide();

    QLabel *space3 = new QLabel(d->btnBox2);

    d->tryBtn = new KPushButton(d->btnBox2);
    d->tryBtn->setGuiItem(KStandardGuiItem::apply());
    d->tryBtn->setText(i18n("Try"));
    d->tryBtn->setToolTip(i18n("Try all settings."));
    if (!(buttonMask & Try))
        d->tryBtn->hide();

    d->btnBox2->setStretchFactor(space3, 10);
    d->btnBox2->setSpacing(spacingHint());
    d->btnBox2->setMargin(0);

    if (!(buttonMask & Load) && !(buttonMask & SaveAs) && !(buttonMask & Try))
        d->btnBox2->hide();

    // ---------------------------------------------------------------

    if (!(toolMask & Histogram))
        d->histogramBox->hide();

    // ---------------------------------------------------------------

    gridSettings->addWidget(d->histogramBox,  0, 0, 2, 2);
    gridSettings->addWidget(frame,            2, 0, 1, 2);
    gridSettings->addWidget(d->plainPage,     3, 0, 1, 2);
    gridSettings->addWidget(d->guideBox,      4, 0, 1, 2);
    gridSettings->addWidget(d->btnBox2,       5, 0, 1, 2);
    gridSettings->addWidget(d->btnBox1,       6, 0, 1, 2);
    gridSettings->setSpacing(spacingHint());
    gridSettings->setMargin(0);

    // ---------------------------------------------------------------

    connect(d->okBtn, SIGNAL(clicked()),
            this, SIGNAL(signalOkClicked()));

    connect(d->cancelBtn, SIGNAL(clicked()),
            this, SIGNAL(signalCancelClicked()));

    connect(d->tryBtn, SIGNAL(clicked()),
            this, SIGNAL(signalTryClicked()));

    connect(d->defaultBtn, SIGNAL(clicked()),
            this, SIGNAL(signalDefaultClicked()));

    connect(d->saveAsBtn, SIGNAL(clicked()),
            this, SIGNAL(signalSaveAsClicked()));

    connect(d->loadBtn, SIGNAL(clicked()),
            this, SIGNAL(signalLoadClicked()));

    connect(d->guideColorBt, SIGNAL(changed(const QColor&)),
            this, SIGNAL(signalColorGuideChanged()));

    connect(d->guideSize, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalColorGuideChanged()));

    connect(d->histogramBox, SIGNAL(signalChannelChanged()),
            this, SIGNAL(signalChannelChanged()));

    connect(d->histogramBox, SIGNAL(signalColorsChanged()),
            this, SIGNAL(signalColorsChanged()));

    connect(d->histogramBox, SIGNAL(signalScaleChanged()),
            this, SIGNAL(signalScaleChanged()));
}

EditorToolSettings::~EditorToolSettings()
{
    delete d;
}

int EditorToolSettings::marginHint()
{
    return KDialog::marginHint();
}

int EditorToolSettings::spacingHint()
{
    return KDialog::spacingHint();
}

QWidget *EditorToolSettings::plainPage() const
{
    return d->plainPage;
}

ImagePanIconWidget* EditorToolSettings::panIconView() const
{
    return d->panIconView;
}

HistogramBox* EditorToolSettings::histogramBox() const
{
    return d->histogramBox;
}

KPushButton* EditorToolSettings::button(int buttonCode) const
{
    if (buttonCode & Default)
        return d->defaultBtn;

    if (buttonCode & Try)
        return d->tryBtn;

    if (buttonCode & Ok)
        return d->okBtn;

    if (buttonCode & Cancel)
        return d->cancelBtn;

    if (buttonCode & Load)
        return d->loadBtn;

    if (buttonCode & SaveAs)
        return d->saveAsBtn;

    return 0;
}

void EditorToolSettings::enableButton(int buttonCode, bool state)
{
    KPushButton *btn = button(buttonCode);
    if (btn) btn->setEnabled(state);
}

QColor EditorToolSettings::guideColor() const
{
    return d->guideColorBt->color();
}

void EditorToolSettings::setGuideColor(const QColor& color)
{
    d->guideColorBt->setColor(color);
}

int EditorToolSettings::guideSize() const
{
    return d->guideSize->value();
}

void EditorToolSettings::setGuideSize(int size)
{
    d->guideSize->setValue(size);
}

} // namespace Digikam
