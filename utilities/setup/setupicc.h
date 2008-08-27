/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-11-24
 * Description : Color management setup tab.
 *
 * Copyright (C) 2005-2007 by F.J. Cruz <fj.cruz@supercable.es>
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

#ifndef SETUPICC_H
#define SETUPICC_H

// Qt includes.

#include <QWidget>
#include <QMap>
#include <QDir>

// Local includes.

#include <kurl.h>
#include "digikam_export.h"

class KPageDialog;

namespace Digikam
{

class SetupICCPriv;

class DIGIKAM_EXPORT SetupICC : public QWidget
{
    Q_OBJECT

public:

    SetupICC(QWidget* parent = 0, KPageDialog* dialog = 0);
    ~SetupICC();

    void applySettings();

    static bool iccRepositoryIsValid();

private:

    void readSettings(bool restore=false);
    void fillCombos(const QString& path, bool report);
    void enableWidgets();
    void disableWidgets();
    void profileInfo(const QString&);
    bool parseProfilesfromDir(const QFileInfoList& files);

private slots:

    void processLcmsUrl(const QString&);
    void slotToggledWidgets(bool);
    void slotToggleManagedView(bool);
    void slotFillCombos(const KUrl& url);
    void slotClickedIn();
    void slotClickedWork();
    void slotClickedMonitor();
    void slotClickedProof();

private:

    SetupICCPriv* d;
};

}  // namespace Digikam

#endif // SETUPICC_H 
