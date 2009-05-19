/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-23
 * Description : a plugin to shear an image
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SHEARTOOL_H
#define SHEARTOOL_H

// Local includes

#include "editortool.h"

namespace DigikamShearToolImagesPlugin
{

class ShearToolPriv;

class ShearTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    ShearTool(QObject* parent);
    ~ShearTool();

private Q_SLOTS:

    void slotResetSettings();
    void slotColorGuideChanged();

private:

    void readSettings();
    void writeSettings();
    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void renderingFinished(void);

private:

    ShearToolPriv* const d;
};

}  // namespace DigikamShearToolImagesPlugin

#endif /* SHEARTOOL_H */
