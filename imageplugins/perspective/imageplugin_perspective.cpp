/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-17
 * Description : a plugin to change image perspective .
 * 
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "imageeffect_perspective.h"
#include "imageplugin_perspective.h"
#include "imageplugin_perspective.moc"

K_EXPORT_COMPONENT_FACTORY(digikamimageplugin_perspective,
                           KGenericFactory<ImagePlugin_Perspective>("digikamimageplugin_perspective"));

ImagePlugin_Perspective::ImagePlugin_Perspective(QObject *parent, const QStringList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_Perspective")
{
    m_perspectiveAction  = new KAction(KIcon("perspective"), i18n("Perspective Adjustment..."), this);
    actionCollection()->addAction("imageplugin_perspective", m_perspectiveAction );
    connect(m_perspectiveAction, SIGNAL(triggered(bool) ), SLOT(slotPerspective()));
    
    setXMLFile("digikamimageplugin_perspective_ui.rc");      
                                    
    DDebug() << "ImagePlugin_Perspective plugin loaded" << endl;
}

ImagePlugin_Perspective::~ImagePlugin_Perspective()
{
}

void ImagePlugin_Perspective::setEnabledActions(bool enable)
{
    m_perspectiveAction->setEnabled(enable);
}

void ImagePlugin_Perspective::slotPerspective()
{
    DigikamPerspectiveImagesPlugin::ImageEffect_Perspective dlg(parentWidget());
    dlg.exec();
}

