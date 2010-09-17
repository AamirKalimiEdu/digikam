/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : implementation of album view interface.
 *
 * Copyright (C) 2002-2005 by Renchi Raju  <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2002-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (c) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
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

#include "digikamview.moc"

// Qt includes

#include <QProcess>

// KDE includes

#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kmessagebox.h>
#include <krun.h>

// Local includes

#include "albumhistory.h"
#include "albumiconviewfilter.h"
#include "albumsettings.h"
#include "albumwidgetstack.h"
#include "batchsyncmetadata.h"
#include "digikamapp.h"
#include "digikamimageview.h"
#include "dzoombar.h"
#include "imagealbummodel.h"
#include "imageinfoalbumsjob.h"
#include "imagepreviewview.h"
#include "imagepropertiessidebardb.h"
#include "imageviewutilities.h"
#include "leftsidebarwidgets.h"
#include "loadingcacheinterface.h"
#include "mapwidgetview.h"
#include "metadatasettings.h"
#include "metadatamanager.h"
#include "queuemgrwindow.h"
#include "scancontroller.h"
#include "sidebar.h"
#include "slideshow.h"
#include "statusprogressbar.h"
#include "tagfiltersidebarwidget.h"
#include "tagmodificationhelper.h"
#include "imagepropertiesversionstab.h"
#include "tagscache.h"
#include "searchxml.h"
#include "faceiface.h"

namespace Digikam
{

class DigikamView::DigikamViewPriv
{
public:

    DigikamViewPriv()
    {
        dockArea              = 0;
        splitter              = 0;
        parent                = 0;
        iconView              = 0;
        albumManager          = 0;
        albumHistory          = 0;
        leftSideBar           = 0;
        rightSideBar          = 0;
        albumWidgetStack      = 0;
        selectionTimer        = 0;
        thumbSizeTimer        = 0;
        needDispatchSelection = false;
        cancelSlideShow       = false;
        useAlbumHistory       = false;
        thumbSize             = ThumbnailSize::Medium;
        optionAlbumViewPrefix = "AlbumView";
        modelCollection       = 0;
    }

    QString                       userPresentableAlbumTitle(const QString& album);

    bool                          needDispatchSelection;
    bool                          cancelSlideShow;
    bool                          useAlbumHistory;

    int                           initialAlbumID;
    int                           thumbSize;

    QMainWindow*                  dockArea;

    SidebarSplitter*              splitter;

    QTimer*                       selectionTimer;
    QTimer*                       thumbSizeTimer;

    // left side bar
    AlbumFolderViewSideBarWidget* albumFolderSideBar;
    TagViewSideBarWidget*         tagViewSideBar;
    DateFolderViewSideBarWidget*  dateViewSideBar;
    TimelineSideBarWidget*        timelineSideBar;
    SearchSideBarWidget*          searchSideBar;
    FuzzySearchSideBarWidget*     fuzzySearchSideBar;

    GPSSearchSideBarWidget*       gpsSearchSideBar;

    PeopleSideBarWidget*          peopleSideBar;

    DigikamApp*                   parent;

    DigikamImageView*             iconView;
    MapWidgetView*                mapView;
    AlbumManager*                 albumManager;
    AlbumHistory*                 albumHistory;
    AlbumWidgetStack*             albumWidgetStack;

    AlbumModificationHelper*      albumModificationHelper;
    TagModificationHelper*        tagModificationHelper;
    SearchModificationHelper*     searchModificationHelper;

    Sidebar*                      leftSideBar;
    ImagePropertiesSideBarDB*     rightSideBar;

    TagFilterSideBarWidget*       tagFilterWidget;
    ImagePropertiesVersionsTab*   versionsTabWidget;

    QString                       optionAlbumViewPrefix;

    QList<SidebarWidget*>         leftSideBarWidgets;

