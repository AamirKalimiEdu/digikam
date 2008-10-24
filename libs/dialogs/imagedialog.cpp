/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-13
 * Description : image files selector dialog.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagedialog.h"
#include "imagedialog.moc"

// Qt includes.

#include <QLabel>
#include <QVBoxLayout>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kimageio.h>

// LibKDcraw includes.

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000400
#include <libkdcraw/dcrawbinary.h>
#endif

// Digikam includes.

#include "dmetadata.h"
#include "loadingdescription.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class ImageDialogPreviewPrivate
{

public:

    ImageDialogPreviewPrivate()
    {
        imageLabel      = 0;
        infoLabel       = 0;
        thumbLoadThread = 0;
    }

    QLabel               *imageLabel;
    QLabel               *infoLabel;

    KUrl                  currentURL;

    DMetadata             metaIface;

    ThumbnailLoadThread *thumbLoadThread;
};

ImageDialogPreview::ImageDialogPreview(QWidget *parent)
                  : KPreviewWidgetBase(parent)
{
    d = new ImageDialogPreviewPrivate;

    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();

    QVBoxLayout *vlay = new QVBoxLayout(this);
    d->imageLabel     = new QLabel(this);
    d->imageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->imageLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    d->infoLabel = new QLabel(this);

    vlay->setMargin(0);
    vlay->setSpacing(KDialog::spacingHint());
    vlay->addWidget(d->imageLabel);
    vlay->addWidget(d->infoLabel);

    setSupportedMimeTypes(KImageIO::mimeTypes());

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotThumbnail(const LoadingDescription&, const QPixmap&)));
}

ImageDialogPreview::~ImageDialogPreview()
{
    delete d;
}

QSize ImageDialogPreview::sizeHint() const
{
    return QSize(256, 256);
}

void ImageDialogPreview::resizeEvent(QResizeEvent *)
{
    QMetaObject::invokeMethod(this, "showPreview", Qt::QueuedConnection);
}

void ImageDialogPreview::showPreview()
{
    KUrl url(d->currentURL);
    clearPreview();
    showPreview(url);
}

void ImageDialogPreview::showPreview(const KUrl& url)
{
    if (!url.isValid())
    {
        clearPreview();
        return;
    }

    if (url != d->currentURL)
    {
        clearPreview();
        d->currentURL = url;
        d->thumbLoadThread->find(d->currentURL.path());

        d->metaIface.load(d->currentURL.path());
        PhotoInfoContainer info = d->metaIface.getPhotographInformations();
        if (!info.isEmpty())
        {
            QString identify;
            QString make, model, dateTime, aperture, focalLength, exposureTime, sensitivity;
            QString unavailable(i18n("<i>unavailable</i>"));
            QString cellBeg("<tr><td><nobr><font size=-1>");
            QString cellMid("</font></nobr></td><td><nobr><font size=-1>");
            QString cellEnd("</font></nobr></td></tr>");

            if (info.make.isEmpty()) make = unavailable;
            else make = info.make;

            if (info.model.isEmpty()) model = unavailable;
            else model = info.model;

            if (!info.dateTime.isValid()) dateTime = unavailable;
            else dateTime = KGlobal::locale()->formatDateTime(info.dateTime, KLocale::ShortDate, true);

            if (info.aperture.isEmpty()) aperture = unavailable;
            else aperture = info.aperture;

            if (info.focalLength.isEmpty()) focalLength = unavailable;
            else focalLength = info.focalLength;

            if (info.exposureTime.isEmpty()) exposureTime = unavailable;
            else exposureTime = info.exposureTime;

            if (info.sensitivity.isEmpty()) sensitivity = unavailable;
            else sensitivity = i18n("%1 ISO", info.sensitivity);

            identify = "<table cellspacing=0 cellpadding=0>";
            identify += cellBeg + i18n("Make:")        + cellMid + make         + cellEnd;
            identify += cellBeg + i18n("Model:")       + cellMid + model        + cellEnd;
            identify += cellBeg + i18n("Created:")     + cellMid + dateTime     + cellEnd;
            identify += cellBeg + i18n("Aperture:")    + cellMid + aperture     + cellEnd;
            identify += cellBeg + i18n("Focal:")       + cellMid + focalLength  + cellEnd;
            identify += cellBeg + i18n("Exposure:")    + cellMid + exposureTime + cellEnd;
            identify += cellBeg + i18n("Sensitivity:") + cellMid + sensitivity  + cellEnd;
            identify += "</table>";

            d->infoLabel->setText(identify);
        }
        else
            d->infoLabel->clear();
    }
}

