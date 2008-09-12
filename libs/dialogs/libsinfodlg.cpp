/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : shared libraries list dialog
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

// Qt includes.

#include <QStringList>
#include <QString>
#include <QLabel>
#include <QLayout>
#include <QGridLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMimeData>
#include <QClipboard>

// KDE includes.

#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>

#include "config-digikam.h"
#ifdef HAVE_MARBLEWIDGET
#include <marble/global.h>
using namespace Marble;
#endif // HAVE_MARBLEWIDGET

// Libkexiv2 includes.

#include <libkexiv2/version.h>
#include <libkexiv2/kexiv2.h>

// Libkdcraw includes.

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000400
#include <libkdcraw/dcrawbinary.h>
#endif

// C ANSI includes

extern "C"
{
#include <png.h>
#include <tiffvers.h>
#include <jpeglib.h>
#include <jasper/jas_version.h>
#include <lcms.h>
}

// Local includes.

#include "daboutdata.h"
#include "greycstorationiface.h"
#include "libsinfodlg.h"
#include "libsinfodlg.moc"

namespace Digikam
{

class LibsInfoDlgPriv
{
public:

    LibsInfoDlgPriv()
    {
        listView = 0;
    }

    QTreeWidget *listView;
};

LibsInfoDlg::LibsInfoDlg(QWidget *parent)
           : KDialog(parent)
{
    setButtons(Help|User1|Ok);
    setDefaultButton(Ok);
    setModal(true);
    setHelp("digikam");
    setCaption(i18n("Shared Libraries and Components Information"));
    setButtonText(User1, i18n("Copy to Clipboard"));

    d = new LibsInfoDlgPriv;

    QWidget *page = new QWidget(this);
    setMainWidget(page);
    QGridLayout* grid = new QGridLayout(page);

    // --------------------------------------------------------

    QLabel *logo = new QLabel(page);
    if (KGlobal::mainComponent().aboutData()->appName() == QString("digikam"))
        logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                                .scaled(92, 92, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else
        logo->setPixmap(QPixmap(KStandardDirs::locate("data", "showfoto/data/logo-showfoto.png"))
                                .scaled(92, 92, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // --------------------------------------------------------

    QLabel *header = new QLabel(page);
    header->setWordWrap(true);
    header->setText(i18n("<font size=\"5\">%1</font><br><b>Version %2</b>"
                         "<p>%3</p>",
                    KGlobal::mainComponent().aboutData()->programName(),
                    KGlobal::mainComponent().aboutData()->version(),
                    digiKamSlogan().toString()));

    // --------------------------------------------------------

    d->listView = new QTreeWidget(page);
    d->listView->setSortingEnabled(false);
    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setColumnCount(2);
    d->listView->setHeaderLabels(QStringList() << i18n("Component") << i18n("Info"));
    d->listView->header()->setResizeMode(QHeaderView::Stretch);

    // --------------------------------------------------------

    grid->addWidget(logo,        0, 0, 1, 1);
    grid->addWidget(header,      0, 1, 1, 2);
    grid->addWidget(d->listView, 1, 0, 1, 2);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(1, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(this, SIGNAL(user1Clicked()),
            this, SLOT(slotCopy2ClipBoard()));

    resize(400, 500);

    // --------------------------------------------------------
    // By default set a list of common components information used by Showfoto and digiKam.

    QMap<QString, QString> list;
    list.insert(i18n("LibQt"),                      qVersion());
    list.insert(i18n("LibKDE"),                     KDE::versionString());
    list.insert(i18n("LibKdcraw"),                  KDcrawIface::KDcraw::version());
#if KDCRAW_VERSION < 0x000400
    list.insert(i18n("Dcraw program"),              KDcrawIface::DcrawBinary::internalVersion());
#else
    list.insert(i18n("LibRaw"),                     KDcrawIface::KDcraw::librawVersion());
#endif
    list.insert(i18n("LibKExiv2"),                  KExiv2Iface::KExiv2::version());
    list.insert(i18n("LibExiv2"),                   KExiv2Iface::KExiv2::Exiv2Version());
    list.insert(i18n("Exiv2 support XMP metadata"), KExiv2Iface::KExiv2::supportXmp() ?
                                                    i18n("Yes") : i18n("No"));
#if KEXIV2_VERSION >= 0x000300
    list.insert(i18n("Exiv2 can write to Jpeg"),    KExiv2Iface::KExiv2::supportMetadataWritting("image/jpeg") ?
                                                    i18n("Yes") : i18n("No"));
    list.insert(i18n("Exiv2 can write to Tiff"),    KExiv2Iface::KExiv2::supportMetadataWritting("image/tiff") ?
                                                    i18n("Yes") : i18n("No"));
    list.insert(i18n("Exiv2 can write to Png"),     KExiv2Iface::KExiv2::supportMetadataWritting("image/png") ?
                                                    i18n("Yes") : i18n("No"));
    list.insert(i18n("Exiv2 can write to Jp2"),     KExiv2Iface::KExiv2::supportMetadataWritting("image/jp2") ?
                                                    i18n("Yes") : i18n("No"));
#endif

    list.insert(i18n("LibPNG"),                     QString(PNG_LIBPNG_VER_STRING));
    list.insert(i18n("LibTIFF"),                    QString(TIFFLIB_VERSION_STR).replace('\n', ' '));
    list.insert(i18n("LibJPEG"),                    QString::number(JPEG_LIB_VERSION));
    list.insert(i18n("LibJasper"),                  QString(jas_getversion()));
    list.insert(i18n("LibCImg"),                    GreycstorationIface::cimgVersionString());
    list.insert(i18n("LibLCMS"),                    QString::number(LCMS_VERSION));

#ifdef HAVE_MARBLEWIDGET
    list.insert(i18n("Marble widget"),              QString(MARBLE_VERSION_STRING));
#endif //HAVE_MARBLEWIDGET

    setComponentsInfoMap(list);
}

LibsInfoDlg::~LibsInfoDlg()
{
    delete d;
}

void LibsInfoDlg::setComponentsInfoMap(const QMap<QString, QString>& list)
{
    for (QMap<QString, QString>::const_iterator it = list.begin() ; it != list.end() ; ++it)
        new QTreeWidgetItem(d->listView, QStringList() << it.key() << it.value());
}

void LibsInfoDlg::slotCopy2ClipBoard()
{
    QString textInfo;

    textInfo.append(KGlobal::mainComponent().aboutData()->programName());
    textInfo.append(" version ");
    textInfo.append(KGlobal::mainComponent().aboutData()->version());
    textInfo.append("\n");

    QTreeWidgetItemIterator it(d->listView);
    while (*it)
    {
        textInfo.append((*it)->text(0));
        textInfo.append(": ");
        textInfo.append((*it)->text(1));
        textInfo.append("\n");
        ++it;
    }

    QMimeData *mimeData = new QMimeData();
    mimeData->setText(textInfo);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

}  // NameSpace Digikam
