/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database settings widget
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dbsettingswidget.h"

// Qt includes

#include <QGridLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSqlDatabase>
#include <QSqlError>
#include <QLabel>
#include <QGroupBox>
#include <QTimer>
#include <QTemporaryFile>
#include <QApplication>
#include <QStyle>
#include <QMessageBox>
#include <QTextBrowser>
#include <QTabWidget>
#include <QMessageBox>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QUrl>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dwidgetutils.h"
#include "dexpanderbox.h"
#include "dbengineparameters.h"

namespace Digikam
{

class DatabaseSettingsWidget::Private
{

public:

    Private()
    {
        dbType         = 0;
        dbPathLabel    = 0;
        expertSettings = 0;
        dbNoticeBox    = 0;
        sqlInit        = 0;
        dbNameCore     = 0;
        dbNameThumbs   = 0;
        dbNameFace     = 0;
        hostName       = 0;
        connectOpts    = 0;
        userName       = 0;
        password       = 0;
        hostPort       = 0;
        dbPathEdit     = 0;
        tab            = 0;
        dbDetailsBox   = 0;
    }

    QLineEdit*         dbNameCore;
    QLineEdit*         dbNameThumbs;
    QLineEdit*         dbNameFace;
    QLineEdit*         hostName;
    QLineEdit*         connectOpts;
    QLineEdit*         userName;
    QLineEdit*         password;

    QSpinBox*          hostPort;

    QComboBox*         dbType;
    QLabel*            dbPathLabel;
    QTextBrowser*      sqlInit;
    QGroupBox*         expertSettings;
    QGroupBox*         dbNoticeBox;
    QGroupBox*         dbDetailsBox;
    QTabWidget*        tab;

    DFileSelector*     dbPathEdit;

