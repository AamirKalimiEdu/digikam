/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-22
 * Description : a control widget for the AdvancedRename utility
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

#include "advancedrenamewidget.moc"

// Qt includes

#include <QAction>
#include <QGridLayout>
#include <QMenu>
#include <QPushButton>
#include <QRegExp>
#include <QToolButton>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdialog.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "advancedrenameinput.h"
#include "dcursortracker.h"
#include "defaultrenameparser.h"
#include "dynamiclayout.h"
#include "rexpanderbox.h"
#include "themeengine.h"

namespace Digikam
{

class AdvancedRenameWidgetPriv
{
    typedef AdvancedRenameWidget::ControlWidgets CWMask;

public:

    AdvancedRenameWidgetPriv() :
        configGroupName("AdvancedRename Widget"),
        configExpandedStateEntry("Options are expanded"),
        configExpandedStateDefault(true),

        tooltipToggleButton(0),
        tokenToolButton(0),
        modifierToolButton(0),
        btnContainer(0),
        tooltipTrackerAlignment(Qt::AlignLeft),
        tooltipTracker(0),
        renameInput(0),
        parser(0),
        optionsLabel(0),
        controlWidgetsMask(AdvancedRenameWidget::DefaultControls)
    {}

    const QString        configGroupName;
    const QString        configExpandedStateEntry;
    bool                 configExpandedStateDefault;

    QToolButton*         tooltipToggleButton;
    QToolButton*         tokenToolButton;
    QToolButton*         modifierToolButton;

    QWidget*             btnContainer;

    Qt::Alignment        tooltipTrackerAlignment;

    DTipTracker*         tooltipTracker;
    AdvancedRenameInput* renameInput;
    Parser*              parser;
    RLabelExpander*      optionsLabel;

