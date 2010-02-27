/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image histogram adjust curves.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ADJUSTCURVESTOOL_H
#define ADJUSTCURVESTOOL_H

// Local includes

#include "editortool.h"

namespace Digikam
{
class DColor;
}

using namespace Digikam;

namespace DigikamAdjustCurvesImagesPlugin
{

class AdjustCurvesToolPriv;

class AdjustCurvesTool : public EditorTool
{
    Q_OBJECT

public:

    AdjustCurvesTool(QObject* parent);
    ~AdjustCurvesTool();

private:

    void readSettings();
    void writeSettings();
    void finalRendering();

private Q_SLOTS:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotEffect();
    void slotResetSettings();
    void slotResetCurrentChannel();
    void slotSpotColorChanged(const Digikam::DColor& color);
    void slotColorSelectedFromTarget(const Digikam::DColor& color);
    void slotPickerColorButtonActived(int);
    void slotChannelChanged();
    void slotScaleChanged();

private:

    AdjustCurvesToolPriv* const d;
};

}  // namespace DigikamAdjustCurvesImagesPlugin

#endif /* ADJUSTCURVESTOOL_H */
