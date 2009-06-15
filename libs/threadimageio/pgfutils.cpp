/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : PGF utils.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "pgfutils.h"

// C Ansi includes

extern "C"
{
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
}

// Qt includes

#include <QImage>
#include <QByteArray>
#include <QFile>

// KDE includes

#include <kdebug.h>

// LibPGF includes

#include "PGFimage.h"

namespace Digikam
{

bool readPGFImageData(const QByteArray& data, QImage& img)
{
    try
    {
        CPGFMemoryStream stream((UINT8*)data.data(), (size_t)data.size());
        CPGFImage        pgfImg;
        pgfImg.Open(&stream);

        if (pgfImg.Channels() != 4)
        {
            kDebug(50003) << "PGF channels not supported";
            return false;
        }

        img = QImage(pgfImg.Width(), pgfImg.Height(), QImage::Format_ARGB32);
        pgfImg.Read();
        pgfImg.GetBitmap(img.bytesPerLine(), (UINT8*)img.bits(), img.depth());
    }
    catch(IOException& e)
    {
        int err = e.error;

        if (err >= AppError) err -= AppError;
        kDebug(50003) << "Error running libpgf (" << err << ")!";
        return false;
    }

    return true;
}

bool writePGFImageData(const QImage& img, QByteArray& data, int quality)
{
    try
    {
        if (img.isNull())
        {
            kDebug(50003) << "Thumb image is null";
            return false;
        }

        // No need Alpha to optimize space on DB.
        if (img.format() != QImage::Format_ARGB32)
            img.convertToFormat(QImage::Format_ARGB32);

        CPGFImage pgfImg;

        PGFHeader header;
        header.width    = img.width();
        header.height   = img.height();
        header.bpp      = img.depth();
        header.channels = 4;
        header.quality  = quality;
        header.mode     = ImageModeRGBA;
        header.background.rgbtBlue = header.background.rgbtGreen = header.background.rgbtRed = 0;
        pgfImg.SetHeader(header);
        pgfImg.ImportBitmap(img.bytesPerLine(), (UINT8*)img.bits(), img.depth());

        // TODO : optimize memory allocation...
        CPGFMemoryStream stream(256000);
        UINT32 nWrittenBytes = 0;
        pgfImg.Write(&stream, 0, NULL, &nWrittenBytes);

        data = QByteArray((const char*)stream.GetBuffer(), nWrittenBytes);
    }
    catch(IOException& e)
    {
        int err = e.error;

        if (err >= AppError) err -= AppError;
        kDebug(50003) << "Error running libpgf (" << err << ")!";
        return false;
    }

    return true;
}

bool loadPGFScaled(QImage& img, const QString& path, int maximumSize)
{
    FILE *file = fopen(QFile::encodeName(path), "rb");
    if (!file)
    {
        kDebug(50003) << "Error: Could not open source file.";
        return false;
    }

    unsigned char header[3];

    if (fread(&header, 3, 1, file) != 1)
    {
        fclose(file);
        return false;
    }

    unsigned char pgfID[3] = { 0x50, 0x47, 0x46 };

    if (memcmp(&header[0], &pgfID, 3) != 0)
    {
        // not a PGF file
        fclose(file);
        return false;
    }

    fclose(file);

    // -------------------------------------------------------------------
    // Initialize PGF API.

#ifdef WIN32
#ifdef Q_CC_MSVC
    HANDLE fd = CreateFile((LPCWSTR)(QFile::encodeName(path).constData()), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#else /* MinGW compiler. */
    HANDLE fd = CreateFile(QFile::encodeName(path), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#endif
	if (fd == INVALID_HANDLE_VALUE)
        return false;
#else
    int fd = open(QFile::encodeName(path), O_RDONLY);
    if (fd == -1)
        return false;
#endif

    try
    {
        CPGFFileStream stream(fd);
        CPGFImage      pgf;
        pgf.Open(&stream);

        // Try to find the right PGF level to get reduced image accordingly
        // with preview size wanted.
        int i=0;

        if (pgf.Levels() > 0)
        {
            for (i=pgf.Levels()-1 ; i>=0 ; --i)
            {
                if (qMin((int)pgf.Width(i), (int)pgf.Height(i)) >= maximumSize)
                    break;
            }
        }

        if (i<0) i=0;

        pgf.Read(i);  // Read PGF image at reduced level i.
        int map[] = { 0, 1, 2 };
        img = QImage(pgf.Width(i), pgf.Height(i), QImage::Format_RGB32);

/*
        const PGFHeader* header = pgf.GetHeader();
        kDebug(50003) << "PGF width    = " << header->width;
        kDebug(50003) << "PGF height   = " << header->height;
        kDebug(50003) << "PGF bbp      = " << header->bpp;
        kDebug(50003) << "PGF channels = " << header->channels;
        kDebug(50003) << "PGF quality  = " << header->quality;
        kDebug(50003) << "PGF mode     = " << header->mode;
        kDebug(50003) << "PGF levels   = " << header->nLevels;
        kDebug(50003) << "Level (w x h)= " << i << "(" << pgf.Width(i)
                      << " x " << pgf.Height(i) << ")" << endl;
        kDebug(50003) << "QImage depth = " << img.depth();
*/

        pgf.GetBitmap(img.bytesPerLine(), (UINT8*)img.bits(), img.depth(), map);
    }
    catch(IOException& e)
    {
        int err = e.error;

        if (err >= AppError) err -= AppError;
        kDebug(50003) << "Error running libpgf (" << err << ")!";
        return false;
    }

    return true;
}

QString libPGFVersion()
{
    return (QString(PGFCodecVersion));
}

}  // namespace Digikam
