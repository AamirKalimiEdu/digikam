/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-02
 * Description : an option to provide directory information to the parser
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

#include "directorynameoption.moc"

// Qt includes

#include <QFileInfo>
#include <QString>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

namespace Digikam
{

DirectoryNameOption::DirectoryNameOption()
                   : Option(i18n("Directory"), i18n("Add the directory name"), SmallIcon("folder"))
{
    addToken("[dir]", i18nc("Directory name", "Current"),
            i18n("Directory name"));

    addToken("[dir.]", i18nc("directory name", "Parent Directory Name"),
            i18n("Directory name of the parent, additional '.' characters move up "
                 "in the directory hierarchy"));

    QRegExp reg("\\[dir(\\.*)\\]");
    reg.setMinimal(true);
    setRegExp(reg);
}

void DirectoryNameOption::parseOperation(const QString& parseString, ParseInformation& info, ParseResults& results)
{
    QFileInfo fi(info.fileUrl.toLocalFile());
    QStringList folders = fi.absolutePath().split('/', QString::SkipEmptyParts);

    QRegExp reg     = regExp();
    int folderCount = folders.count();

    // --------------------------------------------------------

    QString tmp;
    PARSE_LOOP_START(parseString, reg)
    {
        int matchedLength = reg.cap(1).length();

        if (matchedLength == 0)
        {
            tmp = folders.last();
        }
        else if (matchedLength > (folderCount - 1))
        {
            tmp.clear();
        }
        else
        {
            tmp = folders[folderCount - matchedLength - 1];
        }
    }
    PARSE_LOOP_END(parseString, reg, tmp, results)
}

} // namespace Digikam
