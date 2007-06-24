/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-30
 * Description : a plugin to add rain drop over an image
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
#include "imageeffect_raindrop.h"
#include "imageplugin_raindrop.h"
#include "imageplugin_raindrop.moc"

K_EXPORT_COMPONENT_FACTORY(digikamimageplugin_raindrop,
                           KGenericFactory<ImagePlugin_RainDrop>("digikamimageplugin_raindrop"));

ImagePlugin_RainDrop::ImagePlugin_RainDrop(QObject *parent, const QStringList &)
                    : Digikam::ImagePlugin(parent, "ImagePlugin_RainDrop")
{
    m_raindropAction  = new KAction(KIcon("raindrop"), i18n("Raindrops..."), this);
    actionCollection()->addAction("imageplugin_raindrop", m_raindropAction );
    connect(m_raindropAction, SIGNAL(triggered(bool) ), SLOT(slotRainDrop()));
                
    setXMLFile( "digikamimageplugin_raindrop_ui.rc" );    
        
    DDebug() << "ImagePlugin_RainDrop plugin loaded" << endl;
}

ImagePlugin_RainDrop::~ImagePlugin_RainDrop()
{
}

void ImagePlugin_RainDrop::setEnabledActions(bool enable)
{
    m_raindropAction->setEnabled(enable);
}

void ImagePlugin_RainDrop::slotRainDrop()
{
    DigikamRainDropImagesPlugin::ImageEffect_RainDrop dlg(parentWidget());
    dlg.exec();
}
