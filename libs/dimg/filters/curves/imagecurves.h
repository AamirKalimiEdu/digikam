/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image curves manipulation methods.
 *
 * Copyright (c) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGECURVES_H
#define IMAGECURVES_H

// Qt includes

#include <QtCore/QPoint>
#include <QtGui/QPolygon>

// KDE includes

#include <kurl.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class ImageCurvesPriv;

class DIGIKAM_EXPORT ImageCurves
{

public:

    /**
     * The max number of points contained in a curve.
     */
    const static int NUM_POINTS;

    /**
     * Number of channels in a curve.
     */
    const static int NUM_CHANNELS;

    /**
     * Curve points have to multiplied with this value for 16 bit images.
     */
    const static int MULTIPLIER_16BIT;

    enum CurveType
    {
        CURVE_SMOOTH = 0,            // Smooth curve type
        CURVE_FREE                   // Freehand curve type.
    };

    typedef double CRMatrix[4][4];

public:

    ImageCurves(bool sixteenBit);
    ~ImageCurves();

    /**
     * Fills this curves with the data supplied by another curves object. This
     * ensures that 8 and 16 bit curves are properly converted.
     *
     * @param otherCurves other curves object to adapt config from
     */
    void fillFromOtherCurvers(ImageCurves* otherCurves);

    // Methods for to manipulate the curves data.

    bool   isDirty();
    bool   isSixteenBits();
    void   curvesReset();
    void   curvesChannelReset(int channel);
    void   curvesCalculateCurve(int channel);
    float  curvesLutFunc(int n_channels, int channel, float value);
    void   curvesLutSetup(int nchannels);
    void   curvesLutProcess(uchar *srcPR, uchar *destPR, int w, int h);

    // Methods for to set manually the curves values.

    void   setCurveValue(int channel, int bin, int val);
    void   setCurvePointX(int channel, int point, int x);
    void   setCurvePointY(int channel, int point, int y);
    void   setCurveType(int channel, CurveType type);
    void   setCurveType(CurveType type);

    void   setCurvePoint(int channel, int point, const QPoint& val);
    void   setCurvePoints(int channel, const QPolygon& vals);
    void   setCurveValues(int channel, const QPolygon& vals);

    int    getCurveValue(int channel, int bin);
    int    getCurvePointX(int channel, int point);
    int    getCurvePointY(int channel, int point);
    CurveType getCurveType(int channel);

    static QPoint getDisabledValue();
    bool isCurvePointEnabled(int channel, int point) const;
    QPoint getCurvePoint(int channel, int point);
    QPolygon getCurvePoints(int channel);
    QPolygon getCurveValues(int channel);

    // Methods for to save/load the curves values to/from a Gimp curves text file.

    bool   saveCurvesToGimpCurvesFile(const KUrl& fileUrl);
    bool   loadCurvesFromGimpCurvesFile(const KUrl& fileUrl);

private:

    void curvesPlotCurve(int channel, int p1, int p2, int p3, int p4);
    void curvesCRCompose(CRMatrix a, CRMatrix b, CRMatrix ab);

    void freeLutData();

private:

    ImageCurvesPriv* const d;
};

}  // namespace Digikam

#endif /* IMAGECURVES_H */
