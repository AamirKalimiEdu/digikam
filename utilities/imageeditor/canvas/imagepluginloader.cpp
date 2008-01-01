/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-04
 * Description : image plugins loader for image editor.
 * 
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

// Qt includes.

#include <QMap>
#include <QList>

// KDE includes.

#include <kservicetypetrader.h>
#include <klibloader.h>
#include <kapplication.h>
#include <klocale.h>
#include <kxmlguiclient.h>

// Local includes.

#include "ddebug.h"
#include "splashscreen.h"
#include "imagepluginloader.h"

namespace Digikam
{

// List of obsolete image plugins name. 

static const char* ObsoleteImagePluginsList[] =
{
     "digikamimageplugin_blowup",    // Merged with "Resize" tool since 0.9.2.  
     "digikamimageplugin_solarize",  // Renamed "ColorFx" since 0.9.2.  
     "digikamimageplugin_unsharp",   // Merged with "Sharpen" tool since 0.9.2.
     "digikamimageplugin_refocus",   // Merged with "Sharpen" tool since 0.9.2.
     "digikamimageplugin_despeckle", // Renamed "Noise Reduction" since 0.9.2.
     "-1"
};

class ImagePluginLoaderPrivate
{

public:

    ImagePluginLoaderPrivate()
    {
        splash = 0;

        for (int i=0 ; QString(ObsoleteImagePluginsList[i]) != QString("-1") ; i++)
            obsoleteImagePluginsList << ObsoleteImagePluginsList[i];
    }

    QStringList   obsoleteImagePluginsList;

    SplashScreen *splash;

    // a map of _loaded_ plugins
    QMap<QString, ImagePlugin*> pluginMap;
    // a map of _available_ plugins
    QMap<QString, KService::Ptr> pluginServiceMap;
};

ImagePluginLoader* ImagePluginLoader::m_instance=0;

ImagePluginLoader* ImagePluginLoader::instance()
{
    return m_instance;
}

ImagePluginLoader::ImagePluginLoader(QObject *parent, SplashScreen *splash)
                 : QObject(parent)
{
    m_instance = this;
    d = new ImagePluginLoaderPrivate;
    d->splash = splash;

    QStringList imagePluginsList2Load;

    const KService::List offers = KServiceTypeTrader::self()->query("Digikam/ImagePlugin");
    foreach (KService::Ptr service, offers)
    {
        if (service)
            d->pluginServiceMap[service->name()] = service;
    }

    foreach (KService::Ptr service, d->pluginServiceMap)
    {
        if (!d->obsoleteImagePluginsList.contains(service->library()))
            imagePluginsList2Load.append(service->name());
    }

    loadPluginsFromList(imagePluginsList2Load);
}

ImagePluginLoader::~ImagePluginLoader()
{
    QList<QString> pluginNames = d->pluginMap.keys();
    foreach(QString name, pluginNames)
    {
        ImagePlugin *plugin = d->pluginMap.value(name);
        KService::Ptr service = d->pluginServiceMap.value(name);
        delete plugin;
        //if (service)
          //  KLibLoader::self()->unloadLibrary(service->library());
    }
    delete d;
    m_instance = 0;
}

void ImagePluginLoader::loadPluginsFromList(const QStringList& pluginsToLoad)
{
    if (d->splash)
        d->splash->message(i18n("Loading Image Plugins"));

    int cpt = 0;

    // Load plugin core at the first time.

    KService::Ptr corePlugin = d->pluginServiceMap.value("ImagePlugin_Core");

    if (corePlugin && !pluginIsLoaded(corePlugin->name()) )
    {
        QString error;

        ImagePlugin *plugin = KService::createInstance<ImagePlugin>(corePlugin, this, QVariantList(), &error);

        if (plugin && (dynamic_cast<KXMLGUIClient*>(plugin) != 0))
        {
            d->pluginMap[corePlugin->name()] = plugin;

            DDebug() << "ImagePluginLoader: Loaded plugin " << corePlugin->name() << endl;

            ++cpt;
        }
        else
        {
            DWarning() << "ImagePluginLoader: createInstance returned 0 for "
                       << corePlugin->name()
                       << " (" << corePlugin->library() << ")"
                       << " with error: "
                       << error << endl;
        }
    }

    // Load all other image plugins after (make a coherant menu construction in Image Editor).

    foreach (QString name, pluginsToLoad)
    {
        KService::Ptr service = d->pluginServiceMap.value(name);
        ImagePlugin *plugin;

        if( pluginIsLoaded(name) )
            continue;
        else
        {
            QString error;

            plugin = KService::createInstance<ImagePlugin>(service, this, QVariantList(), &error);

            if (plugin && (dynamic_cast<KXMLGUIClient*>(plugin) != 0))
            {
                d->pluginMap[name] = plugin;

                DDebug() << "ImagePluginLoader: Loaded plugin " << service->name() << endl;

                ++cpt;
            }
            else
            {
                DWarning() << "ImagePluginLoader: createInstance returned 0 for "
                           << service->name()
                           << " (" << service->library() << ")"
                           << " with error: "
                           << error << endl;
            }
        }
    }

    d->splash = 0;       // Splashcreen is only displayed at the first time.
                         // If user change plugins list to use in setup, don't try to 
                         // use the old splashscreen instance.
}

ImagePlugin* ImagePluginLoader::pluginIsLoaded(const QString& name)
{
    return d->pluginMap.value(name);
}

ImagePlugin* ImagePluginLoader::pluginInstance(const QString& libraryName)
{
    foreach (KService::Ptr service, d->pluginServiceMap)
    {
        if(service->library() == libraryName)
        {
            return ( pluginIsLoaded(service->name()) );
        }
    }

    return 0;
}

ImagePlugin* ImagePluginLoader::corePluginInstance()
{
    return pluginIsLoaded("ImagePlugin_Core");
}

bool ImagePluginLoader::pluginLibraryIsLoaded(const QString& libraryName)
{
    foreach (KService::Ptr service, d->pluginServiceMap)
    {
        if(service->library() == libraryName)
        {
            if( pluginIsLoaded(service->name()) )
                return true;
        }
    }

    return false;
}

QList<ImagePlugin *> ImagePluginLoader::pluginList()
{
    return d->pluginMap.values();
}

}  // namespace Digikam
