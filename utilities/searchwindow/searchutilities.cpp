/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-14
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

// Qt includes

#include <QAbstractListModel>
#include <QItemDelegate>
#include <QLinearGradient>
#include <QLineEdit>
#include <QListView>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QStyleOption>
#include <QTextEdit>
#include <QTimeLine>
#include <QVBoxLayout>

// KDE includes

#include <kglobalsettings.h>
#include <klocale.h>
#include <kstandardguiitem.h>
#include <kpushbutton.h>

// Local includes

#include "ddebug.h"
#include "constants.h"
#include "albummodel.h"
#include "ratingwidget.h"
#include "themeengine.h"
#include "searchutilities.h"
#include "searchutilities.moc"

namespace Digikam
{


VisibilityController::VisibilityController(QObject *parent)
    : QObject(parent), m_status(Unknown), m_containerWidget(0)
{
}

void VisibilityController::addObject(VisibilityObject *object)
{
    m_objects << object;

    // create clean state
    if (m_status == Unknown)
    {
        if (object->isVisible())
            m_status = Shown;
        else
            m_status = Hidden;
    }

    // set state on object
    if (m_status == Shown || m_status == Showing)
        object->setVisible(true);
    else
        object->setVisible(false);
}

class VisibilityWidgetWrapper : public QObject, public VisibilityObject
{
public:

    VisibilityWidgetWrapper(VisibilityController *parent, QWidget *widget)
        : QObject(parent), m_widget(widget)
    {
    }

    virtual void setVisible(bool visible)
    {
        return m_widget->setVisible(visible);
    }

    virtual bool isVisible()
    {
        return m_widget->isVisible();
    }

    QWidget *m_widget;
};

void VisibilityController::addWidget(QWidget *widget)
{
    addObject(new VisibilityWidgetWrapper(this, widget));
}

void VisibilityController::setContainerWidget(QWidget *widget)
{
    m_containerWidget = widget;
}

void VisibilityController::setVisible(bool shallBeVisible)
{
    if (shallBeVisible)
    {
        if (m_status == Shown || m_status == Showing)
            return;
        m_status = Showing;
        beginStatusChange();
    }
    else
    {
        if (m_status == Hidden || m_status == Hiding)
            return;
        m_status = Hiding;
        beginStatusChange();
    }
}

void VisibilityController::show()
{
    setVisible(true);
}

void VisibilityController::hide()
{
    setVisible(false);
}

void VisibilityController::triggerVisibility()
{
    if (m_status == Shown || m_status == Showing || m_status == Unknown)
        setVisible(false);
    else
        setVisible(true);
}

bool VisibilityController::isVisible() const
{
    if (m_status == Shown || m_status == Showing)
        return true;
    else
        return false;
}

void VisibilityController::beginStatusChange()
{
    allSteps();
}

void VisibilityController::step()
{
    if (m_status == Showing)
    {
        foreach(VisibilityObject *o, m_objects)
        {
            if (!o->isVisible())
            {
                o->setVisible(true);
                return;
            }
        }
    }
    else if (m_status == Hiding)
    {
        foreach(VisibilityObject *o, m_objects)
        {
            if (o->isVisible())
            {
                o->setVisible(false);
                return;
            }
        }
    }
}

void VisibilityController::allSteps()
{
    if (m_status == Showing)
    {
        if (m_containerWidget)
            m_containerWidget->setUpdatesEnabled(false);
        foreach(VisibilityObject *o, m_objects)
            o->setVisible(true);
        if (m_containerWidget)
            m_containerWidget->setUpdatesEnabled(true);
    }
    else if (m_status == Hiding)
    {
        if (m_containerWidget)
            m_containerWidget->setUpdatesEnabled(false);
        foreach(VisibilityObject *o, m_objects)
            o->setVisible(false);
        if (m_containerWidget)
            m_containerWidget->setUpdatesEnabled(true);
    }
}

// ----------------------------------- //

SearchClickLabel::SearchClickLabel(QWidget *parent)
    : QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

SearchClickLabel::SearchClickLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
    setCursor(Qt::PointingHandCursor);
}

void SearchClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    QLabel::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
        event->accept();
    }
}

