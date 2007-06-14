/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-14
 * Description : a plugin to insert a text over an image.
 * 
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEEFFECT_INSERTEXT_H
#define IMAGEEFFECT_INSERTEXT_H

// Qt includes.

#include <qcolor.h>
#include <qimage.h>
//Added by qt3to4:
#include <QLabel>

// Digikam includes.

#include "imagedlgbase.h"

class QLabel;
class QFont;
class Q3HButtonGroup;
class QComboBox;
class QCheckBox;

class KTextEdit;
class KColorButton;

namespace DigikamInsertTextImagesPlugin
{

class InsertTextWidget;
class FontChooserWidget;

class ImageEffect_InsertText : public Digikam::ImageDlgBase
{
    Q_OBJECT
    
public:

    ImageEffect_InsertText(QWidget *parent);
    ~ImageEffect_InsertText();

signals:

    void signalUpdatePreview();

private slots:

    void slotFontPropertiesChanged(const QFont &font);
    void slotUpdatePreview();
    void slotAlignModeChanged(int mode);

private:

    void readUserSettings();
    void writeUserSettings();
    void resetValues();
    void finalRendering();    

private:
    
    int                m_alignTextMode;
    int                m_defaultSizeFont;
        
    QComboBox         *m_textRotation;
    
    QCheckBox         *m_borderText;    
    QCheckBox         *m_transparentText;
    
    Q3HButtonGroup     *m_alignButtonGroup;
        
    QFont              m_textFont;
    
    KColorButton      *m_fontColorButton;
    
    FontChooserWidget *m_fontChooserWidget;
    
    KTextEdit         *m_textEdit;
    
    InsertTextWidget  *m_previewWidget;
};

}  // NameSpace DigikamInsertTextImagesPlugin

#endif /* IMAGEEFFECT_INSERTEXT_H */
