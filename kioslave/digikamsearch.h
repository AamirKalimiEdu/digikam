/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date   : 2005-04-21
 * Description :
 *
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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
 * ============================================================ */

#ifndef DIGIKAMSEARCH_H
#define DIGIKAMSEARCH_H

// KDE includes.

#include <kio/slavebase.h>

class kio_digikamsearch : public KIO::SlaveBase
{

public:

    kio_digikamsearch(const QCString &pool_socket, const QCString &app_socket);
    ~kio_digikamsearch();

    void special(const QByteArray& data);

private:
};

#endif /* DIGIKAMSEARCH_H */