    DigikamModelCollection*       modelCollection;
};

DigikamView::DigikamView(QWidget* parent, DigikamModelCollection* modelCollection)
           : KHBox(parent), d(new DigikamViewPriv)
{
    d->parent          = static_cast<DigikamApp*>(parent);
    d->modelCollection = modelCollection;
    d->albumManager    = AlbumManager::instance();

    d->albumModificationHelper  = new AlbumModificationHelper(this, this);
    d->tagModificationHelper    = new TagModificationHelper(this, this);
    d->searchModificationHelper = new SearchModificationHelper(this, this);

    d->splitter = new SidebarSplitter;
    d->splitter->setFrameStyle( QFrame::NoFrame );
    d->splitter->setFrameShadow( QFrame::Plain );
    d->splitter->setFrameShape( QFrame::NoFrame );
    d->splitter->setOpaqueResize(false);

    d->leftSideBar = new Sidebar(this, d->splitter, KMultiTabBar::Left);
    d->leftSideBar->setObjectName("Digikam Left Sidebar");
    d->splitter->setParent(this);

    // The dock area where the thumbnail bar is allowed to go.
    d->dockArea = new QMainWindow(this, Qt::Widget);
    d->splitter->addWidget(d->dockArea);
    d->albumWidgetStack = new AlbumWidgetStack(d->dockArea);
    d->dockArea->setCentralWidget(d->albumWidgetStack);
    d->albumWidgetStack->setDockArea(d->dockArea);

    d->iconView = d->albumWidgetStack->imageIconView();
    d->mapView = d->albumWidgetStack->mapWidgetView();

    d->rightSideBar = new ImagePropertiesSideBarDB(this, d->splitter, KMultiTabBar::Right, true);
    d->rightSideBar->setObjectName("Digikam Right Sidebar");

    // album folder view
    d->albumFolderSideBar = new AlbumFolderViewSideBarWidget(d->leftSideBar,
                                    d->modelCollection->getAlbumModel(),
                                    d->albumModificationHelper);
    d->leftSideBarWidgets << d->albumFolderSideBar;
    connect(d->albumFolderSideBar, SIGNAL(signalFindDuplicatesInAlbum(Album*)),
            this, SLOT(slotNewDuplicatesSearch(Album*)));

    // date view
    d->dateViewSideBar = new DateFolderViewSideBarWidget(d->leftSideBar,
                    d->modelCollection->getDateAlbumModel(),
                    d->iconView->imageAlbumFilterModel());
    d->leftSideBarWidgets << d->dateViewSideBar;

    // Tags sidebar tab contents.
    d->tagViewSideBar = new TagViewSideBarWidget(d->leftSideBar,
                    d->modelCollection->getTagModel());
    d->leftSideBarWidgets << d->tagViewSideBar;
    connect(d->tagViewSideBar, SIGNAL(signalFindDuplicatesInAlbum(Album*)),
            this, SLOT(slotNewDuplicatesSearch(Album*)));

    // timeline side bar
    d->timelineSideBar = new TimelineSideBarWidget(d->leftSideBar,
                    d->modelCollection->getSearchModel(),
                    d->searchModificationHelper);
    d->leftSideBarWidgets << d->timelineSideBar;

    // Search sidebar tab contents.
    d->searchSideBar = new SearchSideBarWidget(d->leftSideBar,
                    d->modelCollection->getSearchModel(),
                    d->searchModificationHelper);
    d->leftSideBarWidgets << d->searchSideBar;

    // Fuzzy search
    d->fuzzySearchSideBar = new FuzzySearchSideBarWidget(d->leftSideBar,
                    d->modelCollection->getSearchModel(),
                    d->searchModificationHelper);
    d->leftSideBarWidgets << d->fuzzySearchSideBar;

    d->gpsSearchSideBar = new GPSSearchSideBarWidget(d->leftSideBar,
                    d->modelCollection->getSearchModel(),
                    d->searchModificationHelper, 
                    d->iconView->imageFilterModel(),d->iconView->getSelectionModel());

    d->leftSideBarWidgets << d->gpsSearchSideBar;

    // People Sidebar
    d->peopleSideBar = new PeopleSideBarWidget(d->leftSideBar,
                    d->modelCollection->getTagModel(),
                    d->searchModificationHelper);
    connect(d->peopleSideBar, SIGNAL(requestFaceMode(bool)),
            d->iconView, SLOT(setFaceMode(bool)));

    d->leftSideBarWidgets << d->peopleSideBar;

    foreach(SidebarWidget *leftWidget, d->leftSideBarWidgets)
    {
        d->leftSideBar->appendTab(leftWidget, leftWidget->getIcon(),
                        leftWidget->getCaption());
        connect(leftWidget, SIGNAL(requestActiveTab(SidebarWidget*)),
                this, SLOT(slotLeftSideBarActivate(SidebarWidget*)));
    }

    // To the right.

    // Tags Filter sidebar tab contents.
    d->tagFilterWidget = new TagFilterSideBarWidget(d->rightSideBar, d->modelCollection->getTagFilterModel());
    d->rightSideBar->appendTab(d->tagFilterWidget, SmallIcon("tag-assigned"), i18n("Tag Filters"));

    d->selectionTimer = new QTimer(this);
    d->selectionTimer->setSingleShot(true);
    d->selectionTimer->setInterval(75);
    d->thumbSizeTimer = new QTimer(this);
    d->thumbSizeTimer->setSingleShot(true);
    d->thumbSizeTimer->setInterval(300);

    d->albumHistory = new AlbumHistory();

    slotSidebarTabTitleStyleChanged();
    setupConnections();
}

DigikamView::~DigikamView()
{
    saveViewState();

    delete d->albumHistory;
    delete d;
}

void DigikamView::applySettings()
{
    foreach(SidebarWidget *sidebarWidget, d->leftSideBarWidgets)
    {
        sidebarWidget->applySettings();
    }

    refreshView();
}

void DigikamView::refreshView()
{
    d->rightSideBar->refreshTagsView();
}

void DigikamView::setupConnections()
{
    // -- DigikamApp connections ----------------------------------

    connect(d->parent, SIGNAL(signalEscapePressed()),
            this, SLOT(slotEscapePreview()));

    connect(d->parent, SIGNAL(signalEscapePressed()),
            d->albumWidgetStack, SLOT(slotEscapePreview()));

    connect(d->parent, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->parent, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->parent, SIGNAL(signalFirstItem()),
            this, SLOT(slotFirstItem()));

    connect(d->parent, SIGNAL(signalLastItem()),
            this, SLOT(slotLastItem()));

    connect(d->parent, SIGNAL(signalCutAlbumItemsSelection()),
            d->iconView, SLOT(cut()));

    connect(d->parent, SIGNAL(signalCopyAlbumItemsSelection()),
            d->iconView, SLOT(copy()));

    connect(d->parent, SIGNAL(signalPasteAlbumItemsSelection()),
            d->iconView, SLOT(paste()));

    connect(this, SIGNAL(signalProgressBarMode(int, const QString&)),
            d->parent, SLOT(slotProgressBarMode(int, const QString&)));

    connect(this, SIGNAL(signalProgressValue(int)),
            d->parent, SLOT(slotProgressValue(int)));

    connect(d->parent, SIGNAL(signalCancelButtonPressed()),
            this, SLOT(slotCancelSlideShow()));

    // -- AlbumManager connections --------------------------------

    connect(d->albumManager, SIGNAL(signalAlbumCurrentChanged(Album*)),
            this, SLOT(slotAlbumSelected(Album*)));

    connect(d->albumManager, SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(slotAllAlbumsLoaded()));

    connect(d->albumManager, SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(d->albumManager, SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(d->albumManager, SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    connect(d->albumManager, SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    // -- IconView Connections -------------------------------------

    connect(d->iconView->model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(slotImageSelected()));

    connect(d->iconView->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            this, SLOT(slotImageSelected()));

    connect(d->iconView->model(), SIGNAL(layoutChanged()),
            this, SLOT(slotImageSelected()));

    connect(d->iconView, SIGNAL(selectionChanged()),
            this, SLOT(slotImageSelected()));

    connect(d->iconView, SIGNAL(previewRequested(const ImageInfo &)),
            this, SLOT(slotTogglePreviewMode(const ImageInfo &)));

    connect(d->iconView, SIGNAL(gotoAlbumAndImageRequested(const ImageInfo&)),
            this, SLOT(slotGotoAlbumAndItem(const ImageInfo&)));

    connect(d->iconView, SIGNAL(gotoDateAndImageRequested(const ImageInfo&)),
            this, SLOT(slotGotoDateAndItem(const ImageInfo&)));

    connect(d->iconView, SIGNAL(gotoTagAndImageRequested(int)),
            this, SLOT(slotGotoTagAndItem(int)));

    connect(d->iconView, SIGNAL(zoomOutStep()),
            this, SLOT(slotZoomOut()));

    connect(d->iconView, SIGNAL(zoomInStep()),
            this, SLOT(slotZoomIn()));

    // -- Sidebar Connections -------------------------------------

    connect(d->leftSideBar, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotLeftSidebarChangedTab(QWidget*)));

    connect(d->rightSideBar, SIGNAL(signalFirstItem()),
            this, SLOT(slotFirstItem()));

    connect(d->rightSideBar, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->rightSideBar, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->rightSideBar, SIGNAL(signalLastItem()),
            this, SLOT(slotLastItem()));

    connect(this, SIGNAL(signalNoCurrentItem()),
            d->rightSideBar, SLOT(slotNoCurrentItem()));

    connect(d->rightSideBar, SIGNAL(signalProgressBarMode(int, const QString&)),
            d->parent, SLOT(slotProgressBarMode(int, const QString&)));

    connect(d->rightSideBar, SIGNAL(signalProgressValue(int)),
            d->parent, SLOT(slotProgressValue(int)));

    connect(d->fuzzySearchSideBar, SIGNAL(signalUpdateFingerPrints()),
            d->parent, SLOT(slotRebuildFingerPrints()));

    connect(d->fuzzySearchSideBar, SIGNAL(signalGenerateFingerPrintsFirstTime()),
            d->parent, SLOT(slotGenerateFingerPrintsFirstTime()));

    connect(d->peopleSideBar, SIGNAL(signalDetectFaces()),
            d->parent, SLOT(slotScanForFaces()));
    
    /*connect(d->fuzzySearchSideBar, SIGNAL(signalGenerateFingerPrintsFirstTime()),
            d->parent, SLOT(slotGenerateFingerPrintsFirstTime()));*/
    
    connect(this, SIGNAL(signalNoCurrentItem()),
            d->gpsSearchSideBar, SLOT(slotDigikamViewNoCurrentItem()));

    connect(this, SIGNAL(signalImageSelected(const ImageInfoList&, bool, bool, const ImageInfoList&)),
            d->gpsSearchSideBar, SLOT(slotDigikamViewImageSelected(const ImageInfoList&, bool, bool, const ImageInfoList&)));

    connect(d->gpsSearchSideBar, SIGNAL(signalMapSelectedItems(const KUrl::List)),
            d->iconView, SLOT(setSelectedUrls(const KUrl::List&)));

//    connect(d->gpsSearchSideBar, SIGNAL(signalMapSoloItems(const KUrl::List, const QString&)),
//            d->iconView->imageFilterModel(), SLOT(setUrlWhitelist(const KUrl::List, const QString&)));

     connect(d->gpsSearchSideBar, SIGNAL(signalMapSoloItems(const QList<qlonglong>&, const QString&)),
             d->iconView->imageFilterModel(), SLOT(setIdWhitelist(const QList<qlonglong>&, const QString&))); 

    // -- Filter Bars Connections ---------------------------------

    connect(d->tagFilterWidget,
            SIGNAL(tagFilterChanged(const QList<int>&, ImageFilterSettings::MatchingCondition, bool)),
            d->iconView->imageFilterModel(),
            SLOT(setTagFilter(const QList<int>&, ImageFilterSettings::MatchingCondition, bool)));

    // -- Preview image widget Connections ------------------------

    connect(d->albumWidgetStack, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->albumWidgetStack, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->albumWidgetStack, SIGNAL(signalEditItem()),
            this, SLOT(slotImageEdit()));

    connect(d->albumWidgetStack, SIGNAL(signalDeleteItem()),
            this, SLOT(slotImageDelete()));

    connect(d->albumWidgetStack, SIGNAL(signalToggledToPreviewMode(bool)),
            this, SLOT(slotToggledToPreviewMode(bool)));

    connect(d->albumWidgetStack, SIGNAL(signalBack2Album()),
            this, SLOT(slotEscapePreview()));

    connect(d->albumWidgetStack, SIGNAL(signalSlideShow()),
            this, SLOT(slotSlideShowAll()));

    connect(d->albumWidgetStack, SIGNAL(signalZoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged(double)));

    connect(d->albumWidgetStack, SIGNAL(signalInsert2LightTable()),
            this, SLOT(slotImageAddToLightTable()));

    connect(d->albumWidgetStack, SIGNAL(signalInsert2QueueMgr()),
            this, SLOT(slotImageAddToCurrentQueue()));

    connect(d->albumWidgetStack, SIGNAL(signalFindSimilar()),
            this, SLOT(slotImageFindSimilar()));

    connect(d->albumWidgetStack, SIGNAL(signalImageSelected(const ImageInfo&)),
            d->iconView, SLOT(setCurrentInfo(const ImageInfo&)));

    connect(d->albumWidgetStack, SIGNAL(signalAddToExistingQueue(int)),
            this, SLOT(slotImageAddToExistingQueue(int)));

    connect(d->albumWidgetStack, SIGNAL(signalGotoAlbumAndItem(const ImageInfo&)),
            this, SLOT(slotGotoAlbumAndItem(const ImageInfo&)));

    connect(d->albumWidgetStack, SIGNAL(signalGotoDateAndItem(const ImageInfo&)),
            this, SLOT(slotGotoDateAndItem(const ImageInfo&)));

    connect(d->albumWidgetStack, SIGNAL(signalGotoTagAndItem(int)),
            this, SLOT(slotGotoTagAndItem(int)));

    // -- MetadataManager progress ---------------

    connect(MetadataManager::instance(), SIGNAL(progressMessageChanged(const QString&)),
            this, SLOT(slotProgressMessageChanged(const QString&)));

    connect(MetadataManager::instance(), SIGNAL(progressValueChanged(float)),
            this, SLOT(slotProgressValueChanged(float)));

    connect(MetadataManager::instance(), SIGNAL(progressFinished()),
            this, SLOT(slotProgressFinished()));

    connect(MetadataManager::instance(), SIGNAL(orientationChangeFailed(const QStringList&)),
            this, SLOT(slotOrientationChangeFailed(const QStringList&)));

    // -- timers ---------------

    connect(d->selectionTimer, SIGNAL(timeout()),
            this, SLOT(slotDispatchImageSelected()));

    connect(d->thumbSizeTimer, SIGNAL(timeout()),
            this, SLOT(slotThumbSizeEffect()) );

    // -- Album Settings ----------------

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSidebarTabTitleStyleChanged()));

    // -- Album History -----------------
    connect(this, SIGNAL(signalAlbumSelected(bool)),
            d->albumHistory, SLOT(slotAlbumSelected()));

    connect(this, SIGNAL(signalImageSelected(const ImageInfoList&, bool, bool, const ImageInfoList&)),
            d->albumHistory, SLOT(slotImageSelected(const ImageInfoList&)));

    connect(d->iconView, SIGNAL(currentChanged(const ImageInfo&)),
            d->albumHistory, SLOT(slotCurrentChange(const ImageInfo&)));

    connect(d->iconView, SIGNAL(gotoAlbumAndImageRequested(const ImageInfo&)),
            d->albumHistory, SLOT(slotClearSelectPAlbum(const ImageInfo&)));

    connect(d->iconView, SIGNAL(gotoTagAndImageRequested(int)),
            d->albumHistory, SLOT(slotClearSelectTAlbum(int)));

    connect(d->iconView->imageModel(), SIGNAL(imageInfosAdded(QList<ImageInfo>)),
            d->albumHistory, SLOT(slotAlbumCurrentChanged()));

    connect(d->albumHistory, SIGNAL(signalSetCurrent(qlonglong)),
            d->iconView, SLOT(setCurrentWhenAvailable(qlonglong)));

    connect(d->albumHistory, SIGNAL(signalSetSelectedUrls(KUrl::List)),
            d->iconView, SLOT(setSelectedUrls(KUrl::List)));

    connect(d->albumManager, SIGNAL(signalAlbumDeleted(Album*)),
            d->albumHistory, SLOT(slotAlbumDeleted(Album*)));

    // -- Image versions ----------------
    connect(this, SIGNAL(signalImageSelected(const ImageInfoList&, bool, bool, const ImageInfoList&)),
            d->rightSideBar->getFiltersHistoryTab(), SLOT(slotDigikamViewImageSelected(const ImageInfoList&, bool, bool, const ImageInfoList&)));

    connect(this, SIGNAL(signalNoCurrentItem()),
            d->rightSideBar->getFiltersHistoryTab(), SLOT(slotDigikamViewNoCurrentItem()));

    connect(d->rightSideBar->getFiltersHistoryTab(), SIGNAL(setCurrentUrlSignal(KUrl)),
            d->iconView, SLOT(setCurrentUrl(KUrl)));

    connect(d->rightSideBar->getFiltersHistoryTab(), SIGNAL(updateMainViewSignal()),
            d->iconView->imageAlbumModel(), SLOT(refresh()));
    
    connect(d->rightSideBar->getFiltersHistoryTab(), SIGNAL(setCurrentIdSignal(qlonglong)),
            d->iconView, SLOT(setCurrentWhenAvailable(qlonglong)));
}

void DigikamView::connectIconViewFilter(AlbumIconViewFilter *filter)
{
    ImageAlbumFilterModel *model = d->iconView->imageAlbumFilterModel();

    connect(filter, SIGNAL(ratingFilterChanged(int, ImageFilterSettings::RatingCondition)),
            model, SLOT(setRatingFilter(int, ImageFilterSettings::RatingCondition)));

    connect(filter, SIGNAL(mimeTypeFilterChanged(int)),
            model, SLOT(setMimeTypeFilter(int)));

    connect(filter, SIGNAL(textFilterChanged(const SearchTextSettings&)),
            model, SLOT(setTextFilter(const SearchTextSettings&)));

    connect(model, SIGNAL(filterMatches(bool)),
            filter, SLOT(slotFilterMatches(bool)));

    connect(model, SIGNAL(filterMatchesForText(bool)),
            filter, SLOT(slotFilterMatchesForText(bool)));

    connect(model, SIGNAL(filterSettingsChanged(const ImageFilterSettings&)),
            filter, SLOT(slotFilterSettingsChanged(const ImageFilterSettings&)));

    connect(filter, SIGNAL(resetTagFilters()),
            d->tagFilterWidget, SLOT(slotResetTagFilters()));
}

void DigikamView::loadViewState()
{

    foreach(SidebarWidget *widget, d->leftSideBarWidgets)
    {
        widget->loadState();
    }

    d->tagFilterWidget->loadState();

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("MainWindow");

    // Restore the splitter
    d->splitter->restoreState(group);

    // Restore the thumbnail bar dock.
    QByteArray thumbbarState;
    thumbbarState = group.readEntry("ThumbbarState", thumbbarState);
    d->dockArea->restoreState(QByteArray::fromBase64(thumbbarState));

    d->initialAlbumID = group.readEntry("InitialAlbumID", 0);

    MapWidgetView* mapView = d->albumWidgetStack->mapWidgetView();
    mapView->doLoadState();
}

void DigikamView::saveViewState()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("MainWindow");

    foreach(SidebarWidget *widget, d->leftSideBarWidgets)
    {
        widget->saveState();
    }

    d->tagFilterWidget->saveState();

    // Save the splitter states.
    d->splitter->saveState(group);

    // Save the position and size of the thumbnail bar. The thumbnail bar dock
    // needs to be closed explicitly, because when it is floating and visible
    // (when the user is in image preview mode) when the layout is saved, it
    // also reappears when restoring the view, while it should always be hidden.
    d->albumWidgetStack->thumbBarDock()->close();
    group.writeEntry("ThumbbarState", d->dockArea->saveState().toBase64());

    Album *album = AlbumManager::instance()->currentAlbum();
    if(album)
    {
        group.writeEntry("InitialAlbumID", album->globalID());
    }
    else
    {
        group.writeEntry("InitialAlbumID", 0);
    }

    MapWidgetView* mapView = d->albumWidgetStack->mapWidgetView();
    mapView->doSaveState();
}

KUrl::List DigikamView::allUrls() const
{
    return d->iconView->urls();
}

KUrl::List DigikamView::selectedUrls() const
{
    return d->iconView->selectedUrls();
}

void DigikamView::showSideBars()
{
    d->leftSideBar->restore();
    d->rightSideBar->restore();
}

void DigikamView::hideSideBars()
{
    d->leftSideBar->backup();
    d->rightSideBar->backup();
}

void DigikamView::slotFirstItem()
{
    d->iconView->toFirstIndex();
}

void DigikamView::slotPrevItem()
{
    d->iconView->toPreviousIndex();
}

void DigikamView::slotNextItem()
{
    d->iconView->toNextIndex();
}

void DigikamView::slotLastItem()
{
    d->iconView->toLastIndex();
}

void DigikamView::slotSelectItemByUrl(const KUrl& url)
{
    d->iconView->toIndex(url);
}

void DigikamView::slotAllAlbumsLoaded()
{
    disconnect(d->albumManager, SIGNAL(signalAllAlbumsLoaded()),
               this, SLOT(slotAllAlbumsLoaded()));

    loadViewState();
    d->leftSideBar->loadState();
    d->rightSideBar->loadState();
    d->rightSideBar->populateTags();

    // now that all albums have been loaded, activate the albumHistory
    d->useAlbumHistory = true;
    Album *album = d->albumManager->findAlbum(d->initialAlbumID);
    d->albumManager->setCurrentAlbum(album);
}

void DigikamView::slotSortAlbums(int order)
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings)
        return;
    settings->setAlbumSortOrder((AlbumSettings::AlbumSortOrder) order);
    // TODO sorting by anything else then the name is currently not supported by the model
    //d->folderView->resort();
}

