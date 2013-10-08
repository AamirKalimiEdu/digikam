/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Sorter
 *
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013 by Gowtham Ashok <gwty93 at gmail dot com>
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

#ifndef IMGQSORT_H
#define IMGQSORT_H

// Local includes

#include "digikam_export.h"
#include "dimg.h"
#include "globals.h"
#include "imagequalitysettings.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImgQSort
{
public:

    /** Standard constructor with image container to parse
     */
    explicit ImgQSort();
    ~ImgQSort();

    /** Perform  quality estimation and return Pick Label value accordingly.
     */
    PickLabel analyseQuality(const DImg& img, const ImageQualitySettings& imq);

private:

    /** Internal method dedicated to convert DImg pixels from integer values to float values.
     *  These ones will by used internally by ImgQSort through OpenCV API.
     */
    void readImage();

    /**
    * @function CannyThreshold
    * @brief Trackbar callback - Canny thresholds input with a ratio 1:3
    */
    void   CannyThreshold(int, void*) const;

    double blurdetector()        const;
    short  blurdetector2()       const;
    double noisedetector()       const;
    int    compressiondetector() const;
    bool   runningFlag()         const;

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* IMGQSORT_H */