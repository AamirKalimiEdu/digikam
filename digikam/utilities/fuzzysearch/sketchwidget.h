/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : a widget to draw sketch.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SKETCHWIDGET_H
#define SKETCHWIDGET_H

// Qt includes.

#include <QWidget>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace Digikam
{

class SketchWidgetPriv;

class SketchWidget : public QWidget
{
    Q_OBJECT

public:

    SketchWidget(QWidget *parent=0);
    ~SketchWidget();

    QColor  penColor() const;
    int     penWidth() const;
    bool    isClear()  const;

    QImage  sketchImage() const;
    void    setSketchImage(const QImage& image);

    /** This method return the drawing line history
     *  as XML, to be stored in database as SAlbum data.
     */
    void sketchImageToXML(QXmlStreamWriter &writer);
    QString sketchImageToXML();

    /** This method set sketch image using XML data based 
     *  on drawing line history.
     *  Retrun true if data are imported sucessfully.
     */
    bool setSketchImageFromXML(QXmlStreamReader &reader);
    bool setSketchImageFromXML(const QString &xml);

signals:

    void signalSketchChanged(const QImage&);
    void signalUndoRedoStateChanged(bool hasUndo, bool hasRedo);

public slots:

    void setPenColor(const QColor& newColor);
    void setPenWidth(int newWidth);
    void slotClear();
    void slotUndo();
    void slotRedo();

protected:

    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void paintEvent(QPaintEvent*);

private:

    void replayEvents(int index);
    void drawLineTo(const QPoint& endPoint);
    void drawLineTo(int width, const QColor& color, const QPoint& start, const QPoint& end);
    void drawPath(int width, const QColor& color, const QPainterPath &path);
    void addPath(QXmlStreamReader &reader);
    //QDomElement addXmlTextElement(QDomDocument &document, QDomElement &target,
      //                            const QString& tag, const QString& text);

private:

    SketchWidgetPriv *d;
};

}  // namespace Digikam

#endif // SKETCHWIDGET_H
