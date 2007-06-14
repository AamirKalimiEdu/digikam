/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-11
 * Description : a plugin to apply Distortion FX to a picture.
 *
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * Original Distortion algorithms copyrighted 2004-2005 by 
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
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

#ifndef IMAGEEFFECT_DISTORTIONFX_H
#define IMAGEEFFECT_DISTORTIONFX_H

// Digikam includes.

#include "imageguidedlg.h"
//Added by qt3to4:
#include <QLabel>

class QComboBox;
class QLabel;

class KIntNumInput;

namespace DigikamDistortionFXImagesPlugin
{

class ImageEffect_DistortionFX : public Digikam::ImageGuideDlg
{
    Q_OBJECT
    
public:

    ImageEffect_DistortionFX(QWidget *parent);
    ~ImageEffect_DistortionFX();

private slots:

    void slotEffectTypeChanged(int type);
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
    
    QComboBox            *m_effectType;

    QLabel               *m_effectTypeLabel;
    QLabel               *m_levelLabel;
    QLabel               *m_iterationLabel;
    
    KIntNumInput         *m_levelInput;
    KIntNumInput         *m_iterationInput;
};

}  // NameSpace DigikamDistortionFXImagesPlugin

#endif /* IMAGEEFFECT_DISTORTIONFX_H */