void ImageDialogPreview::slotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    if (KUrl(desc.filePath) == d->currentURL)
    {
        QPixmap pixmap;
        QSize s = d->imageLabel->contentsRect().size();

        if (s.width() < pix.width() || s.height() < pix.height())
            pixmap = pix.scaled(s, Qt::KeepAspectRatio);
        else
            pixmap = pix;

        d->imageLabel->setPixmap(pixmap);
    }
}

void ImageDialogPreview::clearPreview()
{
    d->imageLabel->clear();
    d->infoLabel->clear();
    d->currentURL = KUrl();
}

// ------------------------------------------------------------------------

class ImageDialogPrivate
{

public:

    ImageDialogPrivate()
    {
        singleSelect = false;
    }

    bool       singleSelect;

    QString    fileFormats;

    KUrl       url;
    KUrl::List urls;
};

ImageDialog::ImageDialog(QWidget* parent, const KUrl& url, bool singleSelect, const QString& caption)
{
    d = new ImageDialogPrivate;
    d->singleSelect = singleSelect;

    QStringList patternList = KImageIO::pattern(KImageIO::Reading).split('\n', QString::SkipEmptyParts);

    // All Images from list must been always the first entry given by KDE API
    QString allPictures = patternList[0];

#if KDCRAW_VERSION < 0x000400
    // Add other files format witch are missing to All Images" type mime provided by KDE and replace current.
    if (KDcrawIface::DcrawBinary::instance()->versionIsRight())
    {
        allPictures.insert(allPictures.indexOf("|"), QString(KDcrawIface::DcrawBinary::instance()->rawFiles()) + QString(" *.JPE *.TIF"));
        patternList.removeAll(patternList[0]);
        patternList.prepend(allPictures);
        // Added RAW file formats supported by dcraw program like a type mime.
        // Note: we cannot use here "image/x-raw" type mime from KDE because it is incomplete
        // or unavailable(see file #121242 in B.K.O).
        patternList.append(i18n("\n%1|Camera RAW files", QString(KDcrawIface::DcrawBinary::instance()->rawFiles())));
    }
#else
    allPictures.insert(allPictures.indexOf("|"), QString(KDcrawIface::KDcraw::rawFiles()) + QString(" *.JPE *.TIF"));
    patternList.removeAll(patternList[0]);
    patternList.prepend(allPictures);
    // Added RAW file formats supported by dcraw program like a type mime.
    // Note: we cannot use here "image/x-raw" type mime from KDE because it is incomplete
    // or unavailable(see file #121242 in B.K.O).
    patternList.append(i18n("\n%1|Camera RAW files", QString(KDcrawIface::KDcraw::rawFiles())));
#endif

    d->fileFormats = patternList.join("\n");

    kDebug(50003) << "file formats=" << d->fileFormats << endl;

    KFileDialog dlg(url, d->fileFormats, parent);
    ImageDialogPreview *preview = new ImageDialogPreview(&dlg);
    dlg.setPreviewWidget(preview);
    dlg.setOperationMode(KFileDialog::Opening);

    if (d->singleSelect)
    {
        dlg.setMode(KFile::File);
        if (caption.isEmpty()) dlg.setCaption(i18n("Select an Image"));
        else dlg.setWindowTitle(caption);
        dlg.exec();
        d->url = dlg.selectedUrl();
    }
    else
    {
        dlg.setMode(KFile::Files);
        if (caption.isEmpty()) dlg.setCaption(i18n("Select Images"));
        else dlg.setWindowTitle(caption);
        dlg.exec();
        d->urls = dlg.selectedUrls();
    }
}

ImageDialog::~ImageDialog()
{
    delete d;
}

bool ImageDialog::singleSelect() const
{
    return d->singleSelect;
}

QString ImageDialog::fileFormats() const
{
    return d->fileFormats;
}

KUrl ImageDialog::url() const
{
    return d->url;
}

KUrl::List ImageDialog::urls() const
{
    return d->urls;
}

KUrl::List ImageDialog::getImageURLs(QWidget* parent, const KUrl& url, const QString& caption)
{
    ImageDialog dlg(parent, url, false, caption);
    if (!dlg.urls().isEmpty())
        return dlg.urls();
    else
        return KUrl::List();
}


KUrl ImageDialog::getImageURL(QWidget* parent, const KUrl& url, const QString& caption)
{
    ImageDialog dlg(parent, url, true, caption);
    if (dlg.url() != KUrl())
        return dlg.url();
    else
        return KUrl();
}

} // namespace Digikam