void DigikamView::slotNewAlbum()
{
    // TODO use the selection model of the view instead
    d->albumModificationHelper->slotAlbumNew(d->albumFolderSideBar->currentAlbum());
}

void DigikamView::slotDeleteAlbum()
{
    d->albumModificationHelper->slotAlbumDelete(d->albumFolderSideBar->currentAlbum());
}

void DigikamView::slotNewTag()
{
    d->tagModificationHelper->slotTagNew(d->tagViewSideBar->currentAlbum());
}

void DigikamView::slotDeleteTag()
{
    d->tagModificationHelper->slotTagDelete(d->tagViewSideBar->currentAlbum());
}

void DigikamView::slotEditTag()
{
    d->tagModificationHelper->slotTagEdit(d->tagViewSideBar->currentAlbum());
}

void DigikamView::slotNewKeywordSearch()
{
    slotLeftSideBarActivate(d->searchSideBar);
    d->searchSideBar->newKeywordSearch();
}

void DigikamView::slotNewAdvancedSearch()
{
    slotLeftSideBarActivate(d->searchSideBar);
    d->searchSideBar->newAdvancedSearch();
}

void DigikamView::slotNewDuplicatesSearch(Album* album)
{
    slotLeftSideBarActivate(d->fuzzySearchSideBar);
    d->fuzzySearchSideBar->newDuplicatesSearch(album);
}

