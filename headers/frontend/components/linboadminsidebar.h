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

#ifndef LINBOADMINSIDEBAR_H
#define LINBOADMINSIDEBAR_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QSvgRenderer>
#include <QLinearGradient>
#include <QFontMetrics>
#include <QEnterEvent>

#include "linbobackend.h"
#include "linboguitheme.h"

class LinboAdminSidebarItem : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int expandWidth READ expandWidth WRITE setExpandWidth)

public:
    LinboAdminSidebarItem(const QString& label, LinboTheme::Icon icon, QWidget* parent = nullptr);

    int expandWidth() const { return _expandWidth; }
    void setExpandWidth(int w);

    void setFullExpandedWidth(int w) { _fullExpandedWidth = w; }
    void setCollapsedWidth(int w) { _collapsedWidth = w; }
    void setPillHeight(int h) { _pillHeight = h; }
    void setItemDisabled(bool disabled);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

signals:
    void clicked();

private:
    QString _label;
    QString _shortLabel;
    LinboTheme::Icon _iconEnum;
    QSvgRenderer* _svgRenderer;
    QPixmap _iconCache;
    int _iconCacheSize;

    int _expandWidth;
    int _fullExpandedWidth;
    int _collapsedWidth;
    int _pillHeight;
    bool _isHovered;
    bool _isPressed;
    bool _isFocused;
    bool _isItemDisabled;

    QPropertyAnimation* _expandAnimation;

    void _ensureIconCache(int size);
};


class LinboAdminSidebar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int slideOffset READ slideOffset WRITE setSlideOffset)

public:
    LinboAdminSidebar(LinboBackend* backend, QWidget* parent = nullptr);

    int slideOffset() const { return _slideOffset; }
    void setSlideOffset(int offset);

    void setVisibleAnimated(bool visible);
    void resizeToParent();

signals:
    void terminalRequested();
    void cacheUpdateRequested();
    void drivePartitioningRequested();
    void registrationRequested();

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void _handleLinboStateChanged(LinboBackend::LinboState newState);

private:
    LinboBackend* _backend;
    QList<LinboAdminSidebarItem*> _items;
    LinboAdminSidebarItem* _cacheItem;
    LinboAdminSidebarItem* _partitionItem;

    int _slideOffset;
    QPropertyAnimation* _slideAnimation;
    bool _animatingVisible;
};

#endif // LINBOADMINSIDEBAR_H
