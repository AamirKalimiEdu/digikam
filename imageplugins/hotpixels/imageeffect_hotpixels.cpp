/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : a digiKam image plugin for fixing dots produced by
 *               hot/stuck/dead pixels from a CCD.
 * 
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
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

#include <Q3ValueList>
#include <Q3PointArray>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kimageio.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kglobal.h>

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "blackframelistview.h"
#include "imageeffect_hotpixels.h"
#include "imageeffect_hotpixels.moc"

namespace DigikamHotPixelsImagesPlugin
{

ImageEffect_HotPixels::ImageEffect_HotPixels(QWidget* parent)
                     : CtrlPanelDlg(parent, i18n("Hot Pixels Correction"), 
                                    "hotpixels", false, false, false, 
                                    Digikam::ImagePannelWidget::SeparateViewDuplicate)
{
    // No need Abort button action.
    showButton(User1, false); 
    
    QString whatsThis;

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Hot Pixels Correction"), 
                                       digikam_version,
                                       ki18n("A digiKam image plugin for fixing dots produced by "
                                             "hot/stuck/dead pixels from a CCD."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2005-2007, Unai Garro and Gilles Caulier"), 
                                       KLocalizedString(),
                                       "http://www.digikam.org");
                
    about->addAuthor(ki18n("Unai Garro"), ki18n("Author and maintainer"),
                     "ugarro at sourceforge dot net");
    
    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Developer"),
                     "caulier dot gilles at gmail dot com");
        
    setAboutData(about);
    
    // -------------------------------------------------------------
    
    QWidget *gboxSettings     = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings );
    
    QLabel *filterMethodLabel = new QLabel(i18n("Filter:"), gboxSettings);
    m_filterMethodCombo       = new QComboBox(gboxSettings);
    m_filterMethodCombo->addItem(i18n("Average"));
    m_filterMethodCombo->addItem(i18n("Linear"));
    m_filterMethodCombo->addItem(i18n("Quadratic"));
    m_filterMethodCombo->addItem(i18n("Cubic"));

    m_blackFrameButton = new QPushButton(i18n("Black Frame..."), gboxSettings);    
    setButtonWhatsThis( Apply, i18n("<p>Use this button to add a new black frame file which will "
                                    "be used by the hot pixels removal filter.") );  

    m_blackFrameListView = new BlackFrameListView(gboxSettings);

    // -------------------------------------------------------------

    gridSettings->addMultiCellWidget(filterMethodLabel, 0, 0, 0, 0);
    gridSettings->addMultiCellWidget(m_filterMethodCombo, 0, 0, 1, 1);
    gridSettings->addMultiCellWidget(m_blackFrameButton, 0, 0, 2, 2);    
    gridSettings->addMultiCellWidget(m_blackFrameListView, 1, 2, 0, 2);
    gridSettings->setMargin(0);
    gridSettings->setSpacing(spacingHint());
    
    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------
    
    connect(m_filterMethodCombo, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));

    connect(m_blackFrameButton, SIGNAL(clicked()),
            this, SLOT(slotAddBlackFrame()));
                                                  
    connect(m_blackFrameListView, SIGNAL(blackFrameSelected(Q3ValueList<HotPixel>, const KUrl&)),
            this, SLOT(slotBlackFrame(Q3ValueList<HotPixel>, const KUrl&))); 
}

ImageEffect_HotPixels::~ImageEffect_HotPixels()
{
}

void ImageEffect_HotPixels::readUserSettings(void)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("hotpixels Tool Dialog");
    m_blackFrameURL = KUrl(group.readEntry("Last Black Frame File", QString()));
    m_filterMethodCombo->setCurrentItem(group.readEntry("Filter Method",
                                        (int)HotPixelFixer::QUADRATIC_INTERPOLATION));
    
    if (m_blackFrameURL.isValid())
        new BlackFrameListViewItem(m_blackFrameListView, m_blackFrameURL);
}