void DigikamView::slotAlbumAdded(Album *album)
{
    Q_UNUSED(album);
    // right now nothing has to be done here anymore
}

void DigikamView::slotAlbumDeleted(Album *album)
{
    d->albumHistory->deleteAlbum(album);
}

void DigikamView::slotAlbumRenamed(Album *album)
{
    Q_UNUSED(album);
}

void DigikamView::slotAlbumsCleared()
{
    emit signalAlbumSelected(false);
}

void DigikamView::slotAlbumHistoryBack(int steps)
{
    Album *album    = 0;
    QWidget *widget = 0;

    d->albumHistory->back(&album, &widget, steps);

    changeAlbumFromHistory(album, widget);
}

void DigikamView::slotAlbumHistoryForward(int steps)
{
    Album *album    = 0;
    QWidget *widget = 0;

    d->albumHistory->forward(&album, &widget, steps);

    changeAlbumFromHistory(album, widget);
}

// TODO update, use SideBarWidget instead of QWidget
void DigikamView::changeAlbumFromHistory(Album *album, QWidget *widget)
{
    if (album && widget)
    {

        // TODO update, temporary casting until signature is changed
        SidebarWidget *sideBarWidget = dynamic_cast<SidebarWidget*> (widget);
        if (sideBarWidget)
        {
            sideBarWidget->changeAlbumFromHistory(album);
            slotLeftSideBarActivate(sideBarWidget);
        }

        d->parent->enableAlbumBackwardHistory(d->useAlbumHistory && !d->albumHistory->isBackwardEmpty());
        d->parent->enableAlbumForwardHistory(d->useAlbumHistory && !d->albumHistory->isForwardEmpty());
    }
}