    DbEngineParameters orgPrms;
};

DatabaseSettingsWidget::DatabaseSettingsWidget(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setupMainArea();
}

DatabaseSettingsWidget::~DatabaseSettingsWidget()
{
    delete d;
}

void DatabaseSettingsWidget::setupMainArea()
{
    QVBoxLayout* const layout = new QVBoxLayout();
    setLayout(layout);

    // --------------------------------------------------------

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGroupBox* const dbConfigBox    = new QGroupBox(i18n("Database Configuration"), this);
    QVBoxLayout* const vlay         = new QVBoxLayout(dbConfigBox);

    DHBox* const typeHbox           = new DHBox();
    QLabel* const databaseTypeLabel = new QLabel(typeHbox);
    d->dbType                       = new QComboBox(typeHbox);
    databaseTypeLabel->setText(i18n("Type:"));

    // --------------------------------------------------------

    d->dbPathLabel = new QLabel(i18n("<p>Set here the location where the database files will be stored on your system. "
                                     "There are 3 databases : one for all collections properties, "
                                     "one to store compressed thumbnails, "
                                     "and one to store faces recognition metadata.<br/>"
                                     "Write access is required to be able to edit image properties.</p>"
                                     "<p>Databases are digiKam core engines. Take a care to use a place hosted by a fast "
                                     "hardware (as SSD) with enough free space especially for thumbnails database.</p>"
                                     "<p>Note: a remote file system such as NFS, cannot be used here. "
                                     "For performance reasons, it's also recommended to not use a removable media.</p>"
                                     "<p></p>"), dbConfigBox);
    d->dbPathLabel->setWordWrap(true);
    d->dbPathEdit  = new DFileSelector(dbConfigBox);
    d->dbPathEdit->setFileDlgMode(QFileDialog::Directory);

    // --------------------------------------------------------

    d->tab = new QTabWidget(this);
    
    // Only accept printable Ascii char for database names.
    QRegExp asciiRx(QLatin1String("[\x20-\x7F]+$"));
    QValidator* const asciiValidator = new QRegExpValidator(asciiRx, this);

    QLabel* const dbNameCoreLabel                    = new QLabel(i18n("Core Db Name:"));
    d->dbNameCore                                    = new QLineEdit();
    d->dbNameCore->setPlaceholderText(i18n("Set the core database name"));
    d->dbNameCore->setToolTip(i18n("The core database is lead digiKam container used to store\nalbums, items, and searches metadata."));
    d->dbNameCore->setValidator(asciiValidator);

    QLabel* const dbNameThumbsLabel                  = new QLabel(i18n("Thumbs Db Name:"));
    d->dbNameThumbs                                  = new QLineEdit();
    d->dbNameThumbs->setPlaceholderText(i18n("Set the thumbnails database name"));
    d->dbNameThumbs->setToolTip(i18n("The thumbnails database is used by digiKam to host\nimage thumbs with wavelets compression images.\n"
                                     "This one can use quickly a lots of space,\nespecially if you have huge collections."));
    d->dbNameThumbs->setValidator(asciiValidator);

    QLabel* const dbNameFaceLabel                    = new QLabel(i18n("Face Db Name:"));
    d->dbNameFace                                    = new QLineEdit();
    d->dbNameFace->setPlaceholderText(i18n("Set the face database name"));
    d->dbNameFace->setToolTip(i18n("The face database is used by digiKam to host image histograms\ndedicated to faces recognition process.\n"
                                   "This one can use quickly a lots of space, especially\nif you a lots of image with people faces detected "
                                   "and tagged."));
    d->dbNameFace->setValidator(asciiValidator);

    QLabel* const hostNameLabel                      = new QLabel(i18n("Host Name:"));
    d->hostName                                      = new QLineEdit();
    d->hostName->setPlaceholderText(i18n("Set the host computer name"));
    d->hostName->setToolTip(i18n("This is the computer name running Mysql server.\nThis can be \"localhost\" for a local server, "
                                 "or the network computer\n name (or IP address) in case of remote computer."));

    QLabel* const hostPortLabel                      = new QLabel(i18n("Host Port:"));
    d->hostPort                                      = new QSpinBox();
    d->hostPort->setToolTip(i18n("Set the host computer port.\nUsually, Mysql server use port number 3306 by default"));
    d->hostPort->setMaximum(65535);

    QLabel* const connectOptsLabel                   = new QLabel(i18n("Connect options:"));
    d->connectOpts                                   = new QLineEdit();
    d->connectOpts->setPlaceholderText(i18n("Set the database connection options"));
    d->connectOpts->setToolTip(i18n("Set the Mysql server connection options.\nFor advanced users only."));

    QLabel* const userNameLabel                      = new QLabel(i18n("User:"));
    d->userName                                      = new QLineEdit();
    d->userName->setPlaceholderText(i18n("Set the database account name"));
    d->userName->setToolTip(i18n("Set the Mysql server account name used\nby digiKam to be connected to the server."));

    QLabel* const passwordLabel                      = new QLabel(i18n("Password:"));
    d->password                                      = new QLineEdit();
    d->password->setToolTip(i18n("Set the Mysql server account password used\nby digiKam to be connected to the server."));
    d->password->setEchoMode(QLineEdit::Password);

    QPushButton* const checkDatabaseConnectionButton = new QPushButton(i18n("Check Database Connection"));

    d->expertSettings                                = new QGroupBox();
    d->expertSettings->setFlat(true);
    QFormLayout* const expertSettinglayout           = new QFormLayout();
    d->expertSettings->setLayout(expertSettinglayout);

    expertSettinglayout->addRow(hostNameLabel,     d->hostName);
    expertSettinglayout->addRow(hostPortLabel,     d->hostPort);
    expertSettinglayout->addRow(dbNameCoreLabel,   d->dbNameCore);
    expertSettinglayout->addRow(dbNameThumbsLabel, d->dbNameThumbs);
    expertSettinglayout->addRow(dbNameFaceLabel,   d->dbNameFace);
    expertSettinglayout->addRow(userNameLabel,     d->userName);
    expertSettinglayout->addRow(passwordLabel,     d->password);
    expertSettinglayout->addRow(connectOptsLabel,  d->connectOpts);

    expertSettinglayout->addWidget(checkDatabaseConnectionButton);

    d->tab->addTab(d->expertSettings, i18n("Remote Server Settings"));

    // --------------------------------------------------------

    d->dbNoticeBox           = new QGroupBox(i18n("Database Server Instructions"), this);
    QVBoxLayout* const vlay2 = new QVBoxLayout(d->dbNoticeBox);
    QLabel* const notice     = new QLabel(i18n("<p>digiKam expects the above database and user account to already exists. "
                                               "This user also require full access to the database.<br>"
                                               "If your database is not already set up, you can use the following SQL commands "
                                               "(after replacing the password with the correct one)."), d->dbNoticeBox);
    notice->setWordWrap(true);

    d->sqlInit = new QTextBrowser(d->dbNoticeBox);
    d->sqlInit->setOpenExternalLinks(false);
    d->sqlInit->setOpenLinks(false);
    d->sqlInit->setReadOnly(false);

    vlay2->addWidget(notice);
    vlay2->addWidget(d->sqlInit);
    vlay2->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay2->setSpacing(spacing);

    d->tab->addTab(d->dbNoticeBox, i18n("Requirements"));

    // --------------------------------------------------------

    d->dbDetailsBox          = new QGroupBox(i18n("Database Server Technical Details"), this);
    QVBoxLayout* const vlay3 = new QVBoxLayout(d->dbDetailsBox);
    QLabel* const details    = new QLabel(i18n("<p>Use this configuration view to set all information "
                                               "to be connected to a remote Mysql database server through the network. "
                                               "As with Sqlite or Mysql internal server, 3 databases will be stored "
                                               "on the remote server: one for all collections properties, "
                                               "one to store compressed thumbnails, and one to store faces "
                                               "recognition metadata.</p>"
                                               "<p>Unlike Sqlite or Mysql internal server, you can customize the "
                                               "database names to simplify your backups.</p>"
                                               "<p>Databases are digiKam core engines. To prevent performance issues, "
                                               "take a care to use a fast network link between the client and the server "
                                               "computers. It's also recommended to host database files on "
                                               "fast hardware (as SSD) with enough free space, "
                                               "especially for thumbnails database.</p>"
                                               "<p>The databases must be create on the remote server by the administrator. "
                                               "Look in <b>Requirements</b> tab for details.</p>"),
                                               d->dbDetailsBox);
    details->setWordWrap(true);

    vlay3->addWidget(details);
    vlay3->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay3->setSpacing(spacing);

    d->tab->addTab(d->dbDetailsBox, i18n("Documentation"));

    // --------------------------------------------------------

    vlay->addWidget(typeHbox);
    vlay->addWidget(new DLineWidget(Qt::Horizontal));
    vlay->addWidget(d->dbPathLabel);
    vlay->addWidget(d->dbPathEdit);
    vlay->addWidget(d->tab);
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);

