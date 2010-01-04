/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : Image editor interface used by editor tools.
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef EDITORTOOLIFACE_H
#define EDITORTOOLIFACE_H

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class EditorTool;
class EditorWindow;
class ExposureSettingsContainer;
class ICCSettingsContainer;
class EditorToolIfacePriv;

class DIGIKAM_EXPORT EditorToolIface : public QObject
{
    Q_OBJECT

public:

    static EditorToolIface* editorToolIface();

    EditorToolIface(EditorWindow *editor);
    ~EditorToolIface();

    EditorTool* currentTool() const;

    void loadTool(EditorTool* tool);
    void unLoadTool();

    void setToolStartProgress(const QString& toolName);
    void setToolProgress(int progress);
    void setToolStopProgress();

    void setupICC();

    void setExposureSettings(ExposureSettingsContainer* expoSettings);
    void setICCSettings(ICCSettingsContainer* cmSettings);

public Q_SLOTS:

    void slotCloseTool();
    void slotToolAborted();

private:

    static EditorToolIface *m_iface;

    EditorToolIfacePriv* const d;
};

}  // namespace Digikam

#endif /* EDITORTOOLIFACE_H */
