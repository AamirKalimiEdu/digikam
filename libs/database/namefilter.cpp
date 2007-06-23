/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-02
 * Description : Building complex database SQL queries from search descriptions
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

// Qt includes

#include <QStringList>

// Local includes

#include "namefilter.h"

namespace Digikam
{

NameFilter::NameFilter(const QString &filter)
{
    if ( filter.isEmpty() )
        return;

    QChar sep( ';' );
    int i = filter.indexOf( sep );
    if ( i == -1 && filter.indexOf( ' ') != -1 )
        sep = QChar( ' ' );

    QStringList list = filter.split(sep, QString::SkipEmptyParts);
    QStringList::const_iterator it = list.constBegin();
    while ( it != list.constEnd() ) {
        m_filterList << QRegExp( (*it).trimmed() );
        ++it;
    }
}

bool NameFilter::matches(const QString &name)
{
    QList<QRegExp>::const_iterator rit = m_filterList.constBegin();
    while ( rit != m_filterList.constEnd() ) {
        if ( (*rit).exactMatch(name) )
            return true;
        ++rit;
    }
    return false;
}


}




