/****************************************************************************
 ** Modern Linbo GUI
 ** Copyright (C) 2020-2021  Dorian Zedler <dorian@itsblue.de>
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU Affero General Public License as published
 ** by the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Affero General Public License for more details.
 **
 ** You should have received a copy of the GNU Affero General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#ifndef LINBOCLIENTINFOSIDEBAR_H
#define LINBOCLIENTINFOSIDEBAR_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QFontMetrics>
#include <QMouseEvent>

#include "linboconfig.h"
#include "linboguitheme.h"

struct LinboInfoRow {
    QString label;
    QString value;
    bool isSeparator;  // true = draw divider before this row
};

// Small "i" indicator button, always visible on left edge
class LinboInfoIndicator : public QWidget
{
    Q_OBJECT
public:
    LinboInfoIndicator(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

signals:
    void clicked();

private:
    bool _hovered;
    bool _pressed;
};

// Horizontal bottom bar showing host details
class LinboClientInfoDrawer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int slideY READ slideY WRITE setSlideY)

public:
    LinboClientInfoDrawer(LinboConfig* config, QWidget* parent = nullptr);

    int slideY() const { return _slideY; }
    void setSlideY(int y);

    void toggle();
    void resizeToParent();
    bool isOpen() const { return _open; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QList<LinboInfoRow> _rows;
    int _slideY;
    int _panelHeight;
    QPropertyAnimation* _slideAnimation;
    bool _open;
};

// Container managing indicator + drawer
class LinboClientInfoSidebar : public QWidget
{
    Q_OBJECT

public:
    LinboClientInfoSidebar(LinboConfig* config, QWidget* parent = nullptr);

    void setVisibleAnimated(bool visible);
    void resizeToParent();

private:
    LinboInfoIndicator* _indicator;
    LinboClientInfoDrawer* _drawer;
    bool _showClientInfo;
};

#endif // LINBOCLIENTINFOSIDEBAR_H