void DigikamView::clearHistory()
{
    d->albumHistory->clearHistory();
    d->parent->enableAlbumBackwardHistory(false);
    d->parent->enableAlbumForwardHistory(false);
}

void DigikamView::getBackwardHistory(QStringList& titles)
{
    d->albumHistory->getBackwardHistory(titles);
    for (int i = 0; i < titles.size(); ++i)
    {
        titles[i] = d->userPresentableAlbumTitle(titles[i]);
    }
}

void DigikamView::getForwardHistory(QStringList& titles)
{
    d->albumHistory->getForwardHistory(titles);
    for (int i = 0; i < titles.size(); ++i)
    {
        titles[i] = d->userPresentableAlbumTitle(titles[i]);
    }
}

QString DigikamView::DigikamViewPriv::userPresentableAlbumTitle(const QString& title)
{
    if (title == SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarSketchSearch))
        return i18n("Fuzzy Sketch Search");
    else if (title == SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarImageSearch))
        return i18n("Fuzzy Image Search");
    else if (title == SAlbum::getTemporaryTitle(DatabaseSearch::MapSearch))
        return i18n("Map Search");
    else if (title == SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch) ||
             title == SAlbum::getTemporaryTitle(DatabaseSearch::KeywordSearch))
        return i18n("Last Search");
    else if (title == SAlbum::getTemporaryTitle(DatabaseSearch::TimeLineSearch))
        return i18n("Timeline");
    return title;
}

void DigikamView::slotGotoAlbumAndItem(const ImageInfo& imageInfo)
{

    kDebug() << "going to " << imageInfo;

    emit signalNoCurrentItem();

    PAlbum* album = AlbumManager::instance()->findPAlbum(imageInfo.albumId());

    d->albumFolderSideBar->setCurrentAlbum(album);
    slotLeftSideBarActivate(d->albumFolderSideBar);

    // Set the activate item url to find in the Album View after
    // all items have be reloaded.
    d->iconView->setCurrentWhenAvailable(imageInfo.id());

    // And finally toggle album manager to handle album history and
    // reload all items.
    d->albumManager->setCurrentAlbum(album);

}

void DigikamView::slotGotoDateAndItem(const ImageInfo& imageInfo)
{
    QDate date = imageInfo.dateTime().date();

    emit signalNoCurrentItem();

    // Change to Date Album view.
    // Note, that this also opens the side bar if it is closed; this is
    // considered as a feature, because it highlights that the view was changed.
    slotLeftSideBarActivate(d->dateViewSideBar);

    // Set the activate item url to find in the Album View after
    // all items have be reloaded.
    d->iconView->setCurrentWhenAvailable(imageInfo.id());

    // Change the year and month of the iconItem (day is unused).
    d->dateViewSideBar->gotoDate(date);
}

void DigikamView::slotGotoTagAndItem(int tagID)
{
    // FIXME: Arnd: don't know yet how to get the iconItem passed through ...
    //  then we would know how to use the following ...
    //  KURL url( iconItem->imageInfo()->kurl() );
    //  url.cleanPath();

    emit signalNoCurrentItem();

    // Change to Tag Folder view.
    // Note, that this also opens the side bar if it is closed; this is
    // considered as a feature, because it highlights that the view was changed.
    slotLeftSideBarActivate(d->tagViewSideBar);

    // Set the current tag in the tag folder view.
    // TODO this slot should use a TAlbum pointer directly
    TAlbum *tag = AlbumManager::instance()->findTAlbum(tagID);
    if (tag)
    {
        d->tagViewSideBar->setCurrentAlbum(tag);
    }
    else
    {
        kError() << "Could not find a tag album for tag id " << tagID;
    }

    // Set the activate item url to find in the Tag View after
    // all items have be reloaded.
    // FIXME: see above
    // d->iconView->setAlbumItemToFind(url);
}

void DigikamView::slotSelectAlbum(const KUrl &url)
{

    PAlbum *album = d->albumManager->findPAlbum(url);

    if (!album)
    {
        kWarning() << "Unable to find album for " << url;
        return;
    }

    slotLeftSideBarActivate(d->albumFolderSideBar);
    d->albumFolderSideBar->setCurrentAlbum(album);

}

void DigikamView::slotAlbumSelected(Album* album)
{
    emit signalNoCurrentItem();

    if (!album)
    {
        d->iconView->openAlbum(0);
        d->mapView->openAlbum(0);
        emit signalAlbumSelected(false);
        emit signalTagSelected(false);
        return;
    }

    if (album->type() == Album::PHYSICAL)
    {
        emit signalAlbumSelected(true);
        emit signalTagSelected(false);
    }
    else if (album->type() == Album::TAG)
    {
        emit signalAlbumSelected(false);
        /*
        
        kDebug()<<"Album "<<album->title()<<" selected." ;
        
        // Now loop through children of the people album and check if this album is a child.
        Album* peopleAlbum = AlbumManager::instance()->findTAlbum(TagsCache::instance()->tagForPath("/People"));
        int thisAlbumId = album->id();
        
        QList<int> children =  peopleAlbum->childAlbumIds(true);
        
        foreach(int id, children)
        {
            if(id == thisAlbumId)
            {
                kDebug()<<"Is a people tag";
                
                showFaceAlbum(thisAlbumId);
                emit signalTagSelected(true);
                return;
            }
        }

        */
        emit signalTagSelected(true);
    }

    if (d->useAlbumHistory)
    {
        d->albumHistory->addAlbum(album, d->leftSideBar->getActiveTab());
    }
    d->parent->enableAlbumBackwardHistory(d->useAlbumHistory && !d->albumHistory->isBackwardEmpty());
    d->parent->enableAlbumForwardHistory(d->useAlbumHistory && !d->albumHistory->isForwardEmpty());

    d->iconView->openAlbum(album);
    d->mapView->openAlbum(album);

        if (album->isRoot())
            d->albumWidgetStack->setPreviewMode(AlbumWidgetStack::WelcomePageMode);
        else
        {
            if(d->albumWidgetStack->previewMode() != AlbumWidgetStack::MapWidgetMode)
                d->albumWidgetStack->setPreviewMode(AlbumWidgetStack::PreviewAlbumMode);
        }
}

/*
void DigikamView::showFaceAlbum ( int tagID )
{
    QString personname = TagsCache::instance()->tagName(tagID);
    
    SearchXmlWriter writer;
    writer.writeGroup();
    writer.writeField ( "imagetagproperty", SearchXml::Equal );
    writer.writeValue ( QStringList() << "face" << personname );
    writer.finishField();
    writer.finishGroup();
        
    SAlbum* salbum = AlbumManager::instance()->createSAlbum ( personname,
                     DatabaseSearch::UnknownFaceSearch, writer.xml() );
    
    // search types defined in albuminfo.h. Can be a better name.
    AlbumManager::instance()->setCurrentAlbum ( salbum );

}

*/

