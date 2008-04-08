/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-25
 * Description : a bar used to search a string.
 * 
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QColor>
#include <QPalette>
#include <QString>

// KDE includes.

#include <kconfiggroup.h>
#include <kglobal.h>
#include <kconfig.h>

// Local includes

#include "searchtextbar.h"
#include "searchtextbar.moc"

namespace Digikam
{

class SearchTextBarPriv
{
public:

    SearchTextBarPriv()
    {
        textQueryCompletion = false;
    }

    bool textQueryCompletion;
};

SearchTextBar::SearchTextBar(QWidget *parent, const char* name, const QString& msg)
             : KLineEdit(parent)
{
    d = new SearchTextBarPriv;
    setAttribute(Qt::WA_DeleteOnClose);
    setClearButtonShown(true);
    setClickMessage(msg);
    setObjectName(name);

    KCompletion *kcom = new KCompletion;
    kcom->setOrder(KCompletion::Sorted);
    setCompletionObject(kcom, true);
    setAutoDeleteCompletionObject(true);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));

    connect(this, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTextChanged(const QString&)));

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(name + QString(" Search Text Tool"));
    setCompletionMode((KGlobalSettings::Completion)group.readEntry("AutoCompletionMode", 
                      (int)KGlobalSettings::CompletionAuto));
}

SearchTextBar::~SearchTextBar()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(objectName() + QString(" Search Text Tool"));
    group.writeEntry("AutoCompletionMode", (int)completionMode());
    group.sync();

    delete d;
}

void SearchTextBar::setEnableTextQueryCompletion(bool b)
{
    d->textQueryCompletion = b;
}

bool SearchTextBar::textQueryCompletion() const
{
    return d->textQueryCompletion;
}

void SearchTextBar::slotTextChanged(const QString& text)
{
    if (text.isEmpty())
        setPalette(QPalette());
}

void SearchTextBar::slotSearchResult(bool match)
{
    if (text().isEmpty())
    {
        setPalette(QPalette());
        return;
    }

    QPalette pal = palette();
    pal.setColor(QPalette::Active, QColorGroup::Base,
                 match ? QColor(200, 255, 200) :
                 QColor(255, 200, 200));
    pal.setColor(QPalette::Active, QColorGroup::Text, Qt::black);
    setPalette(pal);

    // If search result match the text query, we put the text 
    // in auto-completion history.
    if (d->textQueryCompletion && match)
        completionObject()->addItem(text());
}

}  // namespace Digikam