// ----------------------------------- //

SearchSqueezedClickLabel::SearchSqueezedClickLabel(QWidget *parent)
    : KSqueezedTextLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

SearchSqueezedClickLabel::SearchSqueezedClickLabel(const QString &text, QWidget *parent)
    : KSqueezedTextLabel(text, parent)
{
    setCursor(Qt::PointingHandCursor);
}

void SearchSqueezedClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    KSqueezedTextLabel::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
        event->accept();
    }
}

// ----------------------------------- //

ArrowClickLabel::ArrowClickLabel(QWidget *parent)
    : QWidget(parent),
      m_arrowType(Qt::RightArrow)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_size = 8;
    m_margin = 2;
}

void ArrowClickLabel::setArrowType(Qt::ArrowType type)
{
    m_arrowType = type;
    update();
}

void ArrowClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
    }
}

void ArrowClickLabel::paintEvent(QPaintEvent*)
{
    // Inspired by karrowbutton.cpp,
    //  Copyright (C) 2001 Frerich Raabe <raabe@kde.org>

    QPainter p(this);

    QStyleOptionFrame opt;
    opt.init(this);
    opt.lineWidth    = 2;
    opt.midLineWidth = 0;

    /*
    p.fillRect( rect(), palette().brush( QPalette::Background ) );
    style()->drawPrimitive( QStyle::PE_Frame, &opt, &p, this);
    */

    if (m_arrowType == Qt::NoArrow)
        return;

    if (width() < m_size + m_margin ||
        height() < m_size + m_margin)
        return; // don't draw arrows if we are too small

    unsigned int x = 0, y = 0;
    if (m_arrowType == Qt::DownArrow) {
        x = (width() - m_size) / 2;
        y = height() - (m_size + m_margin);
    } else if (m_arrowType == Qt::UpArrow) {
        x = (width() - m_size) / 2;
        y = m_margin;
    } else if (m_arrowType == Qt::RightArrow) {
        x = width() - (m_size + m_margin);
        y = (height() - m_size) / 2;
    } else { // arrowType == LeftArrow
        x = m_margin;
        y = (height() - m_size) / 2;
    }

    /*
    if (isDown()) {
        x++;
        y++;
    }
    */

    QStyle::PrimitiveElement e = QStyle::PE_IndicatorArrowLeft;
    switch (m_arrowType)
    {
        case Qt::LeftArrow:
            e = QStyle::PE_IndicatorArrowLeft;
            break;
        case Qt::RightArrow:
            e = QStyle::PE_IndicatorArrowRight;
            break;
        case Qt::UpArrow:
            e = QStyle::PE_IndicatorArrowUp;
            break;
        case Qt::DownArrow:
            e = QStyle::PE_IndicatorArrowDown;
            break;
        case Qt::NoArrow:
            break;
    }

    opt.state |= QStyle::State_Enabled;
    opt.rect   = QRect( x, y, m_size, m_size);

    style()->drawPrimitive( e, &opt, &p, this );
}

QSize ArrowClickLabel::sizeHint() const
{
    return QSize(m_size + 2*m_margin, m_size + 2*m_margin);
}

// ----------------------------------- //

// Copied from klineedit_p.h,
// Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>

AnimatedClearButton::AnimatedClearButton(QWidget *parent)
    : QWidget(parent)
{
    m_stayAlwaysVisible = false;
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_timeline = new QTimeLine(200, this);
    m_timeline->setFrameRange(0, 255);
    m_timeline->setCurveShape(QTimeLine::EaseInOutCurve);
    m_timeline->setDirection(QTimeLine::Forward);
    connect(m_timeline, SIGNAL(finished()), this, SLOT(animationFinished()));
    connect(m_timeline, SIGNAL(frameChanged(int)), this, SLOT(update()));

    connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)), this, SLOT(updateAnimationSettings()));
}

QSize AnimatedClearButton::sizeHint () const
{
    QFontMetrics fm(font());
    return QSize(m_pixmap.width(), fm.lineSpacing());
}

bool AnimatedClearButton::stayVisibleWhenAnimatedOut(bool stayVisible)
{
    m_stayAlwaysVisible = stayVisible;
}

