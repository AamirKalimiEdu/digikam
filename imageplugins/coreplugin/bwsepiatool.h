/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : Black and White conversion tool.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BWSEPIATOOL_H
#define BWSEPIATOOL_H

// Local includes

#include "editortool.h"

namespace Digikam
{
class DColor;
}

using namespace Digikam;

namespace DigikamImagesPluginCore
{

class BWSepiaToolPriv;

class BWSepiaTool : public EditorTool
{
    Q_OBJECT

public:

    enum SettingsTab
    {
        FilmTab=0,
        BWFiltersTab,
        ToneTab,
        LuminosityTab
    };
    
public:

    BWSepiaTool(QObject* parent);
    ~BWSepiaTool();

    friend class PreviewPixmapFactory;

private:

    void readSettings();
    void writeSettings();
    void finalRendering();
    void blockWidgetSignals(bool b);

private Q_SLOTS:

    void slotInit();
    void slotResetSettings();
    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotEffect();
    void slotScaleChanged();
    void slotSpotColorChanged(const Digikam::DColor& color);
    void slotColorSelectedFromTarget(const Digikam::DColor& color);
    void slotFilterSelected();

private:

    BWSepiaToolPriv* const d;
};

}  // namespace DigikamImagesPluginCore

#endif /* BWSEPIATOOL_H */
