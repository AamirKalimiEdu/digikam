/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : Camera type selection dialog
 * 
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGridLayout>

// KDE includes.

#include <k3activelabel.h>
#include <k3listview.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kurlrequester.h>
#include <klocale.h>
#include <klineedit.h>
#include <kcursor.h>
#include <kvbox.h>
#include <kapplication.h>

// Local includes.

#include "gpiface.h"
#include "cameraselection.h"
#include "cameraselection.moc"

namespace Digikam
{

class CameraSelectionPriv
{
public:

    CameraSelectionPriv()
    {
        listView         = 0;
        titleEdit        = 0;
        portButtonGroup  = 0;
        usbButton        = 0;
        serialButton     = 0;
        portPathLabel    = 0;
        portPathComboBox = 0;
        umsMountURL      = 0;
    }

    QButtonGroup  *portButtonGroup;

    QRadioButton  *usbButton;
    QRadioButton  *serialButton;

    QLabel        *portPathLabel;

    QComboBox     *portPathComboBox;

    QString        UMSCameraNameActual;
    QString        UMSCameraNameShown;
    QString        PTPCameraNameShown;

    QStringList    serialPortList;

    KLineEdit     *titleEdit;

    K3ListView    *listView;

    KUrlRequester *umsMountURL;
};

CameraSelection::CameraSelection( QWidget* parent )
               : KDialog(parent)
{
    d = new CameraSelectionPriv;

    kapp->setOverrideCursor( Qt::WaitCursor );
    setHelp("cameraselection.anchor", "digikam");
    setCaption(i18n("Camera Configuration"));
    setButtons(KDialog::Help|KDialog::Ok|KDialog::Cancel);
    setDefaultButton(KDialog::Ok);
    setModal(true);

    d->UMSCameraNameActual = QString("Directory Browse");   // Don't be i18n!
    d->UMSCameraNameShown  = i18n("Mounted Camera");
    d->PTPCameraNameShown  = QString("USB PTP Class Camera");

    setMainWidget(new QWidget(this));

    QGridLayout* mainBoxLayout = new QGridLayout(mainWidget());

    // --------------------------------------------------------------

    d->listView = new K3ListView(mainWidget());
    d->listView->addColumn( i18n("Camera List") );
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setResizeMode(K3ListView::LastColumn);
    d->listView->setMinimumWidth(350);
    d->listView->setWhatsThis( i18n("<p>Select here the camera name that you want to use. All "
                                    "default settings on the right panel "
                                    "will be set automatically.</p><p>This list has been generated "
                                    "using the gphoto2 library installed on your computer.</p>"));

    // --------------------------------------------------------------

    QGroupBox* titleBox   = new QGroupBox(i18n("Camera Title"), mainWidget());
    QVBoxLayout *gLayout1 = new QVBoxLayout(titleBox);
    d->titleEdit          = new KLineEdit(titleBox);
    d->titleEdit->setWhatsThis( i18n("<p>Set here the name used in digiKam interface to "
                                     "identify this camera.</p>"));

    gLayout1->addWidget(d->titleEdit);
    gLayout1->setMargin(KDialog::spacingHint());
    gLayout1->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------------

    QGroupBox* portBox    = new QGroupBox(i18n("Camera Port Type"), mainWidget());
    QVBoxLayout *gLayout2 = new QVBoxLayout(portBox);
    d->portButtonGroup    = new QButtonGroup( portBox );
    d->portButtonGroup->setExclusive( true );

    d->usbButton = new QRadioButton( i18n("USB"), portBox );
    d->usbButton->setWhatsThis( i18n("<p>Select this option if your camera is connected to your "
                                     "computer using an USB cable.</p>"));

    d->serialButton = new QRadioButton( i18n("Serial"), portBox );
    d->serialButton->setWhatsThis( i18n("<p>Select this option if your camera is connected to your "
                                        "computer using a serial cable.</p>"));

    d->portButtonGroup->addButton(d->usbButton);
    d->portButtonGroup->addButton(d->serialButton);

    gLayout2->addWidget(d->usbButton);
    gLayout2->addWidget(d->serialButton);
    gLayout2->setMargin(KDialog::spacingHint());
    gLayout2->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------------

    QGroupBox* portPathBox = new QGroupBox(i18n("Camera Port Path"), mainWidget());
    QVBoxLayout *gLayout3  = new QVBoxLayout(portPathBox);

    d->portPathLabel = new QLabel( portPathBox);
    d->portPathLabel->setText( i18n( "Note: only for serial port camera" ) );

    d->portPathComboBox = new QComboBox( portPathBox );
    d->portPathComboBox->setDuplicatesEnabled( false );
    d->portPathComboBox->setWhatsThis( i18n("<p>Select here the serial port to use on your computer. "
                     "This option is only required if you use a serial camera.</p>"));

    gLayout3->addWidget(d->portPathLabel);
    gLayout3->addWidget(d->portPathComboBox);
    gLayout3->setMargin(KDialog::spacingHint());
    gLayout3->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------------

    QGroupBox* umsMountBox = new QGroupBox(i18n("Camera Mount Path"), mainWidget());
    QVBoxLayout *gLayout4  = new QVBoxLayout(umsMountBox);

    QLabel* umsMountLabel = new QLabel( umsMountBox );
    umsMountLabel->setText( i18n( "Note: only for USB/IEEE mass storage camera" ) );

    d->umsMountURL = new KUrlRequester( QString("/mnt/camera"), umsMountBox);
    d->umsMountURL->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
    d->umsMountURL->setWhatsThis( i18n("<p>Set here the mount path to use on your computer. This "
                                       "option is only required if you use an <b>USB Mass Storage</b> "
                                       "camera.</p>"));

    gLayout4->addWidget(umsMountLabel);
    gLayout4->addWidget(d->umsMountURL);
    gLayout4->setMargin(KDialog::spacingHint());
    gLayout4->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------------
    
    QWidget* box2         = new QWidget(mainWidget());
    QGridLayout* gLayout5 = new QGridLayout(box2);

    QLabel* logo = new QLabel(box2);

    KIconLoader* iconLoader = KIconLoader::global();
    logo->setPixmap(iconLoader->loadIcon("digikam", K3Icon::NoGroup, 64, 
                    K3Icon::DefaultState, QStringList(),0, true));

    K3ActiveLabel* link = new K3ActiveLabel(box2);
    link->setText(i18n("<p>To set an <b>USB Mass Storage</b> camera<br>"
                       "(which appears like a removable drive), please<br>"
                       "use <a href=\"umscamera\">%1</a> from camera list.</p>", 
                       d->UMSCameraNameShown));

    K3ActiveLabel* link2 = new K3ActiveLabel(box2);
    link2->setText(i18n("<p>To set a <b>Generic PTP USB Device</b><br>"
                        "(which use Picture Transfer Protocol), please<br>"
                        "use <a href=\"ptpcamera\">%1</a> from camera list.</p>",
                        d->PTPCameraNameShown));

    K3ActiveLabel* explanation = new K3ActiveLabel(box2);
    explanation->setText(i18n("<p>A complete list of camera settings to use is<br>"
                 "available at <a href='http://www.teaser.fr/~hfiguiere/linux/digicam.html'>"
                 "this url</a>.</p>"));

    gLayout5->setMargin(KDialog::spacingHint());
    gLayout5->setSpacing(KDialog::spacingHint());
    gLayout5->addWidget(logo, 0, 0, 1, 1);
    gLayout5->addWidget(link, 0, 1, 2, 1);
    gLayout5->addWidget(link2, 2, 1, 3- 2+1, 1);
    gLayout5->addWidget(explanation, 4, 1, 5- 4+1, 1);

    // --------------------------------------------------------------

    mainBoxLayout->setMargin(0);
    mainBoxLayout->setSpacing(KDialog::spacingHint());
    mainBoxLayout->setColumnStretch(0, 10);
    mainBoxLayout->setRowStretch(6, 10);
    mainBoxLayout->addWidget(d->listView, 0, 0, 6+1, 1);
    mainBoxLayout->addWidget(titleBox, 0, 1, 1, 1);
    mainBoxLayout->addWidget(portBox, 1, 1, 1, 1);
    mainBoxLayout->addWidget(portPathBox, 2, 1, 1, 1);
    mainBoxLayout->addWidget(umsMountBox, 3, 1, 1, 1);
    mainBoxLayout->addWidget(box2, 4, 1, 5- 4+1, 1);

    // Connections --------------------------------------------------

    disconnect(link, SIGNAL(linkClicked(const QString &)),
               link, SLOT(openLink(const QString &)));

    connect(link, SIGNAL(linkClicked(const QString &)),
            this, SLOT(slotUMSCameraLinkUsed()));

    disconnect(link2, SIGNAL(linkClicked(const QString &)),
               link2, SLOT(openLink(const QString &)));

    connect(link2, SIGNAL(linkClicked(const QString &)),
            this, SLOT(slotPTPCameraLinkUsed()));

    connect(d->listView, SIGNAL(selectionChanged(Q3ListViewItem *)),
            this, SLOT(slotSelectionChanged(Q3ListViewItem *)));

    connect(d->portButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(slotPortChanged()));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOkClicked()));

    // Initialize  --------------------------------------------------

    getCameraList();
    getSerialPortList();
    kapp->restoreOverrideCursor();
}