void DigikamView::slotAlbumOpenInKonqui()
{
    Album *album = d->albumManager->currentAlbum();
    if (!album || album->type() != Album::PHYSICAL)
        return;

    PAlbum* palbum = dynamic_cast<PAlbum*>(album);

    new KRun(KUrl(palbum->folderPath()), this); // KRun will delete itself.
}

void DigikamView::slotAlbumOpenInTerminal()
{
    Album *album = d->albumManager->currentAlbum();
    if (!album || album->type() != Album::PHYSICAL)
        return;

    PAlbum* palbum = dynamic_cast<PAlbum*>(album);

    const QString terminalApp("konsole");
    QStringList args;
    args << "--workdir" << palbum->folderPath();
    const bool success = QProcess::startDetached(terminalApp, args);

    if (!success)
    {
        KMessageBox::error(this,
            i18n("Cannot start the \"konsole\" application.\n"
                 "Please make sure that it is installed and in your path."),
            windowTitle()/*i18n("Open Album in Terminal")*/);
    }
}

void DigikamView::slotAlbumRefresh()
{
    // force reloading of thumbnails
    LoadingCacheInterface::cleanThumbnailCache();
    Album *album = d->iconView->currentAlbum();
    // if physical album, schedule a collection scan of current album's path
    if (album && album->type() == Album::PHYSICAL)
        ScanController::instance()->scheduleCollectionScan(static_cast<PAlbum*>(album)->folderPath());
    // force reload. Should normally not be necessary, but we may have bugs
    qlonglong currentId = d->iconView->currentInfo().id();
    d->iconView->imageAlbumModel()->refresh();
    if (currentId != -1)
        d->iconView->setCurrentWhenAvailable(currentId);
}

void DigikamView::slotImageSelected()
{
    // delay to slotDispatchImageSelected
    d->needDispatchSelection = true;
    d->selectionTimer->start();
    emit signalSelectionChanged(d->iconView->numberOfSelectedIndexes());
}

void DigikamView::slotDispatchImageSelected()
{

    if(d->albumWidgetStack->previewMode() == AlbumWidgetStack::MapWidgetMode)
        return;

    if (d->needDispatchSelection)
    {
        // the list of ImageInfos of currently selected items, currentItem first
        ImageInfoList list = d->iconView->selectedImageInfosCurrentFirst();

        ImageInfoList allImages = d->iconView->imageInfos();

        if (list.isEmpty())
        {
            d->albumWidgetStack->setPreviewItem();
            emit signalImageSelected(list, false, false, allImages);
            emit signalNoCurrentItem();
        }
        else
        {
            d->rightSideBar->itemChanged(list);

            ImageInfo previousInfo = d->iconView->previousInfo(list.first());
            ImageInfo nextInfo = d->iconView->nextInfo(list.first());

            if (!d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
                d->albumWidgetStack->setPreviewItem(list.first(), previousInfo, nextInfo);

            emit signalImageSelected(list, !previousInfo.isNull(), !nextInfo.isNull(), allImages);
        }

        d->needDispatchSelection = false;
    }
}

double DigikamView::zoomMin()
{
    return d->albumWidgetStack->zoomMin();
}

double DigikamView::zoomMax()
{
    return d->albumWidgetStack->zoomMax();
}

void DigikamView::setZoomFactor(double zoom)
{
    d->albumWidgetStack->setZoomFactorSnapped(zoom);    
}

void DigikamView::slotZoomFactorChanged(double zoom)
{
    toggleZoomActions();
    emit signalZoomChanged(zoom);
}

void DigikamView::setThumbSize(int size)
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewImageMode)
    {
        double z    = DZoomBar::zoomFromSize(size, zoomMin(), zoomMax());
        setZoomFactor(z);
    }
    else if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
    {
        if (size > ThumbnailSize::Huge)
            d->thumbSize = ThumbnailSize::Huge;
        else if (size < ThumbnailSize::Small)
            d->thumbSize = ThumbnailSize::Small;
        else
            d->thumbSize = size;

        emit signalThumbSizeChanged(d->thumbSize);

        d->thumbSizeTimer->start();
    }
}

void DigikamView::slotThumbSizeEffect()
{
    d->iconView->setThumbnailSize(d->thumbSize);
    toggleZoomActions();

    AlbumSettings::instance()->setDefaultIconSize(d->thumbSize);
}

void DigikamView::toggleZoomActions()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewImageMode)
    {
        d->parent->enableZoomMinusAction(true);
        d->parent->enableZoomPlusAction(true);

        if (d->albumWidgetStack->maxZoom())
            d->parent->enableZoomPlusAction(false);

        if (d->albumWidgetStack->minZoom())
            d->parent->enableZoomMinusAction(false);
    }
    else if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
    {
        d->parent->enableZoomMinusAction(true);
        d->parent->enableZoomPlusAction(true);

        if (d->thumbSize >= ThumbnailSize::Huge)
            d->parent->enableZoomPlusAction(false);

        if (d->thumbSize <= ThumbnailSize::Small)
            d->parent->enableZoomMinusAction(false);
    }
}

void DigikamView::slotZoomIn()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
    {
        setThumbSize(d->thumbSize + ThumbnailSize::Step);
        toggleZoomActions();
        emit signalThumbSizeChanged(d->thumbSize);
    }
    else if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewImageMode)
    {
        d->albumWidgetStack->increaseZoom();
    }
}

void DigikamView::slotZoomOut()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
    {
        setThumbSize(d->thumbSize - ThumbnailSize::Step);
        toggleZoomActions();
        emit signalThumbSizeChanged(d->thumbSize);
    }
    else if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewImageMode)
    {
        d->albumWidgetStack->decreaseZoom();
    }
}

void DigikamView::slotZoomTo100Percents()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewImageMode)
    {
        d->albumWidgetStack->toggleFitToWindowOr100();
    }
}

void DigikamView::slotFitToWindow()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewImageMode)
    {
        d->albumWidgetStack->fitToWindow();
    }
}

void DigikamView::slotAlbumPropsEdit()
{
    d->albumModificationHelper->slotAlbumEdit(d->albumManager->currentPAlbum());
}

void DigikamView::connectBatchSyncMetadata(BatchSyncMetadata *syncMetadata)
{
    connect(syncMetadata, SIGNAL(signalProgressBarMode(int, const QString&)),
            d->parent, SLOT(slotProgressBarMode(int, const QString&)));

    connect(syncMetadata, SIGNAL(signalProgressValue(int)),
            d->parent, SLOT(slotProgressValue(int)));

    //connect(syncMetadata, SIGNAL(signalComplete()),
    //      this, SLOT(slotAlbumSyncPicturesMetadataDone()));

    connect(d->parent, SIGNAL(signalCancelButtonPressed()),
            syncMetadata, SLOT(slotAbort()));
}

void DigikamView::slotAlbumWriteMetadata()
{
    Album *album = d->albumManager->currentAlbum();
    if (!album)
        return;

    BatchSyncMetadata *syncMetadata = new BatchSyncMetadata(album, BatchSyncMetadata::WriteFromDatabaseToFile, this);
    connectBatchSyncMetadata(syncMetadata);
    syncMetadata->parseAlbum();
}

