/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LENSFUNIFACE_H
#define LENSFUNIFACE_H

// Lib LensFun includes

extern "C"
{
#include <lensfun.h>
}

namespace DigikamAutoCorrectionImagesPlugin
{

class LensFunIface
{
    friend class LensFunSettings;
    friend class LensFunFilter;

public:

    LensFunIface();
    virtual ~LensFunIface();

//    typedef QMap<QString, QString> correctionData;
//    correctionData getCorrectionData();
    void setCorrection(bool CCA, bool Vignettation, bool CCI, bool Distortion, bool Geometry);

    bool supportsDistortion();
    bool supportsCCA();
    bool supportsVig();
    bool supportsGeometry(){ return supportsDistortion(); };
    bool supportsCCI()     { return supportsVig();        };

protected:

    bool init();

private:

    // my configuration
    bool                   m_init;
    bool                   m_filterCCA;
    bool                   m_filterVig;
    bool                   m_filterCCI;
    bool                   m_filterDist;
    bool                   m_filterGeom;

    // Database items
    lfDatabase*            m_lfDb;
    const lfCamera* const* m_lfCameras;
    const lfLens**         m_lfLenses;
    const lfMount*         m_lfMounts;

    // To be used for modification
    const lfLens*          m_usedLens;
    float                  m_cropFactor;
    float                  m_focalLength;
    float                  m_aperture;
    float                  m_subjectDistance;
};

}  // namespace DigikamAutoCorrectionImagesPlugin

#endif /* LENSFUNIFACE_H */
