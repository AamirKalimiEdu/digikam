/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-23-03
 * Description : a tool tip widget witch follow cursor movements 
 *               Tool tip content is displayed without delay.
 * 
 * Copyright (C) 2007 by Gilles Caulier  <caulier dot gilles at gmail dot com>
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

#include <QEvent>
#include <QFrame>
#include <QToolTip>
#include <QMouseEvent>
#include <QLabel>

// Local includes.

#include "dcursortracker.h"

namespace Digikam 
{

DCursorTracker::DCursorTracker(const QString& txt, QWidget *parent)
              : QLabel(txt, 0, Qt::X11BypassWindowManagerHint) 
{
    parent->setMouseTracking(true);
    parent->installEventFilter(this);
    setEnable(true);
}

/**
 * Overload to make sure the widget size is correct
 */
void DCursorTracker::setText(const QString& txt) 
{
	QLabel::setText(txt);
	adjustSize();
}

void DCursorTracker::setEnable(bool b) 
{
    m_enable = b;
}

bool DCursorTracker::eventFilter(QObject *object, QEvent *e) 
{
	QWidget *widget = static_cast<QWidget*>(object);
	
	switch (e->type()) 
    {
        case QEvent::MouseMove: 
        {
            QMouseEvent *event = static_cast<QMouseEvent*>(e);
            if (m_enable && (widget->rect().contains(event->pos()) || 
                            (event->buttons() & Qt::LeftButton))) 
            {
                show();
                move(event->globalPos().x() + 15, event->globalPos().y() - 15);
            }
            else 
            {
                hide();
            }
            break;
        }
    
        case QEvent::MouseButtonRelease: 
        {
            QMouseEvent* event = static_cast<QMouseEvent*>(e);
            if ( !widget->rect().contains(event->pos()) ) 
            {
                hide();
            }
            break;
        }
        
        default:
            break;
	}

	return false;
}


DTipTracker::DTipTracker(const QString& txt, QWidget *parent)
           : DCursorTracker(txt, parent) 
{
	setPalette(QToolTip::palette());
	setFrameStyle(QFrame::Plain | QFrame::Box);
	setLineWidth(1);
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
}
	
} // namespace Digikam