void DigikamView::slotAlbumReadMetadata()
{
    Album *album = d->albumManager->currentAlbum();
    if (!album)
        return;

    BatchSyncMetadata *syncMetadata = new BatchSyncMetadata(album, BatchSyncMetadata::ReadFromFileToDatabase, this);
    connectBatchSyncMetadata(syncMetadata);
    syncMetadata->parseAlbum();
}

void DigikamView::slotImageWriteMetadata()
{
    ImageInfoList selected = d->iconView->selectedImageInfos();

    BatchSyncMetadata *syncMetadata = new BatchSyncMetadata(selected, BatchSyncMetadata::WriteFromDatabaseToFile, this);
    connectBatchSyncMetadata(syncMetadata);
    syncMetadata->parseList();
}

void DigikamView::slotImageReadMetadata()
{
    ImageInfoList selected = d->iconView->selectedImageInfos();

    BatchSyncMetadata *syncMetadata = new BatchSyncMetadata(selected, BatchSyncMetadata::ReadFromFileToDatabase, this);
    connectBatchSyncMetadata(syncMetadata);
    syncMetadata->parseList();
}

// ----------------------------------------------------------------

void DigikamView::slotEscapePreview()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode ||
        d->albumWidgetStack->previewMode() == AlbumWidgetStack::WelcomePageMode ||
        d->albumWidgetStack->previewMode() == AlbumWidgetStack::MapWidgetMode)
        return;

    slotTogglePreviewMode(d->iconView->currentInfo());
}

void DigikamView::slotMapWidgetView()
{
    if(d->albumWidgetStack->previewMode() != AlbumWidgetStack::PreviewAlbumMode)
        slotIconView();

    MapWidgetView* mapView = d->albumWidgetStack->mapWidgetView();
    mapView->setActive(true);
    d->albumWidgetStack->setMapViewMode();
}

void DigikamView::slotIconView()
{
    //if it's PreviewImageMode, we close it first. Should I see if it's a movie preview or a welcome page?
    if(d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewImageMode)
    {
        emit signalThumbSizeChanged(d->iconView->thumbnailSize().size());
        emit signalTogglePreview(false);
    }
    
    MapWidgetView* mapView = d->albumWidgetStack->mapWidgetView();
    if(mapView->getActiveState())
        mapView->setActive(false);

    //and switch to icon view
    d->albumWidgetStack->setIconViewMode();
}

void DigikamView::slotImagePreview()
{
    ImageInfo info = d->iconView->currentInfo();
    if (!info.isNull() && (d->albumWidgetStack->previewMode() != AlbumWidgetStack::MapWidgetMode))
        slotTogglePreviewMode(info);
}

// This method toggle between AlbumView and ImagePreview Modes, depending of context.
void DigikamView::slotTogglePreviewMode(const ImageInfo &info)
{
    if(d->albumWidgetStack->previewMode() != AlbumWidgetStack::MapWidgetMode)
    {

        if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode && !info.isNull())
        {
            d->albumWidgetStack->setPreviewItem(info, d->iconView->previousInfo(info), d->iconView->nextInfo(info));
        }
        else
        {
            // We go back to AlbumView Mode.
            d->albumWidgetStack->setPreviewMode( AlbumWidgetStack::PreviewAlbumMode );
        }
    }
}

void DigikamView::slotToggledToPreviewMode(bool b)
{

    if(d->albumWidgetStack->previewMode() != AlbumWidgetStack::MapWidgetMode)
    {

        toggleZoomActions();

        if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
            emit signalThumbSizeChanged(d->iconView->thumbnailSize().size());
        else if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewImageMode)
            slotZoomFactorChanged(d->albumWidgetStack->zoomFactor());

        emit signalTogglePreview(b);
    }

}

void DigikamView::slotImageEdit()
{
    d->iconView->openCurrentInEditor();
}

void DigikamView::slotImageFindSimilar()
{
    ImageInfo current = d->iconView->currentInfo();
    if (!current.isNull())
    {
        d->fuzzySearchSideBar->newSimilarSearch(current);
        slotLeftSideBarActivate(d->fuzzySearchSideBar);
    }
}

void DigikamView::slotImageExifOrientation(int orientation)
{
    d->iconView->setExifOrientationOfSelected(orientation);
}

void DigikamView::slotLightTable()
{
    d->iconView->utilities()->insertToLightTable(ImageInfoList(), ImageInfo(), true);
}

void DigikamView::slotQueueMgr()
{
    d->iconView->utilities()->insertToQueueManager(ImageInfoList(), ImageInfo(), false);
}

void DigikamView::slotImageLightTable()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
    {
        // put images into an emptied light table
        d->iconView->insertSelectedToLightTable(false);
    }
    else
    {
        ImageInfoList list;
        ImageInfo info = d->albumWidgetStack->imagePreviewView()->getImageInfo();
        list.append(info);
        // put images into an emptied light table
        d->iconView->utilities()->insertToLightTable(list, info, false);
    }
}

void DigikamView::slotImageAddToLightTable()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
    {
        // add images to the existing images in the light table
        d->iconView->insertSelectedToLightTable(true);
    }
    else
    {
        ImageInfoList list;
        ImageInfo info = d->albumWidgetStack->imagePreviewView()->getImageInfo();
        list.append(info);
        // add images to the existing images in the light table
        d->iconView->utilities()->insertToLightTable(list, info, true);
    }
}

void DigikamView::slotImageAddToCurrentQueue()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
    {
        d->iconView->insertSelectedToCurrentQueue();
    }
    else
    {
        ImageInfoList list;
        ImageInfo info = d->albumWidgetStack->imagePreviewView()->getImageInfo();
        list.append(info);
        d->iconView->utilities()->insertToQueueManager(list, info, false);
    }
}

void DigikamView::slotImageAddToNewQueue()
{
    bool newQueue = QueueMgrWindow::queueManagerWindowCreated() &&
                   !QueueMgrWindow::queueManagerWindow()->queuesMap().isEmpty();

    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
    {
        if (newQueue)
            d->iconView->insertSelectedToNewQueue();
        else
            d->iconView->insertSelectedToCurrentQueue();
    }
    else
    {
        // FIXME
        ImageInfoList list;
        ImageInfo info = d->albumWidgetStack->imagePreviewView()->getImageInfo();
        list.append(info);
        d->iconView->utilities()->insertToQueueManager(list, info, newQueue);
    }
}

void DigikamView::slotImageAddToExistingQueue(int queueid)
{
    ImageInfoList list;

    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
        list = d->albumWidgetStack->imageIconView()->selectedImageInfos();
    else
        list << d->albumWidgetStack->imagePreviewView()->getImageInfo();

    if (!list.isEmpty())
    {
        d->iconView->utilities()->insertSilentToQueueManager(list, list.first(), queueid);
    }
}

void DigikamView::slotImageRename()
{
    d->iconView->rename();
}

void DigikamView::slotImageDelete()
{
    d->iconView->deleteSelected(false);
}

void DigikamView::slotImageDeletePermanently()
{
    d->iconView->deleteSelected(true);
}

void DigikamView::slotImageDeletePermanentlyDirectly()
{
    d->iconView->deleteSelectedDirectly(false);
}

void DigikamView::slotImageTrashDirectly()
{
    d->iconView->deleteSelectedDirectly(true);
}