CameraSelection::~CameraSelection()
{
    delete d;
}

void CameraSelection::slotUMSCameraLinkUsed()
{
    Q3ListViewItem *item = d->listView->findItem(d->UMSCameraNameShown, 0);
    if (item)
    {
        d->listView->setCurrentItem(item);
        d->listView->ensureItemVisible(item);
    }
}

void CameraSelection::slotPTPCameraLinkUsed()
{
    Q3ListViewItem *item = d->listView->findItem(d->PTPCameraNameShown, 0);
    if (item)
    {
        d->listView->setCurrentItem(item);
        d->listView->ensureItemVisible(item);
    }
}

void CameraSelection::setCamera(const QString& title, const QString& model,
                                const QString& port, const QString& path)
{
    QString camModel(model);

    if (camModel == d->UMSCameraNameActual)
        camModel = d->UMSCameraNameShown;

    Q3ListViewItem* item = d->listView->findItem(camModel, 0);
    if (!item) return;

    d->listView->setSelected(item, true);
    d->listView->ensureItemVisible(item);
    
    d->titleEdit->setText(title);

    if (port.contains("usb"))
    {
        d->usbButton->setChecked(true);
    }
    else if (port.contains("serial")) 
    {
        d->serialButton->setChecked(true);

        for (int i=0 ; i < d->portPathComboBox->count() ; i++) 
        {
            if (port == d->portPathComboBox->itemText(i)) 
            {
                d->portPathComboBox->setCurrentIndex(i);
                break;
            }
        }
    }

    d->umsMountURL->setUrl(path);
}

