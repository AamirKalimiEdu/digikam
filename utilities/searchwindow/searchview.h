/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 * 
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef SEARCHVIEW_H
#define SEARCHVIEW_H

// Qt includes

#include <QCache>
#include <QList>
#include <QRect>
#include <QWidget>

// KDE includes

// Local includes


class QVBoxLayout;
class QTimeLine;
class KDialogButtonBox;
class KPushButton;

namespace Digikam
{

class SearchGroup;
class SearchViewBottomBar;
class SearchClickLabel;
class SearchXmlCachingReader;
class SearchXmlWriter;

class SearchViewThemedPartsCache
{
public:

    virtual ~SearchViewThemedPartsCache() {}
    virtual QPixmap groupLabelPixmap(int w, int h) = 0;
    virtual QPixmap bottomBarPixmap(int w, int h) = 0;
};

class AbstractSearchGroupContainer : public QWidget
{

    Q_OBJECT

public:

    /// Abstract base class for classes that contain SearchGroups
    // To contain common code of SearchView and SearchGroup,
    // as SearchGroups can have subgroups.

    AbstractSearchGroupContainer(QWidget *parent = 0);

public slots:

    SearchGroup *addSearchGroup();
    void removeSearchGroup(SearchGroup *group);

protected:

    /// Call before reading the XML part that could contain group elements
    void startReadingGroups(SearchXmlCachingReader &reader);
    /// Call when a group element is the current element
    void readGroup(SearchXmlCachingReader &reader);
    /// Call when the XML part is finished
    void finishReadingGroups();
    /// Write contained groups to writer
    void writeGroups(SearchXmlWriter &writer);
    /// Collects the data from the same method of all contained groups (position relative to this widget)
    QList<QRect> startupAnimationAreaOfGroups() const;
    /// Reimplement: create and setup a search group
    virtual SearchGroup *createSearchGroup() = 0;
    /// Reimplement: Adds a newly created group to the layout structures
    virtual void addGroupToLayout(SearchGroup *group) = 0;

protected slots:

    void removeSendingSearchGroup();

protected:

    int m_groupIndex;
    QList<SearchGroup *> m_groups;
};

class SearchViewPrivate;
class SearchView : public AbstractSearchGroupContainer, public SearchViewThemedPartsCache
{

    Q_OBJECT

public:

    SearchView();
    ~SearchView();

    void setup();
    void setBottomBar(SearchViewBottomBar *bar);

    void read(const QString &search);
    QString write();

signals:

    void searchOk();
    void searchTryout();
    void searchCancel();

protected slots:

    void setTheme();
    void slotAddGroupButton();
    void slotResetButton();
    void startAnimation();
    void animationFrame(int);
    void timeLineFinished();

public:

    QPixmap groupLabelPixmap(int w, int h);
    QPixmap bottomBarPixmap(int w, int h);

protected:

    virtual void paintEvent(QPaintEvent *e);
    virtual void showEvent(QShowEvent *event);

    virtual SearchGroup *createSearchGroup();
    virtual void addGroupToLayout(SearchGroup *group);
    QPixmap cachedBannerPixmap(int w, int h);

private:

    SearchViewPrivate   *d;
};

class SearchViewBottomBar : public QWidget
{
    Q_OBJECT

public:

    SearchViewBottomBar(SearchViewThemedPartsCache * cache, QWidget *parent = 0);

signals:

    void okPressed();
    void cancelPressed();
    void tryoutPressed();
    void addGroupPressed();
    void resetPressed();

protected:

    virtual void paintEvent(QPaintEvent *);

    SearchViewThemedPartsCache *m_themeCache;
    QHBoxLayout                *m_mainLayout;
    KDialogButtonBox           *m_buttonBox;
    KPushButton                *m_addGroupsButton;
    KPushButton                *m_resetButton;
};

}

#endif


