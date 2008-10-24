/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning of a single image
 *
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes.

#include "imagescanner.h"

// Qt includes.

#include <QImageReader>

// KDE includes.

#include <kdebug.h>
#include <kmimetype.h>
#include <klocale.h>

// Digikam includes.

#include "databaseurl.h"
#include "databaseaccess.h"
#include "albumdb.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "imagecomments.h"
#include "imagecopyright.h"

namespace Digikam
{

ImageScanner::ImageScanner(const QFileInfo &info, const ItemScanInfo &scanInfo)
    : m_fileInfo(info), m_scanInfo(scanInfo)
{
}

ImageScanner::ImageScanner(const QFileInfo &info)
    : m_fileInfo(info)
{
}

ImageScanner::ImageScanner(qlonglong imageid)
{
    ItemShortInfo shortInfo;
    {
        DatabaseAccess access;
        shortInfo  = access.db()->getItemShortInfo(imageid);
        m_scanInfo = access.db()->getItemScanInfo(imageid);
    }

    QString albumRootPath = CollectionManager::instance()->albumRootPath(shortInfo.albumRootID);
    m_fileInfo = QFileInfo(DatabaseUrl::fromAlbumAndName(shortInfo.itemName,
                            shortInfo.album, albumRootPath, shortInfo.albumRootID).fileUrl().path());
}

void ImageScanner::setCategory(DatabaseItem::Category category)
{
    // we don't have the necessary information in this class, but in CollectionScanner
    m_scanInfo.category = category;
}

void ImageScanner::fileModified()
{
    loadFromDisk();
    updateHardInfos();
}

void ImageScanner::newFile(int albumId)
{
    loadFromDisk();
    addImage(albumId);
    if (!scanFromIdenticalFile())
        scanFile();
}

void ImageScanner::fullScan()
{
    loadFromDisk();
    updateImage();
    scanFile();
}

void ImageScanner::copiedFrom(int albumId, qlonglong srcId)
{
    loadFromDisk();
    addImage(albumId);
    // first use source, if it exists
    if (!copyFromSource(srcId))
        // check if we can establish identity
        if (!scanFromIdenticalFile())
            // scan newly
            scanFile();
}

void ImageScanner::addImage(int albumId)
{
    // there is a limit here for file size <2TB
    m_scanInfo.albumID = albumId;
    m_scanInfo.itemName = m_fileInfo.fileName();
    m_scanInfo.status = DatabaseItem::Visible;
    // category is set by setCategory
    m_scanInfo.modificationDate = m_fileInfo.lastModified();
    int fileSize = (int)m_fileInfo.size();
    // the QByteArray is an ASCII hex string
    m_scanInfo.uniqueHash = QString(m_img.getUniqueHash());

    kDebug(50003) << "Adding new item" << m_fileInfo.filePath();
    m_scanInfo.id = DatabaseAccess().db()->addItem(m_scanInfo.albumID, m_scanInfo.itemName, m_scanInfo.status, m_scanInfo.category,
                                                   m_scanInfo.modificationDate, fileSize, m_scanInfo.uniqueHash);
}

void ImageScanner::updateImage()
{
    // part from addImage()
    m_scanInfo.modificationDate = m_fileInfo.lastModified();
    int fileSize = (int)m_fileInfo.size();
    m_scanInfo.uniqueHash = QString(m_img.getUniqueHash());

    DatabaseAccess().db()->updateItem(m_scanInfo.id, m_scanInfo.category,
                                      m_scanInfo.modificationDate, fileSize, m_scanInfo.uniqueHash);
}

void ImageScanner::scanFile()
{
    if (m_scanInfo.category == DatabaseItem::Image)
    {
        scanImageInformation();
        if (m_hasMetadata)
        {
            scanImageMetadata();
            scanImagePosition();
            scanImageComments();
            scanImageCopyright();
            scanIPTCCore();
            scanTags();
        }
    }
}

bool lessThanForIdentity(const ItemScanInfo &a, const ItemScanInfo &b)
{
    if (a.status != b.status)
    {
        // First: sort by status

        // put UndefinedStatus to back
        if (a.status == DatabaseItem::UndefinedStatus)
            return false;
        // enum values are in the order we want it
        return a.status < b.status;
    }
    else
    {
        // Second: sort by modification date, descending
        return a.modificationDate > b.modificationDate;
    }
}

bool ImageScanner::scanFromIdenticalFile()
{
    // Get a list of other images that are identical. Source image shall not be included.
    QList<ItemScanInfo> candidates =
            DatabaseAccess().db()->getIdenticalFiles((int)m_fileInfo.size(), m_scanInfo.uniqueHash, m_scanInfo.id);

    if (!candidates.isEmpty())
    {
        // Sort by priority, as implemented by custom lessThan()
        qStableSort(candidates.begin(), candidates.end(), lessThanForIdentity);

        kDebug(50003) << "Recognized" << m_fileInfo.filePath() << "as identical to item" << candidates.first().id;

        // Copy attributes.
        // Todo for the future is to worry about syncing identical files.
        DatabaseAccess().db()->copyImageAttributes(candidates.first().id, m_scanInfo.id);
        return true;
    }
    return false;
}

bool ImageScanner::copyFromSource(qlonglong srcId)
{
    DatabaseAccess access;

    // some basic validity checking
    if (srcId == m_scanInfo.id)
        return false;
    ItemScanInfo info = access.db()->getItemScanInfo(srcId);
    if (!info.id)
        return false;

    kDebug(50003) << "Recognized" << m_fileInfo.filePath() << "as copied from" << srcId;
    access.db()->copyImageAttributes(srcId, m_scanInfo.id);
    return true;
}

void ImageScanner::updateHardInfos()
{
    updateImage();
    updateImageInformation();
}

void ImageScanner::scanImageInformation()
{
    MetadataFields fields;
    fields << MetadataInfo::Rating
           << MetadataInfo::CreationDate
           << MetadataInfo::DigitizationDate
           << MetadataInfo::Orientation;
    QVariantList metadataInfos = m_metadata.getMetadataFields(fields);
    QSize size = m_img.size();

    // creation date: fall back to file system property
    if (metadataInfos[1].isNull() || !metadataInfos[1].toDateTime().isValid())
    {
        metadataInfos[1] = m_fileInfo.created();
    }

    QVariantList infos;
    infos << metadataInfos
          << size.width()
          << size.height()
          << detectFormat()
          << m_img.originalBitDepth()
          << m_img.originalColorModel();

    DatabaseAccess().db()->addImageInformation(m_scanInfo.id, infos);
}

static bool hasValidField(const QVariantList &list)
{
    for (QVariantList::const_iterator it = list.begin();
         it != list.end(); ++it)
    {
        if (!(*it).isNull())
            return true;
    }
    return false;
}

void ImageScanner::updateImageInformation()
{
    QSize size = m_img.size();

    QVariantList infos;
    infos << size.width()
          << size.height()
          << detectFormat()
          << m_img.originalBitDepth()
          << m_img.originalColorModel();

    DatabaseAccess access;
    access.db()->changeImageInformation(m_scanInfo.id, infos,
                                                    DatabaseFields::Width
                                                    | DatabaseFields::Height
                                                    | DatabaseFields::Format
                                                    | DatabaseFields::ColorDepth
                                                    | DatabaseFields::ColorModel);
}

static MetadataFields allImageMetadataFields()
{
    // This list must reflect the order required by AlbumDB::addImageMetadata
    MetadataFields fields;
    fields << MetadataInfo::Make
           << MetadataInfo::Model
           << MetadataInfo::Lens
           << MetadataInfo::Aperture
           << MetadataInfo::FocalLength
           << MetadataInfo::FocalLengthIn35mm
           << MetadataInfo::ExposureTime
           << MetadataInfo::ExposureProgram
           << MetadataInfo::ExposureMode
           << MetadataInfo::Sensitivity
           << MetadataInfo::FlashMode
           << MetadataInfo::WhiteBalance
           << MetadataInfo::WhiteBalanceColorTemperature
           << MetadataInfo::MeteringMode
           << MetadataInfo::SubjectDistance
           << MetadataInfo::SubjectDistanceCategory;
    return fields;
}

void ImageScanner::scanImageMetadata()
{
    QVariantList metadataInfos = m_metadata.getMetadataFields(allImageMetadataFields());

    if (hasValidField(metadataInfos))
        DatabaseAccess().db()->addImageMetadata(m_scanInfo.id, metadataInfos);
}

void ImageScanner::scanImagePosition()
{
    // This list must reflect the order required by AlbumDB::addImagePosition
    MetadataFields fields;
    fields << MetadataInfo::Latitude
           << MetadataInfo::LatitudeNumber
           << MetadataInfo::Longitude
           << MetadataInfo::LongitudeNumber
           << MetadataInfo::Altitude
           << MetadataInfo::PositionOrientation
           << MetadataInfo::PositionTilt
           << MetadataInfo::PositionRoll
           << MetadataInfo::PositionAccuracy
           << MetadataInfo::PositionDescription;

    QVariantList metadataInfos = m_metadata.getMetadataFields(fields);

    if (hasValidField(metadataInfos))
        DatabaseAccess().db()->addImagePosition(m_scanInfo.id, metadataInfos);
}

void ImageScanner::scanImageComments()
{
    MetadataFields fields;
    fields << MetadataInfo::Comment
           << MetadataInfo::Description
           << MetadataInfo::Headline
           << MetadataInfo::Title
           << MetadataInfo::DescriptionWriter;

    QVariantList metadataInfos = m_metadata.getMetadataFields(fields);

    // note that this cannot be replaced with hasValidField, as the last is skipped
    bool noComment = (metadataInfos[0].isNull() && metadataInfos[1].isNull()
                      && metadataInfos[2].isNull() && metadataInfos[3].isNull());
    if (noComment)
        return;

    DatabaseAccess access;
    ImageComments comments(access, m_scanInfo.id);

    // Description
    if (!metadataInfos[1].isNull())
    {
        QString author = metadataInfos[4].toString(); // possibly null

        const QMap<QString, QVariant> &map = metadataInfos[1].toMap();

        for (QMap<QString, QVariant>::const_iterator it = map.begin();
             it != map.end(); ++it)
        {
            comments.addComment(it.value().toString(), it.key(), author);
            /*
             * Add here any handling of always adding one x-default comment, if needed
             * TODO: Store always one x-default value? Or is it ok to have only real language values? Then we will need
             * a global default language based on which x-default is chosen, when writing to image metadata
             * (not by this class!)
             */
        }
    }

    // old-style comment. Maybe overwrite x-default from above.
    if (!metadataInfos[0].isNull())
    {
        comments.addComment(metadataInfos[0].toString());
    }

    // Headline
    if (!metadataInfos[2].isNull())
    {
        comments.addHeadline(metadataInfos[2].toString());
    }

    // Title
    if (!metadataInfos[3].isNull())
    {
        comments.addTitle(metadataInfos[2].toString());
    }
}

void ImageScanner::scanImageCopyright()
{
    MetadataFields fields;
    fields << MetadataInfo::IptcCoreCopyrightNotice
           << MetadataInfo::IptcCoreCreator
           << MetadataInfo::IptcCoreProvider
           << MetadataInfo::IptcCoreRightsUsageTerms
           << MetadataInfo::IptcCoreSource
           << MetadataInfo::IptcCoreCreatorJobTitle
           << MetadataInfo::IptcCoreSource;

    QVariantList metadataInfos = m_metadata.getMetadataFields(fields);

    if (!hasValidField(metadataInfos))
        return;

    ImageCopyright copyright(m_scanInfo.id);

    if (!metadataInfos[0].isNull())
    {
        QMap<QString, QVariant> map = metadataInfos[0].toMap();
        // replace all entries for the first time, after that, add entries
        ImageCopyright::ReplaceMode mode = ImageCopyright::ReplaceAllEntries;
        for (QMap<QString, QVariant>::iterator it = map.begin(); it != map.end(); ++it)
        {
            copyright.setCopyrightNotice(it.value().toString(), it.key(), mode);
            mode = ImageCopyright::ReplaceLanguageEntry;
        }
    }
    if (!metadataInfos[1].isNull())
    {
        QList<QVariant> list = metadataInfos[1].toList();
        ImageCopyright::ReplaceMode mode = ImageCopyright::ReplaceAllEntries;
        foreach(const QVariant &var, list)
        {
            copyright.setCreator(var.toString(), mode);
            mode = ImageCopyright::AddEntryToExisting;
        }
    }
    if (!metadataInfos[2].isNull())
    {
        copyright.setProvider(metadataInfos[2].toString());
    }
    if (!metadataInfos[3].isNull())
    {
        QMap<QString, QVariant> map = metadataInfos[0].toMap();
        ImageCopyright::ReplaceMode mode = ImageCopyright::ReplaceAllEntries;
        for (QMap<QString, QVariant>::iterator it = map.begin(); it != map.end(); ++it)
        {
            copyright.setRightsUsageTerms(it.value().toString(), it.key(), mode);
            mode = ImageCopyright::ReplaceLanguageEntry;
        }
    }
    if (!metadataInfos[4].isNull())
    {
        copyright.setSource(metadataInfos[4].toString());
    }
    if (!metadataInfos[5].isNull())
    {
        copyright.setCreatorJobTitle(metadataInfos[5].toString());
    }
    if (!metadataInfos[6].isNull())
    {
        copyright.setInstructions(metadataInfos[6].toString());
    }
}

void ImageScanner::scanIPTCCore()
{
    MetadataFields fields;
    fields << MetadataInfo::IptcCoreCountryCode
           << MetadataInfo::IptcCoreCountry
           << MetadataInfo::IptcCoreCity
           << MetadataInfo::IptcCoreLocation
           << MetadataInfo::IptcCoreProvinceState
           << MetadataInfo::IptcCoreIntellectualGenre
           << MetadataInfo::IptcCoreJobID
           << MetadataInfo::IptcCoreScene
           << MetadataInfo::IptcCoreSubjectCode;

    QVariantList metadataInfos = m_metadata.getMetadataFields(fields);

    if (!hasValidField(metadataInfos))
        return;

    DatabaseAccess access;
    for (int i=0; i<metadataInfos.size(); i++)
    {
        const QVariant &var = metadataInfos[i];

        if (var.isNull())
            continue;

        // Data is of type String or StringList (Scene and SubjectCode)
        if (var.type() == QVariant::String)
        {
            access.db()->setImageProperty(m_scanInfo.id, iptcCorePropertyName(fields[i]), var.toString());
        }
        else if (var.type() == QVariant::StringList)
        {
            QStringList list = var.toStringList();
            QString property = iptcCorePropertyName(fields[i]);
            foreach(const QString &str, list)
                access.db()->setImageProperty(m_scanInfo.id, property, str);
        }
    }
}

void ImageScanner::scanTags()
{
    QVariant var = m_metadata.getMetadataField(MetadataInfo::Keywords);
    QStringList keywords = var.toStringList();
    if (!keywords.isEmpty())
    {
        DatabaseAccess access;
        // get tag ids, create if necessary
        QList<int> tagIds = access.db()->getTagsFromTagPaths(keywords, true);
        access.db()->addTagsToItems(QList<qlonglong>() << m_scanInfo.id, tagIds);
    }
}

void ImageScanner::loadFromDisk()
{
    m_hasMetadata = m_metadata.load(m_fileInfo.filePath());
    m_hasImage    = m_img.loadImageInfo(m_fileInfo.filePath(), false, false);
    // faster than loading twice from disk
    if (m_hasMetadata)
    {
        m_img.setComments(m_metadata.getComments());
        m_img.setExif(m_metadata.getExif());
        m_img.setIptc(m_metadata.getIptc());
        m_img.setXmp(m_metadata.getXmp());
    }
}

QString ImageScanner::detectFormat()
{
    DImg::FORMAT dimgFormat = m_img.fileFormat();
    switch (dimgFormat)
    {
        case DImg::JPEG:
            return "JPG";
        case DImg::PNG:
            return "PNG";
        case DImg::TIFF:
            return "TIFF";
        case DImg::PPM:
            return "PPM";
        case DImg::JP2K:
            return "JP2k";
        case DImg::RAW:
        {
            QString format = "RAW-";
            format += m_fileInfo.suffix().toUpper();
            return format;
        }
        case DImg::NONE:
        case DImg::QIMAGE:
        {
            KMimeType::Ptr mimetype = KMimeType::mimeType(m_fileInfo.path(), KMimeType::ResolveAliases);
            if (mimetype)
            {
                QString name = mimetype->name();
                if (name.startsWith("image/"))
                {
                    return name.mid(6).toUpper();
                }
            }
            else
                kWarning(50003) << "Detecting file format: KMimeType for" << m_fileInfo.path() << "is null" << endl;

            QByteArray format = QImageReader::imageFormat(m_fileInfo.fileName());
            if (!format.isEmpty())
            {
                return QString(format).toUpper();
            }
        }
    }
    return QString();
}

QString ImageScanner::formatToString(const QString &format)
{
    if (format == "JPG")
    {
        return "JPEG";
    }
    else if (format == "PNG")
    {
        return format;
    }
    else if (format == "TIFF")
    {
        return format;
    }
    else if (format == "PPM")
    {
        return format;
    }
    else if (format == "JP2K")
    {
        return "JPEG 2000";
    }
    else if (format.startsWith("RAW-"))
    {
        return i18nc("RAW image file (), the parentheses contain the file suffix, like MRW",
                     "RAW image file (%1)",
                     format.mid(4));
    }
    else
        return format;
}

QString ImageScanner::colorModelToString(DImg::COLORMODEL colorModel)
{
    switch (colorModel)
    {
        case DImg::RGB:
            return i18nc("Color Model: RGB", "RGB");
        case DImg::GRAYSCALE:
            return i18nc("Color Model: Grayscale", "Grayscale");
        case DImg::MONOCHROME:
            return i18nc("Color Model: Monochrome", "Monochrome");
        case DImg::INDEXED:
            return i18nc("Color Model: Indexed", "Indexed");
        case DImg::YCBCR:
            return i18nc("Color Model: YCbCr", "YCbCr");
        case DImg::CMYK:
            return i18nc("Color Model: CMYK", "CMYK");
        case DImg::CIELAB:
            return i18nc("Color Model: CIE L*a*b*", "CIE L*a*b*");
        case DImg::COLORMODELRAW:
            return i18nc("Color Model: Uncalibrated (RAW)", "Uncalibrated (RAW)");
        case DImg::COLORMODELUNKNOWN:
        default:
            return i18nc("Color Model: Unknown", "Unknown");
    }
}

QString ImageScanner::iptcCorePropertyName(MetadataInfo::Field field)
{
    // These strings are specified in DBSCHEMA.ods
    switch (field)
    {
        // copyright table
        case MetadataInfo::IptcCoreCopyrightNotice:
            return "copyrightNotice";
        case MetadataInfo::IptcCoreCreator:
            return "creator";
        case MetadataInfo::IptcCoreProvider:
            return "provider";
        case MetadataInfo::IptcCoreRightsUsageTerms:
            return "rightsUsageTerms";
        case MetadataInfo::IptcCoreSource:
            return "source";
        case MetadataInfo::IptcCoreCreatorJobTitle:
            return "creatorJobTitle";
        case MetadataInfo::IptcCoreInstructions:
            return "instructions";

        // ImageProperties table
        case MetadataInfo::IptcCoreCountryCode:
            return "countryCode";
        case MetadataInfo::IptcCoreCountry:
            return "country";
        case MetadataInfo::IptcCoreCity:
            return "city";
        case MetadataInfo::IptcCoreLocation:
            return "location";
        case MetadataInfo::IptcCoreProvinceState:
            return "provinceState";
        case MetadataInfo::IptcCoreIntellectualGenre:
            return "intellectualGenre";
        case MetadataInfo::IptcCoreJobID:
            return "jobId";
        case MetadataInfo::IptcCoreScene:
            return "scene";
        case MetadataInfo::IptcCoreSubjectCode:
            return "subjectCode";
        default:
            return QString();
    }
}

void ImageScanner::fillCommonContainer(qlonglong imageid, ImageCommonContainer *container)
{
    QVariantList imagesFields;
    QVariantList imageInformationFields;

    {
        DatabaseAccess access;
        imagesFields = access.db()->getImagesFields(imageid,
                                           DatabaseFields::Name |
                                           DatabaseFields::ModificationDate |
                                           DatabaseFields::FileSize
                                                   );
        imageInformationFields = access.db()->getImageInformation(imageid,
                                           DatabaseFields::Rating |
                                           DatabaseFields::CreationDate |
                                           DatabaseFields::DigitizationDate |
                                           DatabaseFields::Orientation |
                                           DatabaseFields::Width |
                                           DatabaseFields::Height |
                                           DatabaseFields::Format |
                                           DatabaseFields::ColorDepth |
                                           DatabaseFields::ColorModel
                                                                 );
    }

    if (!imagesFields.isEmpty())
    {
        container->fileName             = imagesFields[0].toString();
        container->fileModificationDate = imagesFields[1].toDateTime();
        container->fileSize             = imagesFields[2].toInt();
    }

    if (!imageInformationFields.isEmpty())
    {
        container->rating           = imageInformationFields[0].toInt();
        container->creationDate     = imageInformationFields[1].toDateTime();
        container->digitizationDate = imageInformationFields[2].toDateTime();
        container->orientation      = DMetadata::valueToString(imageInformationFields[3], MetadataInfo::Orientation);
        container->width            = imageInformationFields[4].toInt();
        container->height           = imageInformationFields[5].toInt();
        container->format           = formatToString(imageInformationFields[6].toString());
        container->colorDepth       = imageInformationFields[7].toInt();
        container->colorModel       = colorModelToString((DImg::COLORMODEL)imageInformationFields[8].toInt());
    }
}

void ImageScanner::fillMetadataContainer(qlonglong imageid, ImageMetadataContainer *container)
{
    // read from database
    QVariantList fields = DatabaseAccess().db()->getImageMetadata(imageid);

    // check we have at least one valid field
    container->allFieldsNull = !hasValidField(fields);

    if (container->allFieldsNull)
        return;

    // DMetadata does all translation work
    QStringList strings = DMetadata::valuesToString(fields, allImageMetadataFields());

    // associate with hard-coded variables
    container->make             = strings[0];
    container->model            = strings[1];
    container->lens             = strings[2];
    container->aperture         = strings[3];
    container->focalLength      = strings[4];
    container->focalLength35    = strings[5];
    container->exposureTime     = strings[6];
    container->exposureProgram  = strings[7];
    container->exposureMode     = strings[8];
    container->sensitivity      = strings[9];
    container->flashMode        = strings[10];
    container->whiteBalance     = strings[11];
    container->whiteBalanceColorTemperature
                               = strings[12];
    container->meteringMode     = strings[13];
    container->subjectDistance  = strings[14];
    container->subjectDistanceCategory
                               = strings[15];
}


}
