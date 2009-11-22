/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide date information to the parser
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "dateoption.moc"

// Qt includes

#include <QDateTime>
#include <QPointer>
#include <QTimer>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "ui_dateoptiondialogwidget.h"

namespace Digikam
{

const QString dateFormatLink = QString("<a href='http://doc.trolltech.com/latest/qdatetime.html#toString'>"
                                       "format settings"
                                       "</a>");

// --------------------------------------------------------

DateFormat::DateFormat()
{
    m_map.clear();

    m_map.insert(Standard, DateFormatDescriptor(QString("Standard"), QString("yyyyMMddThhmmss")));
    m_map.insert(ISO,      DateFormatDescriptor(QString("ISO"),      Qt::ISODate));
    m_map.insert(FullText, DateFormatDescriptor(QString("Text"),     Qt::TextDate));
    m_map.insert(Locale,   DateFormatDescriptor(QString("Locale"),   Qt::SystemLocaleShortDate));
    m_map.insert(Custom,   DateFormatDescriptor(QString("Custom"),   QString("")));
}

QString DateFormat::identifier(Type type)
{
    DateFormatDescriptor desc = m_map.at((int)type);
    return desc.first;
}

QVariant DateFormat::formatType(Type type)
{
    DateFormatDescriptor desc = m_map.at((int)type);
    return desc.second;
}

QVariant DateFormat::formatType(QString identifier)
{
    QVariant v;
    foreach (const DateFormatDescriptor& desc, m_map)
    {
        if (desc.first == identifier)
        {
            v = desc.second;
            break;
        }
    }

    if (identifier.isEmpty())
    {
        return m_map.at(Standard).second;
    }

    return v;
}

// --------------------------------------------------------

DateOptionDialog::DateOptionDialog(ParseObject* parent)
                : ParseObjectDialog(parent), ui(new Ui::DateOptionDialogWidget)
{
    QWidget* mainWidget = new QWidget(this);
    ui->setupUi(mainWidget);

    // fill the date format combobox
    DateFormat df;
    foreach (const DateFormat::DateFormatDescriptor& desc, df.map())
    {
        ui->dateFormatPicker->addItem(desc.first);
    }
    ui->dateFormatLink->setOpenExternalLinks(true);
    ui->dateFormatLink->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::LinksAccessibleByKeyboard);
    ui->dateFormatLink->setText(dateFormatLink);

    ui->customFormatInput->setClickMessage(i18n("Enter custom date format"));
    connect(ui->dateFormatPicker, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotDateFormatChanged(int)));

    connect(ui->customFormatInput, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotCustomFormatChanged(const QString&)));

    ui->dateFormatPicker->setCurrentIndex(DateFormat::Standard);
    slotDateFormatChanged(ui->dateFormatPicker->currentIndex());

    // --------------------------------------------------------

    setSettingsWidget(mainWidget);
}

DateOptionDialog::~DateOptionDialog()
{
    delete ui;
}

QString DateOptionDialog::formattedDateTime(const QDateTime& date)
{
    if (ui->dateFormatPicker->currentIndex() == DateFormat::Custom)
    {
        return date.toString(ui->customFormatInput->text());
    }

    DateFormat df;
    QVariant v;

    v = df.formatType((DateFormat::Type)ui->dateFormatPicker->currentIndex());
    QString tmp;
    if (v.type() == QVariant::String)
    {
        tmp = date.toString(v.toString());
    }
    else
    {
        tmp = date.toString((Qt::DateFormat)v.toInt());
    }
    return tmp;
}

void DateOptionDialog::slotDateFormatChanged(int index)
{
    ui->customFormatInput->setEnabled(index == DateFormat::Custom);

    ui->dateFormatLink->setEnabled(index == DateFormat::Custom);
    ui->dateFormatLink->setVisible(index == DateFormat::Custom);

    updateExampleLabel();
}

void DateOptionDialog::slotCustomFormatChanged(const QString&)
{
    updateExampleLabel();
}

void DateOptionDialog::updateExampleLabel()
{
    QString tmp = QString("example: %1").arg(formattedDateTime(QDateTime::currentDateTime()));
    ui->exampleLabel->setText(tmp);
}

// --------------------------------------------------------

DateOption::DateOption()
          : Option(i18n("Date && Time..."),
                   i18n("Add date and time information"),
                   SmallIcon("view-pim-calendar"))
{
    setUseTokenMenu(false);

    addTokenDescription("[date]", i18n("Date && Time"),
             i18n("Date and time (standard format)"));

    addTokenDescription("[date:|key|]", i18n("Date && Time (key)"),
             i18n("Date and time (|key| = ISO/Text/Locale)"));

    addTokenDescription("[date:|format|]", i18n("Date && Time (custom format)"),
             i18n("Date and time") + " (" +  dateFormatLink + ')');

    QRegExp reg("\\[date(:.*)?\\]");
    reg.setMinimal(true);
    setRegExp(reg);
}

void DateOption::parseOperation(const QString& parseString, ParseInformation& info, ParseResults& results)
{
    QRegExp reg = regExp();

    // --------------------------------------------------------

    QString tmp;
    PARSE_LOOP_START(parseString, reg)
    {
        DateFormat df;

        QString token = reg.cap(1);
        if (!token.isEmpty())
        {
            token.remove(0, 1);
        }

        QVariant v = df.formatType(token);
        if (v.isNull())
        {
            tmp = info.dateTime.toString(token);
        }
        else
        {
            if (v.type() == QVariant::String)
            {
                tmp = info.dateTime.toString(v.toString());
            }
            else
            {
                tmp = info.dateTime.toString((Qt::DateFormat)v.toInt());
            }
        }
    }
    PARSE_LOOP_END(parseString, reg, tmp, results)
}

void DateOption::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QVariant v;
    DateFormat df;
    QString dateString;

    QString tokenStr               = QString("[date:%1]");
    QPointer<DateOptionDialog> dlg = new DateOptionDialog(this);

    if (dlg->exec() == KDialog::Accepted)
    {
        int index = dlg->ui->dateFormatPicker->currentIndex();

        // use custom date format?
        if (dlg->ui->fixedDateBtn->isChecked())
        {
            QDateTime date;
            date.setDate(dlg->ui->datePicker->date());
            date.setTime(dlg->ui->timePicker->time());

            v = (index == DateFormat::Custom) ? dlg->ui->customFormatInput->text()
                                              : df.formatType((DateFormat::Type)index);

            if (v.type() == QVariant::String)
            {
                dateString = date.toString(v.toString());
            }
            else
            {
                dateString = date.toString((Qt::DateFormat)v.toInt());
            }
        }
        else        // use predefined keywords for date formatting
        {
            switch (index)
            {
                case DateFormat::Standard:
                    dateString = tokenStr.arg(QString(""));
                    dateString.remove(':');
                    break;
                case DateFormat::Custom:
                    dateString = tokenStr.arg(dlg->ui->customFormatInput->text());
                    break;
                default:
                    QString identifier = df.identifier((DateFormat::Type) index);
                    dateString         = tokenStr.arg(identifier);
                    break;
            }
        }
    }

    delete dlg;
    emit signalTokenTriggered(dateString);
}

} // namespace Digikam