    CWMask               controlWidgetsMask;
};

AdvancedRenameWidget::AdvancedRenameWidget(QWidget* parent)
                 : QWidget(parent), d(new AdvancedRenameWidgetPriv)
{
    setupWidgets();
    setParser(new DefaultRenameParser());
}

AdvancedRenameWidget::~AdvancedRenameWidget()
{
    writeSettings();

    // we need to delete it manually, because it has no parent
    delete d->tooltipTracker;

    delete d->parser;
    delete d;
}

QString AdvancedRenameWidget::text() const
{
    return d->renameInput->text();
}

void AdvancedRenameWidget::setText(const QString& text)
{
    d->renameInput->setText(text);
}

void AdvancedRenameWidget::clearText()
{
    d->renameInput->slotClearText();
}

void AdvancedRenameWidget::setTooltipAlignment(Qt::Alignment alignment)
{
    d->tooltipTrackerAlignment = alignment;
    d->tooltipTracker->setTrackerAlignment(alignment);
}

void AdvancedRenameWidget::clear()
{
    d->renameInput->slotClearTextAndHistory();
}

void AdvancedRenameWidget::slotHideToolTipTracker()
{
    d->tooltipToggleButton->setChecked(false);
    slotToolTipButtonToggled(false);
}

QString AdvancedRenameWidget::parse(ParseInformation& info) const
{
    if (!d->parser)
    {
        return QString();
    }

    QString parseString = d->renameInput->text();

    QString parsed;
    parsed = d->parser->parse(parseString, info);

    return parsed;
}

void AdvancedRenameWidget::createToolTip()
{
    QRegExp optionsRegExp("\\|(.*)\\|");
    optionsRegExp.setMinimal(true);


#define MARK_OPTIONS(str)                                                                      \
        str.replace(optionsRegExp, QString("<i><font color=\"%1\">\\1</font></i>")             \
           .arg(ThemeEngine::instance()->textSpecialRegColor().name()))

#define TOOLTIP_HEADER(str)                                                                    \
    do                                                                                         \
    {                                                                                          \
        tooltip += QString("<tr bgcolor=\"%1\"><td colspan=\"2\">"                             \
                           "<nobr><font color=\"%2\"><center><b>")                             \
                           .arg(ThemeEngine::instance()->baseColor().name())                   \
                           .arg(ThemeEngine::instance()->textRegColor().name());               \
        tooltip += QString(str);                                                               \
        tooltip += QString("</b></center></font></nobr></td></tr>");                           \
    } while (0)                                                                                \


#define TOOLTIP_ENTRIES(type, data)                                                            \
    do                                                                                         \
    {                                                                                          \
        foreach (type* t, data)                                                                \
        {                                                                                      \
            foreach (Token* token, t->tokens())                                                \
            {                                                                                  \
                tooltip += QString("<tr>"                                                      \
                                   "<td bgcolor=\"%1\">"                                       \
                                       "<font color=\"%2\"><b>&nbsp;%3&nbsp;</b></font></td>"  \
                                   "<td>&nbsp;%4&nbsp;</td></tr>")                             \
                                   .arg(ThemeEngine::instance()->baseColor().name())           \
                                   .arg(ThemeEngine::instance()->textRegColor().name())        \
                                   .arg(MARK_OPTIONS(token->id()))                             \
                                   .arg(MARK_OPTIONS(token->description()));                   \
            }                                                                                  \
        }                                                                                      \
    } while (0)

    // --------------------------------------------------------

    if (!d->parser)
    {
        d->tooltipTracker->setText(QString());
    }
    else
    {
        QString tooltip;
        tooltip += QString("<qt><table cellspacing=\"0\" cellpadding=\"0\" border=\"0\">");

        // --------------------------------------------------------

        TOOLTIP_HEADER(i18n("Renaming Options"));
        TOOLTIP_ENTRIES(Option, d->parser->options());

        tooltip += QString("<tr></tr>");

        TOOLTIP_HEADER(i18n("Modifiers"));
        TOOLTIP_ENTRIES(Modifier, d->parser->modifiers());

        // --------------------------------------------------------

        tooltip += QString("</table></qt>");
        tooltip += i18n("<p><i>Modifiers can be applied to every renaming option. <br/>"
                        "It is possible to assign multiple modifiers to an option, "
                        "they are applied in the order you assign them.</i></p>");

        d->tooltipTracker->setText(tooltip);
    }

    // --------------------------------------------------------

#undef TOOLTIP_HEADER
#undef TOOLTIP_ENTRIES
#undef MARK_OPTIONS
}

void AdvancedRenameWidget::slotToolTipButtonToggled(bool checked)
{
    d->tooltipTracker->setVisible(checked);
    slotUpdateTrackerPos();
}

void AdvancedRenameWidget::slotUpdateTrackerPos()
{
    d->tooltipTracker->refresh();
}

void AdvancedRenameWidget::setControlWidgets(ControlWidgets mask)
{
    bool enable       = d->parser && !(d->parser->options().isEmpty());
    bool enableModBtn = enable && !(d->parser->modifiers().isEmpty());

    d->renameInput->setEnabled(enable);
    d->optionsLabel->setVisible(enable && (mask & TokenButtons));
    d->tokenToolButton->setVisible(enable && (mask & TokenToolButton));
    d->modifierToolButton->setVisible(enableModBtn && (mask & ModifierToolButton));
    d->tooltipToggleButton->setVisible(enable && (mask & ToolTipButton));

    d->controlWidgetsMask = mask;
}

void AdvancedRenameWidget::registerParserControls()
{
   if (d->parser)
   {
       setupWidgets();

       QMenu* tokenToolBtnMenu    = new QMenu(d->tokenToolButton);
       QMenu* modifierToolBtnMenu = new QMenu(d->modifierToolButton);
       QPushButton* btn           = 0;
       QAction* action            = 0;
       DynamicLayout* layout      = new DynamicLayout(KDialog::marginHint(), KDialog::marginHint());

       foreach (Option* option, d->parser->options())
       {
           btn    = option->registerButton(this);
           action = option->registerMenu(tokenToolBtnMenu);

           if (!btn || !action)
           {
               continue;
           }

           // set button tooltip
           btn->setToolTip(option->description());

           layout->addWidget(btn);

           connect(option, SIGNAL(signalTokenTriggered(const QString&)),
                   d->renameInput, SLOT(slotAddToken(const QString&)));
       }

       // --------------------------------------------------------

       // register modifiers
       foreach (Modifier* modifier, d->parser->modifiers())
       {
           action = modifier->registerMenu(modifierToolBtnMenu);
           if (!action)
           {
               continue;
           }

           connect(modifier, SIGNAL(signalTokenTriggered(const QString&)),
                   d->renameInput, SLOT(slotAddToken(const QString&)));
       }

       // --------------------------------------------------------

       d->btnContainer->setLayout(layout);
       setMinimumWidth(d->btnContainer->layout()->sizeHint().width());

       d->tokenToolButton->setMenu(tokenToolBtnMenu);
       d->modifierToolButton->setMenu(modifierToolBtnMenu);

       d->renameInput->setParser(d->parser);
       createToolTip();
   }
}

void AdvancedRenameWidget::setParser(Parser* parser)
{
    if (!parser)
    {
        return;
    }

    if (d->parser)
    {
        delete d->parser;
    }
    d->parser = parser;

    registerParserControls();
    setControlWidgets(d->controlWidgetsMask);
}

void AdvancedRenameWidget::setupWidgets()
{
    /*
     * This methods needs to delete all main widgets, do not remove the delete lines!
     * If a new parser is set or the layout has changed, we need to call setupWidgets() again.
     * So any widget that is created in here needs to be removed first, to avoid memory leaks and
     * duplicate signal/slot connections.
     */
    delete d->renameInput;
    d->renameInput = new AdvancedRenameInput;

    delete d->tooltipToggleButton;
    d->tooltipToggleButton = new QToolButton;
    d->tooltipToggleButton->setCheckable(true);
    d->tooltipToggleButton->setIcon(SmallIcon("dialog-information"));
    d->tooltipToggleButton->setToolTip(i18n("Show help"));

    // --------------------------------------------------------

    delete d->btnContainer;
    d->btnContainer = new QWidget(this);

    delete d->optionsLabel;
    d->optionsLabel = new RLabelExpander(this);
    d->optionsLabel->setText(i18n("Renaming Options"));
    d->optionsLabel->setWidget(d->btnContainer);
    d->optionsLabel->setLineVisible(false);

    // --------------------------------------------------------

    delete d->tokenToolButton;
    d->tokenToolButton = new QToolButton;
    d->tokenToolButton->setPopupMode(QToolButton::InstantPopup);
    d->tokenToolButton->setIcon(SmallIcon("list-add"));
    d->tokenToolButton->setToolTip(i18n("Quickly add a renaming option"));

    delete d->modifierToolButton;
    d->modifierToolButton = new QToolButton;
    d->modifierToolButton->setPopupMode(QToolButton::InstantPopup);
    d->modifierToolButton->setIcon(SmallIcon("document-edit"));
    d->modifierToolButton->setToolTip(i18n("Quickly add a modifier to a renaming option"));

    // --------------------------------------------------------

    // Although we delete every other widget in here, DON'T delete this one!
    // It has a parent now and will be deleted automatically.
    d->tooltipTracker = new DTipTracker(QString(), d->renameInput, Qt::AlignLeft);
    d->tooltipTracker->setTextFormat(Qt::RichText);
    d->tooltipTracker->setEnable(false);
    d->tooltipTracker->setKeepOpen(true);
    d->tooltipTracker->setOpenExternalLinks(true);
    setTooltipAlignment(d->tooltipTrackerAlignment);

    // --------------------------------------------------------

    delete layout();
    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(d->renameInput,         0, 0, 1, 1);
    mainLayout->addWidget(d->tooltipToggleButton, 0, 1, 1, 1);
    mainLayout->addWidget(d->tokenToolButton,     0, 2, 1, 1);
    mainLayout->addWidget(d->modifierToolButton,  0, 3, 1, 1);
    mainLayout->addWidget(d->optionsLabel,        1, 0, 1,-1);
    mainLayout->setColumnStretch(0, 10);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(KDialog::marginHint());
    setLayout(mainLayout);

    // --------------------------------------------------------

    connect(d->tooltipToggleButton, SIGNAL(toggled(bool)),
            this, SLOT(slotToolTipButtonToggled(bool)));

    connect(d->renameInput, SIGNAL(signalTextChanged(const QString&)),
            this, SIGNAL(signalTextChanged(const QString&)));

    connect(d->renameInput, SIGNAL(signalTokenMarked(bool)),
            this, SLOT(slotTokenMarked(bool)));

    connect(d->renameInput, SIGNAL(signalReturnPressed()),
            this, SIGNAL(signalReturnPressed()));

    slotTokenMarked(false);
    readSettings();
}

void AdvancedRenameWidget::slotTokenMarked(bool marked)
{
    bool enable    = marked && d->parser;
    bool enableMod = enable && !(d->parser->modifiers().isEmpty());
    d->modifierToolButton->setEnabled(enableMod);
}

void AdvancedRenameWidget::focusLineEdit()
{
    d->renameInput->slotSetFocus();
}

void AdvancedRenameWidget::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->optionsLabel->setExpanded(group.readEntry(d->configExpandedStateEntry, d->configExpandedStateDefault));
}

void AdvancedRenameWidget::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    // remove duplicate entries and save pattern history, omit empty strings
    QString pattern = d->renameInput->text();
    group.writeEntry(d->configExpandedStateEntry, d->optionsLabel
                                                  ? d->optionsLabel->isExpanded()
                                                  : d->configExpandedStateDefault);
}

}  // namespace Digikam
