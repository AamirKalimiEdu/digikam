/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-10
 * Description : Film Grain settings view.
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FILMGRAINSETTINGS_H
#define FILMGRAINSETTINGS_H

// Local includes

#include <QWidget>

// KDE includes

#include <kconfig.h>

// Local includes

#include "digikam_export.h"
#include "filmgrainfilter.h"

namespace Digikam
{

class FilmGrainSettingsPriv;

class DIGIKAM_EXPORT FilmGrainSettings : public QWidget
{
    Q_OBJECT

public:

    FilmGrainSettings(QWidget* parent);
    ~FilmGrainSettings();

    FilmGrainContainer defaultSettings() const;
    void resetToDefault();

    FilmGrainContainer settings() const;
    void setSettings(const FilmGrainContainer& settings);

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:    
    
    void slotAddLuminanceNoise(bool);
    void slotAddChrominanceNoise(bool);

private:

    FilmGrainSettingsPriv* const d;
};

}  // namespace Digikam

#endif /* FILMGRAINSETTINGS_H */
