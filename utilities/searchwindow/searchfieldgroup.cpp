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

// Qt includes

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>

// KDE includes

// Local includes

#include "ddebug.h"
#include "searchgroup.h"
#include "searchfields.h"
#include "searchutilities.h"
#include "searchfieldgroup.h"
#include "searchfieldgroup.moc"

namespace Digikam
{

SearchFieldGroup::SearchFieldGroup(SearchGroup *parent)
    : QWidget(parent)
{
    m_layout = new QGridLayout;
    setLayout(m_layout);
    m_controller = new VisibilityController(this);
    m_controller->setContainerWidget(this);
}

void SearchFieldGroup::addField(SearchField *field)
{
    //FIXME: When all fields added in searchgroup.cpp are implemented, remove
    if (!field) return;

    // initialize widgets in field
    field->setup(m_layout);
    // take care that category labels are not duplicated in two subsequent rows
    if (!m_fields.isEmpty())
        field->setCategoryLabelVisibleFromPreviousField(m_fields.last());
    // add to our list
    m_fields << field;
    // add to visibility controller
    m_controller->addObject(field);
}

void SearchFieldGroup::setLabel(SearchFieldGroupLabel *label)
{
    m_label = label;
    connect(m_label, SIGNAL(clicked()),
            this, SLOT(slotLabelClicked()));
}

SearchField *SearchFieldGroup::fieldForName(const QString &fieldName)
{
    foreach(SearchField *field, m_fields)
    {
        if (field->supportsField(fieldName))
            return field;
    }
    return 0;
}

void SearchFieldGroup::write(SearchXmlWriter &writer)
{
    foreach(SearchField *field, m_fields)
    {
        field->write(writer);
    }
}

void SearchFieldGroup::reset()
{
    foreach(SearchField *field, m_fields)
    {
        field->reset();
    }
}

void SearchFieldGroup::slotLabelClicked()
{
    DDebug() << "slotLabelClicked";
    m_controller->triggerVisibility();
}

// ----------------------------------- //

SearchFieldGroupLabel::SearchFieldGroupLabel(QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *layout = new QGridLayout;

    m_titleLabel = new SearchClickLabel;
    m_titleLabel->setObjectName("SearchFieldGroupLabel_Label");
    m_expandLabel = new QLabel;
    QFrame *hline = new QFrame;
    hline->setFrameStyle(QFrame::HLine | QFrame::Raised);

    layout->addWidget(m_titleLabel, 0, 0);
    layout->addWidget(m_expandLabel, 0, 1);
    layout->setColumnStretch(2, 1);
    layout->addWidget(hline, 1, 0, 1, 3);
    layout->setSpacing(2);
    setLayout(layout);

    connect(m_titleLabel, SIGNAL(leftClicked()),
            this, SIGNAL(clicked()));
}

void SearchFieldGroupLabel::setTitle(const QString &title)
{
    m_title = title;
    m_titleLabel->setText(title);
}

void SearchFieldGroupLabel::displayExpanded()
{
}

void SearchFieldGroupLabel::displayFolded()
{
}

}