    // --------------------------------------------------------

    layout->setContentsMargins(QMargins());
    layout->setSpacing(spacing);
    layout->addWidget(dbConfigBox);
    layout->addStretch();

    // --------- fill with default values ---------------------

    d->dbType->addItem(i18n("SQLite"),                        SQlite);

#ifdef HAVE_MYSQLSUPPORT

#   ifdef HAVE_INTERNALMYSQL
    d->dbType->addItem(i18n("MySQL Internal (experimental)"), MysqlInternal);
#   endif

    d->dbType->addItem(i18n("MySQL Server (experimental)"),   MysqlServer);
#endif

    d->dbType->setToolTip(i18n("<p>Select here the type of database backend.</p>"
                               "<p><b>SQlite</b> backend is for local database storage with a small or medium collection sizes. "
                               "It is the default and recommended backend.</p>"
#ifdef HAVE_MYSQLSUPPORT

#   ifdef HAVE_INTERNALMYSQL
                               "<p><b>MySQL Internal</b> backend is for local database storage with huge collection sizes. "
                               "Be careful: this one still in experimental stage.</p>"
#   endif

                               "<p><b>MySQL Server</b> backend is a more robust solution especially for remote and shared database storage. "
                               "It is also more efficient to manage huge collection sizes. "
                               "Be careful: this one still in experimental stage.</p>"
#endif
                              ));

    // --------------------------------------------------------