void ImageEffect_HotPixels::writeUserSettings(void)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("hotpixels Tool Dialog");
    group.writeEntry("Last Black Frame File", m_blackFrameURL.url());
    group.writeEntry("Filter Method", m_filterMethodCombo->currentItem());
    group.sync();
}

void ImageEffect_HotPixels::resetValues(void)
{
    m_filterMethodCombo->blockSignals(true);
    m_filterMethodCombo->setCurrentItem(HotPixelFixer::QUADRATIC_INTERPOLATION);
    m_filterMethodCombo->blockSignals(false);
} 

void ImageEffect_HotPixels::slotAddBlackFrame()
{
    //Does one need to do this if digikam did so already?
     
    
    KFileDialog fileSelectDialog(QString(), KImageIO::pattern(), this, "", true);
    fileSelectDialog.setCaption(i18n("Select Black Frame Image"));
    fileSelectDialog.setUrl(m_blackFrameURL.path());
    
    if (fileSelectDialog.exec() != QDialog::Rejected)
    {
       //Load the selected file and insert into the list
        
       m_blackFrameURL = fileSelectDialog.selectedURL();
       m_blackFrameListView->clear();
       new BlackFrameListViewItem(m_blackFrameListView, m_blackFrameURL);
    }
}

void ImageEffect_HotPixels::renderingFinished(void)
{
    m_filterMethodCombo->setEnabled(true);
    m_blackFrameListView->setEnabled(true);
    enableButton(Apply, true);     
}

void ImageEffect_HotPixels::prepareEffect()
{
    m_filterMethodCombo->setEnabled(false);
    m_blackFrameListView->setEnabled(false);
    enableButton(Apply, false);     

    Digikam::DImg image     = m_imagePreviewWidget->getOriginalRegionImage();
    int interpolationMethod = m_filterMethodCombo->currentItem();

    Q3ValueList<HotPixel> hotPixelsRegion;
    QRect area = m_imagePreviewWidget->getOriginalImageRegionToRender();
    Q3ValueList<HotPixel>::Iterator end(m_hotPixelsList.end()); 
    
    for (Q3ValueList<HotPixel>::Iterator it = m_hotPixelsList.begin() ; it != end ; ++it )
    {
        HotPixel hp = (*it);
        
        if ( area.contains( hp.rect ) )
        {
           hp.rect.moveTopLeft(QPoint( hp.rect.x()-area.x(), hp.rect.y()-area.y() ));
           hotPixelsRegion.append(hp);
        }
    }

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new HotPixelFixer(&image, this, hotPixelsRegion, interpolationMethod));
}

void ImageEffect_HotPixels::prepareFinal()
{
    m_filterMethodCombo->setEnabled(false);
    m_blackFrameListView->setEnabled(false);
    enableButton(Apply, false);     
        
    int interpolationMethod = m_filterMethodCombo->currentItem();

    Digikam::ImageIface iface(0, 0);
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new HotPixelFixer(iface.getOriginalImg(), this,m_hotPixelsList,interpolationMethod));
}

void ImageEffect_HotPixels::putPreviewData(void)
{
    m_imagePreviewWidget->setPreviewImage(m_threadedFilter->getTargetImage());
}

void ImageEffect_HotPixels::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Hot Pixels Correction"), m_threadedFilter->getTargetImage().bits());
}

void ImageEffect_HotPixels::slotBlackFrame(Q3ValueList<HotPixel> hpList, const KUrl& blackFrameURL)
{
    m_blackFrameURL = blackFrameURL;
    m_hotPixelsList = hpList;
    
    Q3PointArray pointList(m_hotPixelsList.size());
    Q3ValueList <HotPixel>::Iterator it;
    int i = 0;
    Q3ValueList <HotPixel>::Iterator end(m_hotPixelsList.end());
    
    for (it = m_hotPixelsList.begin() ; it != end ; ++it, i++)
       pointList.setPoint(i, (*it).rect.center());
        
    m_imagePreviewWidget->setPanIconHighLightPoints(pointList);
    
    slotEffect();
}

}  // NameSpace DigikamHotPixelsImagesPlugin