void AnimatedClearButton::animateVisible(bool visible)
{
    // skip animation if widget is not visible
    if (!isVisible())
        setDirectlyVisible(visible);

    if (visible) {
        if (m_timeline->direction() == QTimeLine::Forward) {
            return;
        }

        m_timeline->setDirection(QTimeLine::Forward);
        m_timeline->setDuration(150);
        if (!m_stayAlwaysVisible)
            show();
    }
    else {
        if (m_timeline->direction() == QTimeLine::Backward) {
            return;
        }

        m_timeline->setDirection(QTimeLine::Backward);
        m_timeline->setDuration(250);
    }

    if (KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) {
        if (m_timeline->state() != QTimeLine::Running)
            m_timeline->start();
    }
    else
    {
        if (!m_stayAlwaysVisible)
            setVisible(m_timeline->direction() == QTimeLine::Forward);
    }
}

void AnimatedClearButton::setDirectlyVisible(bool visible)
{
    // We dont overload setVisible() here. QWidget::setVisible is virtual,
    // and we dont want to replace it in all occurrences.
    // Most notably, we want to call the QWidget version from animateVisible above.

    if (visible) {
        if (m_timeline->direction() == QTimeLine::Forward) {
            return;
        }

        // these need to be set as paintEvent depends on these values
        m_timeline->setDirection(QTimeLine::Forward);
        m_timeline->setCurrentTime(150);
    } else {
        if (m_timeline->direction() == QTimeLine::Backward) {
            return;
        }

        // these need to be set as paintEvent depends on these values
        m_timeline->setDirection(QTimeLine::Backward);
        m_timeline->setCurrentTime(0);
    }

    if (m_stayAlwaysVisible)
        update();
    else
        setVisible(m_timeline->direction() == QTimeLine::Forward);
}

void AnimatedClearButton::setPixmap(const QPixmap& p)
{
    m_pixmap = p;
}

QPixmap AnimatedClearButton::pixmap()
{
    return m_pixmap;
}

void AnimatedClearButton::updateAnimationSettings()
{
    bool animationsEnabled = KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects;

    // We need to set the current time in the case that we had the clear
    // button shown, for it being painted on the paintEvent(). Otherwise
    // it wont be painted, resulting (m->timeLine->currentTime() == 0) true,
    // and therefore a bad painting. This is needed for the case that we
    // come from a non animated widget and want it animated. (ereslibre)
    if (animationsEnabled && m_timeline->direction() == QTimeLine::Forward) {
        m_timeline->setCurrentTime(150);
    }
}

void AnimatedClearButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    if (KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) {
        if (m_pixmap.isNull() || m_timeline->currentTime() == 0) {
            return;
        }

        QPainter p(this);
        p.setOpacity(m_timeline->currentValue());
        p.drawPixmap((width() - m_pixmap.width()) / 2,
                    (height() - m_pixmap.height()) / 2,
                    m_pixmap);
    } else {
        QPainter p(this);
        p.setOpacity(1); // make sure
        p.drawPixmap((width() - m_pixmap.width()) / 2,
                    (height() - m_pixmap.height()) / 2,
                    m_pixmap);
    }
}

void AnimatedClearButton::animationFinished()
{
    if (m_timeline->direction() == QTimeLine::Forward) {
        update();
    } else {
        if (!m_stayAlwaysVisible)
            hide();
    }
}

void AnimatedClearButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit clicked();
    }
}

// ----------------------------------- //

StyleSheetDebugger::StyleSheetDebugger(QWidget *object)
    : QWidget(0), m_widget(object)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *vbox = new QVBoxLayout;

    m_edit = new QTextEdit;
    vbox->addWidget(m_edit, 1);
    m_okButton = new KPushButton(KStandardGuiItem::ok());
    vbox->addWidget(m_okButton, 0, Qt::AlignRight);

    setLayout(vbox);

    connect(m_okButton, SIGNAL(clicked()),
            this, SLOT(buttonClicked()));

    m_edit->setPlainText(m_widget->styleSheet());

    resize(400, 300);
    show();
}

void StyleSheetDebugger::buttonClicked()
{
    m_widget->setStyleSheet(m_edit->toPlainText());
}


}

