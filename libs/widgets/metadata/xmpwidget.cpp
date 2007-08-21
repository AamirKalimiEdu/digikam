/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-07-19
 * Description : A widget to display XMP metadata
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QFile>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "xmpwidget.h"
#include "xmpwidget.moc"

namespace Digikam
{

static const char* XmpHumanList[] =
{
     "Description",
     "City",
     "Relation",
     "Rights",
     "Publisher",
     "CreateDate",
     "Title",
     "Identifier",
     "State",
     "Source",
     "Rating",
     "Advisory",
     "-1"
};

static const char* StandardXmpEntryList[] =
{
     "aux",       // Schema for Additional Exif Properties.   
     "crs",       // Camera Raw schema.
     "dc",        // Dublin Core schema.
     "exif",      // Schema for Exif-specific Properties.
     "iptc",      // IPTC Core schema.
     "pdf",       // Adobe PDF schema.
     "photoshop", // Adobe Photoshop schema.
     "tiff",      // Schema for TIFF Properties
     "xmp",       // Basic schema.
     "xmpBJ",     // Basic Job Ticket schema.
     "xmpDM",     // Dynamic Media schema.
     "xmpMM",     // Media Management schema.
     "xmpRights", // Rights Management schema.
     "xmpTPg",    // Paged-Text schema.
     "-1"
};

XmpWidget::XmpWidget(QWidget* parent, const char* name)
         : MetadataWidget(parent, name)
{
    for (int i=0 ; QString(StandardXmpEntryList[i]) != QString("-1") ; i++)
        m_keysFilter << StandardXmpEntryList[i];

    for (int i=0 ; QString(XmpHumanList[i]) != QString("-1") ; i++)
        m_tagsfilter << XmpHumanList[i];
}

XmpWidget::~XmpWidget()
{
}

QString XmpWidget::getMetadataTitle(void)
{
    return i18n("XMP Schema");
}

bool XmpWidget::loadFromURL(const KUrl& url)
{
    setFileName(url.fileName());

    if (url.isEmpty())
    {
        setMetadata();
        return false;
    }
    else
    {
        DMetadata metadata(url.path());

        if (!metadata.asXmp())
        {
            setMetadata();
            return false;
        }
        else
            setMetadata(metadata);
    }

    return true;
}

bool XmpWidget::decodeMetadata()
{
    DMetadata data = getMetadata();
    if (!data.asXmp())
        return false;

    // Update all metadata contents.
    setMetadataMap(data.getXmpTagsDataList(m_keysFilter));
    return true;
}

void XmpWidget::buildView(void)
{
    if (getMode() == SIMPLE)
    {
        setIfdList(getMetadataMap(), m_tagsfilter);
    }
    else
    {
        setIfdList(getMetadataMap());
    }
}

QString XmpWidget::getTagTitle(const QString& key)
{
    QString title = DMetadata::getXmpTagTitle(key.toAscii());

    if (title.isEmpty())
        return i18n("Unknown");

    return title;
}

QString XmpWidget::getTagDescription(const QString& key)
{
    QString desc = DMetadata::getXmpTagDescription(key.toAscii());

    if (desc.isEmpty())
        return i18n("No description available");

    return desc;
}

void XmpWidget::slotSaveMetadataToFile(void)
{
    KUrl url = saveMetadataToFile(i18n("XMP File to Save"),
                                  QString("*.dat|"+i18n("XMP binary Files (*.dat)")));
    storeMetadataToFile(url, getMetadata().getXmp());
}

}  // namespace Digikam

