/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-01-11
 * Description : a directory selection widget.
 * 
 * Copyright 2005-2007 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Library General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * ============================================================ */

#ifndef DIRSELECTWIDGET_H
#define DIRSELECTWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qstring.h>

// KDE includes.

#include <k3filetreeview.h>
#include <kurl.h>

namespace DigikamSuperImposeImagesPlugin
{

class DirSelectWidget : public K3FileTreeView 
{
Q_OBJECT

public:
     
    DirSelectWidget(QWidget* parent, const char* name=0, QString headerLabel=QString());

    DirSelectWidget(KUrl rootUrl=KUrl("/"), KUrl currentUrl=KUrl(), 
                    QWidget* parent=0, const char* name=0, QString headerLabel=QString());

    ~DirSelectWidget();
     
    KUrl path() const;
    KUrl rootPath(void);
    void setRootPath(KUrl rootUrl, KUrl currentUrl=KUrl(QString()));
    void setCurrentPath(KUrl currentUrl);

signals :
    
    void folderItemSelected(const KUrl &url);
        
protected slots:

    void load();
    void slotFolderSelected(Q3ListViewItem *);

private:
    
    struct Private;
    Private* d;
};

}  // NameSpace DigikamSuperImposeImagesPlugin

#endif /* DIRSELECTWIDGET_H */
