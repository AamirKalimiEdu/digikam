/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image histogram adjust curves. 
 * 
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes.
  
#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>

// Local includes.

#include "ddebug.h"
#include "adjustcurves.h"
#include "imageplugin_adjustcurves.h"
#include "imageplugin_adjustcurves.moc"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_adjustcurves,
                            KGenericFactory<ImagePlugin_AdjustCurves>("digikamimageplugin_adjustcurves"));

ImagePlugin_AdjustCurves::ImagePlugin_AdjustCurves(QObject *parent,
                                                   const QStringList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_AdjustCurves")
{
    m_curvesAction  = new KAction(KIcon("adjustcurves"), i18n("Curves Adjust..."), this);
    actionCollection()->addAction("imageplugin_adjustcurves", m_curvesAction );

    connect(m_curvesAction, SIGNAL(triggered(bool) ), 
            this, SLOT(slotCurvesAdjust()));

    m_curvesAction->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_M));
    setXMLFile("digikamimageplugin_adjustcurves_ui.rc");

    DDebug() << "ImagePlugin_AdjustCurves plugin loaded" << endl;
}

ImagePlugin_AdjustCurves::~ImagePlugin_AdjustCurves()
{
}

void ImagePlugin_AdjustCurves::setEnabledActions(bool enable)
{
    m_curvesAction->setEnabled(enable);
}

void ImagePlugin_AdjustCurves::slotCurvesAdjust()
{
    DigikamAdjustCurvesImagesPlugin::AdjustCurveDialog dlg(parentWidget());
    dlg.exec();
}
