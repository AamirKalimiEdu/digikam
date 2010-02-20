/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-04
 * Description : digiKam image editor plugin core
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPLUGIN_CORE_H
#define IMAGEPLUGIN_CORE_H

// Qt includes

#include <QVariant>

// Local includes

#include "imageplugin.h"
#include "digikam_export.h"

namespace Digikam
{
    class IccProfile;
}

class ImagePlugin_CorePriv;

class ImagePlugin_Core : public Digikam::ImagePlugin
{
    Q_OBJECT

public:

    ImagePlugin_Core(QObject *parent, const QVariantList& args);
    ~ImagePlugin_Core();

    void setEnabledSelectionActions(bool b);
    void setEnabledActions(bool b);

    typedef Digikam::IccProfile IccProfile; // to make signal/slot work

private Q_SLOTS:

    void slotBlur();
    void slotSharpen();
    void slotNoiseReduction();
    void slotRedEye();

    void slotBCG();
    void slotCB();
    void slotHSL();
    void slotAutoCorrection();
    void slotInvert();
    void slotBW();
    void slotInfrared();
    void slotConvertTo8Bits();
    void slotConvertTo16Bits();
    void slotConvertToColorSpace(const IccProfile&);
    void slotProfileConversionTool();

    void slotRatioCrop();
    void slotResize();

    void slotUpdateColorSpaceMenu();
    void slotSetupICC();

private:

    ImagePlugin_CorePriv* const d;
};

#endif /* IMAGEPLUGIN_CORE_H */