void CameraSelection::getCameraList()
{
    int count = 0;
    QStringList clist;
    QString cname;
    
    GPIface::getSupportedCameras(count, clist);
    
    for (int i = 0 ; i < count ; i++) 
    {
        cname = clist[i];
        if (cname == d->UMSCameraNameActual)
            new K3ListViewItem(d->listView, d->UMSCameraNameShown);
        else
            new K3ListViewItem(d->listView, cname);
    }
}

void CameraSelection::getSerialPortList()
{
    QStringList plist;

    GPIface::getSupportedPorts(plist);

    d->serialPortList.clear();
    
    for (int i = 0; i < plist.count() ; i++) 
    {
        if ((plist[i]).startsWith("serial:"))
            d->serialPortList.append(plist[i]);
    }
}

void CameraSelection::slotSelectionChanged(Q3ListViewItem *item)
{
    if (!item) return;

    QString model(item->text(0));
    
    if (model == d->UMSCameraNameShown) 
    {
        model = d->UMSCameraNameActual;

        d->titleEdit->setText(model);
        
        d->serialButton->setEnabled(true);
        d->serialButton->setChecked(false);
        d->serialButton->setEnabled(false);
        d->usbButton->setEnabled(true);
        d->usbButton->setChecked(false);
        d->usbButton->setEnabled(false);
        d->portPathComboBox->setEnabled(true);
        d->portPathComboBox->insertItem(0, QString("NONE"));
        d->portPathComboBox->setEnabled(false);

        d->umsMountURL->setEnabled(true);
        d->umsMountURL->clear();
        d->umsMountURL->setUrl(QString("/mnt/camera"));
        return;
    }
    else 
    {
        d->umsMountURL->setEnabled(true);
        d->umsMountURL->clear();
        d->umsMountURL->setUrl(QString("/"));
        d->umsMountURL->setEnabled(false);
    }

    d->titleEdit->setText(model);
    
    QStringList plist;
    GPIface::getCameraSupportedPorts(model, plist);

    if (plist.contains("serial")) 
    {
        d->serialButton->setEnabled(true);
        d->serialButton->setChecked(true);
    }
    else 
    {
        d->serialButton->setEnabled(true);
        d->serialButton->setChecked(false);
        d->serialButton->setEnabled(false);
    }

    if (plist.contains("usb")) 
    {
        d->usbButton->setEnabled(true);
        d->usbButton->setChecked(true);
    }
    else 
    {
        d->usbButton->setEnabled(true);
        d->usbButton->setChecked(false);
        d->usbButton->setEnabled(false);
    }

    slotPortChanged();
}

void CameraSelection::slotPortChanged()
{
    if (d->usbButton->isChecked()) 
    {
        d->portPathComboBox->setEnabled(true);
        d->portPathComboBox->clear();
        d->portPathComboBox->insertItem( 0, QString("usb:"));
        d->portPathComboBox->setEnabled(false);
        return;
    }

    if (d->serialButton->isChecked()) 
    {
        d->portPathComboBox->setEnabled(true);
        d->portPathComboBox->clear();
        d->portPathComboBox->insertItems(0, d->serialPortList);
    }
}

QString CameraSelection::currentTitle()
{
    return d->titleEdit->text();    
}

QString CameraSelection::currentModel()
{
    Q3ListViewItem* item = d->listView->currentItem();
    if (!item)
        return QString();

    QString model(item->text(0));
    if (model == d->UMSCameraNameShown)
        model = d->UMSCameraNameActual;

    return model;
}

QString CameraSelection::currentPortPath()
{
    return d->portPathComboBox->currentText();
}

QString CameraSelection::currentCameraPath()
{
    return d->umsMountURL->url().path();
}

void CameraSelection::slotOkClicked()
{
    emit signalOkClicked(currentTitle(),    currentModel(),
                         currentPortPath(), currentCameraPath());
}

}  // namespace Digikam