    connect(d->dbType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotHandleDBTypeIndexChanged(int)));

    connect(checkDatabaseConnectionButton, SIGNAL(clicked()),
            this, SLOT(slotCheckMysqlServerConnection()));

    connect(d->dbNameCore, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(d->dbNameThumbs, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(d->dbNameFace, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(d->userName, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    slotHandleDBTypeIndexChanged(d->dbType->currentIndex());
}

int DatabaseSettingsWidget::databaseType() const
{
    return d->dbType->currentIndex();
}

QString DatabaseSettingsWidget::databasePath() const
{
    return d->dbPathEdit->lineEdit()->text();
}

void DatabaseSettingsWidget::setDatabasePath(const QString& path)
{
    d->dbPathEdit->lineEdit()->setText(path);
}
    
DbEngineParameters DatabaseSettingsWidget::orgDatabasePrm() const
{
    return d->orgPrms;
}

QString DatabaseSettingsWidget::databaseBackend() const
{
    switch(databaseType())
    {
        case MysqlInternal:
        case MysqlServer:
        {
            return DbEngineParameters::MySQLDatabaseType();
        }
        default: // SQlite
        {
            return DbEngineParameters::SQLiteDatabaseType();
        }
    }
}

void DatabaseSettingsWidget::slotHandleDBTypeIndexChanged(int index)
{
    setDatabaseInputFields(index);
    handleInternalServer(index);
    slotUpdateSqlInit();
}

void DatabaseSettingsWidget::setDatabaseInputFields(int index)
{
    switch(index)
    {
        case MysqlInternal:
        case SQlite:
        {
            d->dbPathLabel->setVisible(true);
            d->dbPathEdit->setVisible(true);
            d->tab->setVisible(false);

            connect(d->dbPathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                    this, SLOT(slotDatabasePathEditedDelayed()));

            break;
        }
        default: // MysqlServer
        {
            d->dbPathLabel->setVisible(false);
            d->dbPathEdit->setVisible(false);
            d->tab->setVisible(true);

            disconnect(d->dbPathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                       this, SLOT(slotDatabasePathEditedDelayed()));
            break;
        }
    }
}

void DatabaseSettingsWidget::handleInternalServer(int index)
{
    bool internal = (index == MysqlInternal);

    d->hostName->setDisabled(internal);
    d->hostPort->setDisabled(internal);
    d->dbNameCore->setDisabled(internal);
    d->dbNameThumbs->setDisabled(internal);
    d->dbNameFace->setDisabled(internal);
    d->userName->setDisabled(internal);
    d->password->setDisabled(internal);
    d->connectOpts->setDisabled(internal);
    d->hostPort->setValue(internal ? -1 : 3306);
}

void DatabaseSettingsWidget::slotUpdateSqlInit()
{
    QString sql = QString::fromLatin1("CREATE DATABASE %1; "
                                      "GRANT ALL PRIVILEGES ON %2.* TO \'%3\'@\'localhost\' IDENTIFIED BY \'password\'; "
                                      "FLUSH PRIVILEGES;\n")
                                      .arg(d->dbNameCore->text())
                                      .arg(d->dbNameCore->text())
                                      .arg(d->userName->text());

    if (d->dbNameThumbs->text() != d->dbNameCore->text())
    {
        sql += QString::fromLatin1("CREATE DATABASE %1; "
                                   "GRANT ALL PRIVILEGES ON %2.* TO \'%3\'@\'localhost\' IDENTIFIED BY \'password\'; "
                                   "FLUSH PRIVILEGES;\n")
                                   .arg(d->dbNameThumbs->text())
                                   .arg(d->dbNameThumbs->text())
                                   .arg(d->userName->text());
    }

    if (d->dbNameFace->text() != d->dbNameCore->text())
    {
        sql += QString::fromLatin1("CREATE DATABASE %1; "
                                   "GRANT ALL PRIVILEGES ON %2.* TO \'%3\'@\'localhost\' IDENTIFIED BY \'password\'; "
                                   "FLUSH PRIVILEGES;\n")
                                   .arg(d->dbNameFace->text())
                                   .arg(d->dbNameFace->text())
                                   .arg(d->userName->text());
    }

    d->sqlInit->setText(sql);
}

void DatabaseSettingsWidget::slotCheckMysqlServerConnection()
{
    QString error;

    if (checkMysqlServerConnection(error))
    {
        QMessageBox::information(qApp->activeWindow(), i18n("Database connection test"),
                                 i18n("Database connection test successful."));
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), i18n("Database connection test"),
                              i18n("Database connection test was not successful. <p>Error was: %1</p>",
                                   error));
    }
}

bool DatabaseSettingsWidget::checkMysqlServerConnectionConfig(QString& error)
{
    if (d->hostName->text().isEmpty())
    {
        error = i18n("The server hostname is empty");
        return false;
    }

    if (d->userName->text().isEmpty())
    {
        error = i18n("The server user name is empty");
        return false;
    }

    return true;
}

bool DatabaseSettingsWidget::checkMysqlServerDbNamesConfig(QString& error)
{
    if (d->dbNameCore->text().isEmpty())
    {
        error = i18n("The core database name is empty");
        return false;
    }

    if (d->dbNameThumbs->text().isEmpty())
    {
        error = i18n("The thubnails database name is empty");
        return false;
    }

    if (d->dbNameFace->text().isEmpty())
    {
        error = i18n("The face database name is empty");
        return false;
    }

    return true;
}

bool DatabaseSettingsWidget::checkMysqlServerConnection(QString& error)
{
    if (!checkMysqlServerConnectionConfig(error))
    {
        return false;
    }

    qApp->setOverrideCursor(Qt::WaitCursor);

    QString databaseID(QLatin1String("ConnectionTest"));
    QSqlDatabase testDatabase = QSqlDatabase::addDatabase(databaseBackend(), databaseID);

    DbEngineParameters prm    = getDbEngineParameters();
    qCDebug(DIGIKAM_DATABASE_LOG) << "Testing DB connection (" << databaseID << ") with these settings:";
    qCDebug(DIGIKAM_DATABASE_LOG) << prm;

    testDatabase.setHostName(prm.hostName);
    testDatabase.setPort(prm.port);
    testDatabase.setUserName(prm.userName);
    testDatabase.setPassword(prm.password);
    testDatabase.setConnectOptions(prm.connectOptions);

    bool result = testDatabase.open();
    error       = testDatabase.lastError().text();
    testDatabase.close();
    QSqlDatabase::removeDatabase(databaseID);

    qApp->restoreOverrideCursor();

    return result;
}

void DatabaseSettingsWidget::setParametersFromSettings(const ApplicationSettings* const settings)
{
    d->orgPrms = settings->getDbEngineParameters();

    if (d->orgPrms.databaseType == DbEngineParameters::SQLiteDatabaseType())
    {
        d->dbPathEdit->lineEdit()->setText(d->orgPrms.getCoreDatabaseNameOrDir());
        d->dbType->setCurrentIndex(SQlite);
    }
#ifdef HAVE_MYSQLSUPPORT

#   ifdef HAVE_INTERNALMYSQL
    else if (d->orgPrms.databaseType == DbEngineParameters::MySQLDatabaseType() && d->orgPrms.internalServer)
    {
        d->dbPathEdit->lineEdit()->setText(d->orgPrms.internalServerPath());
        d->dbType->setCurrentIndex(MysqlInternal);
    }
#   endif
    else
    {
        d->dbType->setCurrentIndex(MysqlServer);
    }
#endif

    d->dbNameCore->setText(d->orgPrms.databaseNameCore);
    d->dbNameThumbs->setText(d->orgPrms.databaseNameThumbnails);
    d->dbNameFace->setText(d->orgPrms.databaseNameFace);
    d->hostName->setText(d->orgPrms.hostName);
    d->hostPort->setValue(d->orgPrms.port);
    d->connectOpts->setText(d->orgPrms.connectOptions);

    d->userName->setText(d->orgPrms.userName);
    d->password->setText(d->orgPrms.password);

    slotHandleDBTypeIndexChanged(d->dbType->currentIndex());
}

DbEngineParameters DatabaseSettingsWidget::getDbEngineParameters() const
{
    DbEngineParameters prm;

    switch(databaseType())
    {
        case SQlite:
            prm = DbEngineParameters::parametersForSQLiteDefaultFile(databasePath());
            break;

        case MysqlInternal:
            prm = DbEngineParameters::defaultParameters(databaseBackend());
            prm.setInternalServerPath(databasePath());
            break;

        default: // MysqlServer
            prm.internalServer         = false;
            prm.databaseType           = databaseBackend();
            prm.databaseNameCore       = d->dbNameCore->text();
            prm.databaseNameThumbnails = d->dbNameThumbs->text();
            prm.databaseNameFace       = d->dbNameFace->text();
            prm.connectOptions         = d->connectOpts->text();
            prm.hostName               = d->hostName->text();
            prm.port                   = d->hostPort->text().toInt();
            prm.userName               = d->userName->text();
            prm.password               = d->password->text();
            break;
    }

    return prm;
}

void DatabaseSettingsWidget::slotDatabasePathEditedDelayed()
{
    QTimer::singleShot(300, this, SLOT(slotDatabasePathEdited()));
}

void DatabaseSettingsWidget::slotDatabasePathEdited()
{
    QString newPath = databasePath();

#ifndef _WIN32

    if (!newPath.isEmpty() && !QDir::isAbsolutePath(newPath))
    {
        d->dbPathEdit->lineEdit()->setText(QDir::homePath() + QLatin1Char('/') + QDir::fromNativeSeparators(newPath));
    }

#endif

    d->dbPathEdit->lineEdit()->setText(QDir::toNativeSeparators(newPath));
}

bool DatabaseSettingsWidget::checkDatabaseSettings()
{
    switch (databaseType())
    {
        case SQlite:
        case MysqlInternal:
        {
            QString dbFolder = databasePath();
            qCDebug(DIGIKAM_DATABASE_LOG) << "Database directory is : " << dbFolder;

            if (dbFolder.isEmpty())
            {
                QMessageBox::information(qApp->activeWindow(), qApp->applicationName(), 
                                        i18n("You must select a folder for digiKam to "
                                             "store information and metadata in a database file."));
                return false;
            }

            QDir targetPath(dbFolder);

            if (!targetPath.exists())
            {
                int rc = QMessageBox::question(qApp->activeWindow(), i18n("Create Database Folder?"),
                                            i18n("<p>The folder to put your database in does not seem to exist:</p>"
                                                 "<p><b>%1</b></p>"
                                                 "Would you like digiKam to create it for you?", dbFolder));

                if (rc == QMessageBox::No)
                {
                    return false;
                }

                if (!targetPath.mkpath(dbFolder))
                {
                    QMessageBox::information(qApp->activeWindow(), i18n("Create Database Folder Failed"),
                                            i18n("<p>digiKam could not create the folder to host your database file.\n"
                                                 "Please select a different location.</p>"
                                                 "<p><b>%1</b></p>", dbFolder));
                    return false;
                }
            }

            QFileInfo path(dbFolder);

#ifdef _WIN32
            // Work around bug #189168
            QTemporaryFile temp;
            temp.setFileTemplate(dbFolder + QLatin1String("XXXXXX"));

            if (!temp.open())
#else
            if (!path.isWritable())
#endif
            {
                QMessageBox::information(qApp->activeWindow(), i18n("No Database Write Access"),
                                        i18n("<p>You do not seem to have write access "
                                             "for the folder to host the database file.<br/>"
                                             "Please select a different location.</p>"
                                             "<p><b>%1</b></p>", dbFolder));
                return false;
            }

            break;
        }

        default:  // MysqlServer
        {
            QString error;

            if (!checkMysqlServerConnectionConfig(error))
            {
                QMessageBox::critical(qApp->activeWindow(), i18n("Database configuration"),
                                      i18n("The database connection configuration is not valid. Error is <br/><p>%1</p><br/>"
                                           "Please check your configuration.",
                                           error));
                return false;
            }

            if (!checkMysqlServerDbNamesConfig(error))
            {
                QMessageBox::critical(qApp->activeWindow(), i18n("Database configuration"),
                                      i18n("The database names configuration is not valid. Error is <br/><p>%1</p><br/>"
                                           "Please check your configuration.",
                                           error));
                return false;
            }

            if (!checkMysqlServerConnection(error))
            {
                QMessageBox::critical(qApp->activeWindow(), i18n("Database connection test"),
                                      i18n("Testing database connection has failed with error<br/><p>%1</p><br/>"
                                           "Please check your configuration.",
                                           error));
                return false;
            }

            break;
        }
    }

    return true;
}

} // namespace Digikam
