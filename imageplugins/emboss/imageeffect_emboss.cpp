/* ============================================================
 * File  : imageeffect_emboss.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a Digikam image editor plugin for to emboss 
 *               an image.
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

// Imlib2 include.

#define X_DISPLAY_MISSING 1
#include <Imlib2.h>

// C++ include.

#include <cstring>

// Qt includes.

#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qslider.h>
#include <qlayout.h>
#include <qframe.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kimageeffect.h>
#include <knuminput.h>
#include <kstandarddirs.h>

// Digikam includes.

#include <digikam/imageiface.h>
#include <digikam/imagepreviewwidget.h>

// Local includes.

#include "version.h"
#include "imageeffect_emboss.h"

namespace DigikamEmbossImagesPlugin
{

ImageEffect_Emboss::ImageEffect_Emboss(QWidget* parent)
                  : KDialogBase(Plain, i18n("Emboss Image"),
                                Help|Ok|Cancel, Ok,
                                parent, 0, true, true),
                    m_parent(parent)
{
    QString whatsThis;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Emboss Image"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("An embossed image effect plugin for Digikam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004, Gilles Caulier", 
                                       0,
                                       "http://digikam.sourceforge.net");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Emboss Image handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("Emboss Image"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addWidget(headerFrame);
    
    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left", "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    // -------------------------------------------------------------

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);
    
    m_imagePreviewWidget = new Digikam::ImagePreviewWidget(240, 160, 
                                                           i18n("Emboss image preview"),
                                                           plainPage());
    hlay1->addWidget(m_imagePreviewWidget);
    
    // -------------------------------------------------------------
    
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label1 = new QLabel(i18n("Radius:"), plainPage());
    
    m_radiusSlider = new QSlider(1, 100, 1, 10, Qt::Horizontal, plainPage(), "m_radiusSlider");
    m_radiusSlider->setTickmarks(QSlider::Below);
    m_radiusSlider->setTickInterval(10);
    m_radiusSlider->setTracking ( false );
    
    m_radiusInput = new KDoubleSpinBox(0.1, 10.0, 0.1, 1.0, 1, plainPage(), "m_radiusInput");
    whatsThis = i18n("<p>Set here the radius of the Gaussian, not counting the center pixel.");
        
    QWhatsThis::add( m_radiusInput, whatsThis);
    QWhatsThis::add( m_radiusSlider, whatsThis);

    hlay->addWidget(label1, 1);
    hlay->addWidget(m_radiusSlider, 3);
    hlay->addWidget(m_radiusInput, 1);
    
    // -------------------------------------------------------------
    
    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label2 = new QLabel(i18n("Sigma:"), plainPage());
    
    m_sigmaSlider = new QSlider(1, 100, 1, 10, Qt::Horizontal, plainPage(), "m_sigmaSlider");
    m_sigmaSlider->setTickmarks(QSlider::Below);
    m_sigmaSlider->setTickInterval(10);
    m_sigmaSlider->setTracking ( false );
    
    m_sigmaInput = new KDoubleSpinBox(0.1, 10.0, 0.1, 1.0, 1, plainPage(), "m_sigmaInput");
    whatsThis = i18n("<p>Set here the standard deviation of the Gaussian.");
    
    QWhatsThis::add( m_sigmaSlider, whatsThis);
    QWhatsThis::add( m_sigmaInput, whatsThis);
                 
    hlay2->addWidget(label2, 1);
    hlay2->addWidget(m_sigmaSlider, 3);
    hlay2->addWidget(m_sigmaInput, 1);
    
    // -------------------------------------------------------------
    
    adjustSize();
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_radiusSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderRadiusChanged(int)));
    connect(m_radiusInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotSpinBoxRadiusChanged(double)));            
    connect(m_radiusInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));   

    connect(m_sigmaSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderSigmaChanged(int)));
    connect(m_sigmaInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotSpinBoxSigmaChanged(double)));            
    connect(m_sigmaInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotEffect()));
}

ImageEffect_Emboss::~ImageEffect_Emboss()
{
}

void ImageEffect_Emboss::slotSliderRadiusChanged(int v)
{
    blockSignals(true);
    m_radiusInput->setValue((double)v/10.0);
    blockSignals(false);
}

void ImageEffect_Emboss::slotSpinBoxRadiusChanged(double v)
{
    blockSignals(true);
    m_radiusSlider->setValue((int)(v*10.0));
    blockSignals(false);
}

void ImageEffect_Emboss::slotSliderSigmaChanged(int v)
{
    blockSignals(true);
    m_sigmaInput->setValue((double)v/10.0);
    blockSignals(false);
}

void ImageEffect_Emboss::slotSpinBoxSigmaChanged(double v)
{
    blockSignals(true);
    m_sigmaSlider->setValue((int)(v*10.0));
    blockSignals(false);
}

void ImageEffect_Emboss::slotHelp()
{
    KApplication::kApplication()->invokeHelp("emboss",
                                             "digikamimageplugins");
}

void ImageEffect_Emboss::closeEvent(QCloseEvent *e)
{
    e->accept();    
}

void ImageEffect_Emboss::slotEffect()
{
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    double radius = m_radiusInput->value();
    double sigma  = m_sigmaInput->value();
    QImage image = m_imagePreviewWidget->getOriginalClipImage();
#if KDE_VERSION >= 0x30200
    QImage newImage = KImageEffect::emboss(image, radius, sigma);
#else
    QImage newImage = KImageEffect::emboss(image);
#endif
    m_imagePreviewWidget->setPreviewImageData(newImage);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
}

void ImageEffect_Emboss::slotOk()
{
    accept();
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
    
    uint* data    = iface.getOriginalData();
    int w         = iface.originalWidth();
    int h         = iface.originalHeight();
    double radius = m_radiusInput->value();
    double sigma  = m_sigmaInput->value();

    if (data) 
        {
        QImage image;
        image.create( w, h, 32 );
        image.setAlphaBuffer(true) ;
        memcpy(image.bits(), data, image.numBytes());

#if KDE_VERSION >= 0x30200
        QImage newImage = KImageEffect::emboss(image, radius, sigma);
#else
        QImage newImage = KImageEffect::emboss(image);
#endif
    
        memcpy(data, newImage.bits(), newImage.numBytes());
        iface.putOriginalData(data);
        delete [] data;
        }
    
    m_parent->setCursor( KCursor::arrowCursor() );        
}

}  // NameSpace DigikamEmbossImagesPlugin

#include "imageeffect_emboss.moc"