void DigikamView::slotSelectAll()
{
    d->iconView->selectAll();
}

void DigikamView::slotSelectNone()
{
    d->iconView->clearSelection();
}

void DigikamView::slotSelectInvert()
{
    d->iconView->invertSelection();
}

void DigikamView::slotSortImages(int sortRole)
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings)
        return;
    settings->setImageSortOrder(sortRole);
    d->iconView->imageFilterModel()->setSortRole((ImageSortSettings::SortRole) sortRole);
}

void DigikamView::slotSortImagesOrder(int order)
{
    d->iconView->imageFilterModel()->setSortOrder((ImageSortSettings::SortOrder) order);
}

void DigikamView::slotGroupImages(int categoryMode)
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings)
        return;
    settings->setImageGroupMode(categoryMode);
    d->iconView->imageFilterModel()->setCategorizationMode((ImageSortSettings::CategorizationMode) categoryMode);
}

void DigikamView::slotMoveSelectionToAlbum()
{
    d->iconView->createNewAlbumForSelected();
}

void DigikamView::slotLeftSidebarChangedTab(QWidget* w)
{

    // TODO update, temporary cast
    SidebarWidget *widget = dynamic_cast<SidebarWidget*> (w);
    foreach(SidebarWidget *sideBarWidget, d->leftSideBarWidgets)
    {
        bool active = (widget && (widget == sideBarWidget));
        sideBarWidget->setActive(active);
    }

}

void DigikamView::slotAssignRating(int rating)
{
    d->iconView->assignRatingToSelected(rating);
}

void DigikamView::slotAssignRatingNoStar()
{
    d->iconView->assignRatingToSelected(0);
}

void DigikamView::slotAssignRatingOneStar()
{
    d->iconView->assignRatingToSelected(1);
}

void DigikamView::slotAssignRatingTwoStar()
{
    d->iconView->assignRatingToSelected(2);
}

void DigikamView::slotAssignRatingThreeStar()
{
    d->iconView->assignRatingToSelected(3);
}

void DigikamView::slotAssignRatingFourStar()
{
    d->iconView->assignRatingToSelected(4);
}

void DigikamView::slotAssignRatingFiveStar()
{
    d->iconView->assignRatingToSelected(5);
}

void DigikamView::slotSlideShowAll()
{
    slideShow(d->iconView->imageInfos());
}

void DigikamView::slotSlideShowSelection()
{
    slideShow(d->iconView->selectedImageInfos());
}

void DigikamView::slotSlideShowRecursive()
{
    Album *album = AlbumManager::instance()->currentAlbum();
    if(album)
    {
        AlbumList albumList;
        albumList.append(album);
        AlbumIterator it(album);
        while (it.current())
        {
            albumList.append(*it);
            ++it;
        }

        ImageInfoAlbumsJob *job = new ImageInfoAlbumsJob;
        connect(job, SIGNAL(signalCompleted(const ImageInfoList&)),
                this, SLOT(slotItemsInfoFromAlbums(const ImageInfoList&)));
        job->allItemsFromAlbums(albumList);
    }
}

void DigikamView::slotItemsInfoFromAlbums(const ImageInfoList& infoList)
{
    ImageInfoList list = infoList;
    slideShow(list);
}

void DigikamView::slideShow(const ImageInfoList& infoList)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");
    bool startWithCurrent     = group.readEntry("SlideShowStartCurrent", false);

    int     i = 0;
    float cnt = (float)infoList.count();
    emit signalProgressBarMode(StatusProgressBar::CancelProgressBarMode,
                               i18np("Preparing slideshow of 1 image. Please wait...","Preparing slideshow of %1 images. Please wait...", infoList.count()));

    SlideShowSettings settings;
    settings.exifRotate           = MetadataSettings::instance()->settings().exifRotate;
    settings.ratingColor          = ThemeEngine::instance()->textSpecialRegColor();
    settings.delay                = group.readEntry("SlideShowDelay", 5) * 1000;
    settings.printName            = group.readEntry("SlideShowPrintName", true);
    settings.printDate            = group.readEntry("SlideShowPrintDate", false);
    settings.printApertureFocal   = group.readEntry("SlideShowPrintApertureFocal", false);
    settings.printExpoSensitivity = group.readEntry("SlideShowPrintExpoSensitivity", false);
    settings.printMakeModel       = group.readEntry("SlideShowPrintMakeModel", false);
    settings.printComment         = group.readEntry("SlideShowPrintComment", false);
    settings.printRating          = group.readEntry("SlideShowPrintRating", false);
    settings.loop                 = group.readEntry("SlideShowLoop", false);

    d->cancelSlideShow = false;
    for (ImageInfoList::const_iterator it = infoList.constBegin();
         !d->cancelSlideShow && (it != infoList.constEnd()) ; ++it)
    {
        ImageInfo info = *it;
        settings.fileList.append(info.fileUrl());
        SlidePictureInfo pictInfo;
        pictInfo.comment   = info.comment();
        pictInfo.rating    = info.rating();
        pictInfo.photoInfo = info.photoInfoContainer();
        settings.pictInfoMap.insert(info.fileUrl(), pictInfo);

        emit signalProgressValue((int)((i++/cnt)*100.0));
        kapp->processEvents();
    }

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());

    if (!d->cancelSlideShow)
    {
        SlideShow *slide = new SlideShow(settings);
        if (startWithCurrent)
        {
            slide->setCurrent(d->iconView->currentUrl());
        }

        slide->show();
    }
}

void DigikamView::slotCancelSlideShow()
{
    d->cancelSlideShow = true;
}

void DigikamView::toggleShowBar(bool b)
{
    d->albumWidgetStack->thumbBarDock()->showThumbBar(b);
}

void DigikamView::setRecurseAlbums(bool recursive)
{
    d->iconView->imageAlbumModel()->setRecurseAlbums(recursive);
}

void DigikamView::setRecurseTags(bool recursive)
{
    d->iconView->imageAlbumModel()->setRecurseTags(recursive);
}

void DigikamView::slotSidebarTabTitleStyleChanged()
{
    d->leftSideBar->setStyle(AlbumSettings::instance()->getSidebarTitleStyle());
    d->rightSideBar->setStyle(AlbumSettings::instance()->getSidebarTitleStyle());
    d->rightSideBar->applySettings();
}

void DigikamView::slotProgressMessageChanged(const QString& descriptionOfAction)
{
    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode, descriptionOfAction);
}

void DigikamView::slotProgressValueChanged(float percent)
{
    emit signalProgressValue(lround(percent * 100));
}

void DigikamView::slotProgressFinished()
{
    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
}

void DigikamView::slotOrientationChangeFailed(const QStringList& failedFileNames)
{
    if (failedFileNames.isEmpty())
        return;

    if (failedFileNames.count() == 1)
    {
        KMessageBox::error(0, i18n("Failed to revise Exif orientation for file %1.",
                                    failedFileNames[0]));
    }
    else
    {
        KMessageBox::errorList(0, i18n("Failed to revise Exif orientation these files:"),
                                        failedFileNames);
    }
}

void DigikamView::slotLeftSideBarActivate(SidebarWidget *widget)
{
    d->leftSideBar->setActiveTab(widget);
}

}  // namespace Digikam
