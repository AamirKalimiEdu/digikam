/*  This file is part of the KDE project
    Copyright (C) 2007 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef DIGIKAM_EXPORT_H
#define DIGIKAM_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef DIGIKAM_EXPORT
# if defined(MAKE_DIGIKAM_LIB)
   /* We are building this library */ 
#  define DIGIKAM_EXPORT KDE_EXPORT
# else
   /* We are using this library */ 
#  define DIGIKAM_EXPORT KDE_IMPORT
# endif
#endif

#ifndef DIGIKAM_DATABASE_EXPORT
# if defined(MAKE_DIGIKAMDATABASE_LIB)
   /* We are building this library */ 
#  define DIGIKAM_DATABASE_EXPORT KDE_EXPORT
# else
   /* We are using this library */ 
#  define DIGIKAM_DATABASE_EXPORT KDE_IMPORT
# endif
#endif

#ifndef DIGIKAMIMAGEPLUGINS_EXPORT
# if defined(MAKE_DIGIKAMIMAGEPLUGINS_LIB)
   /* We are building this library */
#  define DIGIKAMIMAGEPLUGINS_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define DIGIKAMIMAGEPLUGINS_EXPORT KDE_IMPORT
# endif
#endif


#ifndef DIGIKAMIMAGEEDITOR_EXPORT
# if defined(MAKE_DIGIKAMIMAGEEDITOR_LIB)
   /* We are building this library */
#  define DIGIKAMIMAGEEDITOR_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define DIGIKAMIMAGEEDITOR_EXPORT KDE_IMPORT
# endif
#endif

#ifndef DIGIKAMIMAGEFILTERS_EXPORT
# if defined(MAKE_DIGIKAMIMAGEFILTERS_LIB)
   /* We are building this library */
#  define DIGIKAMIMAGEFILTERS_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define DIGIKAMIMAGEFILTERS_EXPORT KDE_IMPORT
# endif
#endif

#ifndef DIGIKAMIMAGEWIDGET_EXPORT
# if defined(MAKE_DIGIKAMIMAGEWIDGET_LIB)
   /* We are building this library */
#  define DIGIKAMIMAGEWIDGET_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define DIGIKAMIMAGEWIDGET_EXPORT KDE_IMPORT
# endif
#endif


# ifndef DIGIKAM_EXPORT_DEPRECATED
#  define DIGIKAM_EXPORT_DEPRECATED KDE_DEPRECATED DIGIKAM_EXPORT
# endif

#endif
