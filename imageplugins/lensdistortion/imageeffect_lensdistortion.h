/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-27
 * Description : a plugin to reduce lens distorsions to an image.
 * 
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEEFFECT_LENSDISTORTION_H
#define IMAGEEFFECT_LENSDISTORTION_H

// Qt includes.

#include <qimage.h>
//Added by qt3to4:
#include <QLabel>

// Digikam includes.

#include "dimg.h"
#include "imageguidedlg.h"

class QLabel;

class KDoubleNumInput;

namespace DigikamLensDistortionImagesPlugin
{

class ImageEffect_LensDistortion : public Digikam::ImageGuideDlg
{
    Q_OBJECT
    
public:

    ImageEffect_LensDistortion(QWidget *parent);
    ~ImageEffect_LensDistortion();

private slots:

    void readUserSettings();

private:

    void writeUserSettings();
    void resetValues();     
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:

    QLabel          *m_maskPreviewLabel;

    KDoubleNumInput *m_mainInput;
    KDoubleNumInput *m_edgeInput;
    KDoubleNumInput *m_rescaleInput;
    KDoubleNumInput *m_brightenInput;

    Digikam::DImg    m_previewRasterImage;
};

}  // NameSpace DigikamLensDistortionImagesPlugin

#endif /* IMAGEEFFECT_LENSDISTORTION_H */
