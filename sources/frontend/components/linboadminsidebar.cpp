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

#include "linboadminsidebar.h"

// ============================================================================
// LinboAdminSidebarItem
// ============================================================================

LinboAdminSidebarItem::LinboAdminSidebarItem(const QString& label, LinboTheme::Icon icon, QWidget* parent)
    : QWidget(parent),
    _label(label),
    _iconEnum(icon),
    _iconCacheSize(0),
    _expandWidth(0),
    _fullExpandedWidth(200),
    _collapsedWidth(60),
    _pillHeight(40),
    _isHovered(false),
    _isPressed(false),
    _isFocused(false),
    _isItemDisabled(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::TabFocus);

    // Extract first word as short label
    _shortLabel = _label.split(' ').first();

    // Load SVG renderer
    QString iconPath = gTheme->iconPath(_iconEnum);
    _svgRenderer = new QSvgRenderer(iconPath, this);

    _expandAnimation = new QPropertyAnimation(this, "expandWidth");
    _expandAnimation->setDuration(200);
    _expandAnimation->setEasingCurve(QEasingCurve::InOutQuad);
}

void LinboAdminSidebarItem::setExpandWidth(int w) {
    _expandWidth = w;
    update();
}

void LinboAdminSidebarItem::setItemDisabled(bool disabled) {
    _isItemDisabled = disabled;
    if(disabled) {
        setCursor(Qt::ArrowCursor);
    } else {
        setCursor(Qt::PointingHandCursor);
    }
    update();
}

void LinboAdminSidebarItem::_ensureIconCache(int size) {
    if(size <= 0) return;
    if(_iconCacheSize == size) return;

    _iconCache = QPixmap(size, size);
    _iconCache.fill(Qt::transparent);
    QPainter p(&_iconCache);
    p.setRenderHint(QPainter::Antialiasing);
    _svgRenderer->render(&p, QRectF(0, 0, size, size));
    _iconCacheSize = size;
}

void LinboAdminSidebarItem::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int h = this->height();
    int collW = _collapsedWidth;
    int iconSize = qMax(1, (int)(h * 0.65));
    _ensureIconCache(iconSize);

    qreal opacity = _isItemDisabled ? 0.3 : 1.0;
    p.setOpacity(opacity);

    if(_expandWidth > 0) {
        // Draw expanded gradient pill (slim, subtle)
        int pillW = _expandWidth;
        int pillH = _pillHeight;
        int pillY = (h - pillH) / 2;
        int pillX = this->width() - pillW;

        QLinearGradient grad(pillX, pillY, pillX + pillW, pillY);
        QColor ciGreen("#8fc046");
        QColor ciBlue("#0081c6");

        if(_isPressed) {
            ciGreen = ciGreen.darker(120);
            ciBlue = ciBlue.darker(120);
        }

        if(gTheme->lowFxMode()) {
            QColor flat = ciBlue;
            if(_isPressed) flat = flat.darker(120);
            p.setBrush(flat);
        } else {
            grad.setColorAt(0.0, ciGreen);
            grad.setColorAt(1.0, ciBlue);
            p.setBrush(grad);
        }
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(pillX, pillY, pillW, pillH, pillH / 2, pillH / 2);

        // Icon right, centered in the collapsed strip area
        int iconPillSize = qMax(1, pillH * 50 / 100);
        _ensureIconCache(iconPillSize);
        int iconX = this->width() - collW / 2 - iconPillSize / 2;
        int iconY = (h - iconPillSize) / 2;
        if(!_iconCache.isNull())
            p.drawPixmap(iconX, iconY, _iconCache);

        // Label text left of icon
        QFont font;
        int fontSize = qMax(1, pillH * 36 / 100);
        font.setPixelSize(gTheme->toFontSize(fontSize));
        font.setBold(true);
        p.setFont(font);
        p.setPen(gTheme->textAt(240));

        int textX = pillX + pillH * 40 / 100;
        int textW = pillW - collW - pillH * 30 / 100;
        if(textW > 0) {
            QRect textRect(textX, pillY, textW, pillH);
            p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, _label);
        }
    } else {
        // Collapsed state: small icon centered + label below
        int iconX = (collW - iconSize) / 2 + (this->width() - collW);
        int labelFontSize = qMax(1, (int)(h * 0.124));

        QFont labelFont;
        labelFont.setPixelSize(gTheme->toFontSize(labelFontSize));

        // Auto-shrink font if text is wider than collapsed area
        QFontMetrics fm(labelFont);
        int maxTextW = collW - 4;
        while(fm.horizontalAdvance(_shortLabel) > maxTextW && labelFontSize > 4) {
            labelFontSize--;
            labelFont.setPixelSize(gTheme->toFontSize(labelFontSize));
            fm = QFontMetrics(labelFont);
        }

        int labelH = fm.height();

        int totalH = iconSize + 2 + labelH;
        int startY = (h - totalH) / 2;

        int iconY = startY;
        if(!_iconCache.isNull())
            p.drawPixmap(iconX, iconY, _iconCache);

        p.setFont(labelFont);
        p.setPen(gTheme->textAt(120));
        QRect labelRect(this->width() - collW, iconY + iconSize + 2, collW, labelH);
        p.drawText(labelRect, Qt::AlignCenter, _shortLabel);
    }

    // Subtle focus indicator
    if(_isFocused && !_isItemDisabled) {
        p.setOpacity(0.4);
        p.setPen(QPen(gTheme->textAt(80), 1, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        int focusX = this->width() - collW + 3;
        p.drawRoundedRect(focusX, 3, collW - 6, h - 6, 4, 4);
    }
}

void LinboAdminSidebarItem::enterEvent(QEnterEvent* event) {
    Q_UNUSED(event)
    if(_isItemDisabled) return;
    _isHovered = true;
    _expandAnimation->stop();
    _expandAnimation->setStartValue(_expandWidth);
    _expandAnimation->setEndValue(_fullExpandedWidth);
    _expandAnimation->start();
}

void LinboAdminSidebarItem::leaveEvent(QEvent* event) {
    Q_UNUSED(event)
    _isHovered = false;
    _isPressed = false;
    _expandAnimation->stop();
    _expandAnimation->setStartValue(_expandWidth);
    _expandAnimation->setEndValue(0);
    _expandAnimation->start();
}

void LinboAdminSidebarItem::mousePressEvent(QMouseEvent* event) {
    Q_UNUSED(event)
    if(_isItemDisabled) return;
    _isPressed = true;
    update();
}

void LinboAdminSidebarItem::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event)
    if(_isItemDisabled) return;
    if(_isPressed) {
        _isPressed = false;
        update();
        emit clicked();
    }
}

void LinboAdminSidebarItem::keyPressEvent(QKeyEvent* event) {
    if(_isItemDisabled) { QWidget::keyPressEvent(event); return; }
    if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Space) {
        _isPressed = true;
        update();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void LinboAdminSidebarItem::keyReleaseEvent(QKeyEvent* event) {
    if(_isItemDisabled) { QWidget::keyReleaseEvent(event); return; }
    if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Space) {
        _isPressed = false;
        update();
        emit clicked();
    } else {
        QWidget::keyReleaseEvent(event);
    }
}

void LinboAdminSidebarItem::focusInEvent(QFocusEvent* event) {
    Q_UNUSED(event)
    _isFocused = true;
    // Expand on focus (for keyboard nav)
    if(!_isItemDisabled) {
        _expandAnimation->stop();
        _expandAnimation->setStartValue(_expandWidth);
        _expandAnimation->setEndValue(_fullExpandedWidth);
        _expandAnimation->start();
    }
    update();
}

void LinboAdminSidebarItem::focusOutEvent(QFocusEvent* event) {
    Q_UNUSED(event)
    _isFocused = false;
    _expandAnimation->stop();
    _expandAnimation->setStartValue(_expandWidth);
    _expandAnimation->setEndValue(0);
    _expandAnimation->start();
    update();
}


// ============================================================================
// LinboAdminSidebar
// ============================================================================

LinboAdminSidebar::LinboAdminSidebar(LinboBackend* backend, QWidget* parent)
    : QWidget(parent),
    _backend(backend),
    _slideOffset(0),
    _animatingVisible(false)
{
    setAttribute(Qt::WA_TranslucentBackground);

    bool hasOs = _backend->config()->operatingSystems().length() > 0;

    // Create 4 items with same translation IDs as LinboMainActions
    //% "Open terminal"
    auto* terminalItem = new LinboAdminSidebarItem(qtTrId("main_root_button_openTerminal"), LinboTheme::TerminalIcon, this);
    connect(terminalItem, &LinboAdminSidebarItem::clicked, this, &LinboAdminSidebar::terminalRequested);
    _items.append(terminalItem);

    //% "Update cache"
    _cacheItem = new LinboAdminSidebarItem(qtTrId("main_root_button_updateCache"), LinboTheme::SyncIcon, this);
    _cacheItem->setVisible(hasOs);
    connect(_cacheItem, &LinboAdminSidebarItem::clicked, this, &LinboAdminSidebar::cacheUpdateRequested);
    _items.append(_cacheItem);

    //% "Partition drive"
    _partitionItem = new LinboAdminSidebarItem(qtTrId("main_root_button_partitionDrive"), LinboTheme::PartitionIcon, this);
    _partitionItem->setVisible(hasOs);
    connect(_partitionItem, &LinboAdminSidebarItem::clicked, this, &LinboAdminSidebar::drivePartitioningRequested);
    _items.append(_partitionItem);

    //% "Register"
    auto* registerItem = new LinboAdminSidebarItem(qtTrId("main_root_button_register"), LinboTheme::RegisterIcon, this);
    connect(registerItem, &LinboAdminSidebarItem::clicked, this, &LinboAdminSidebar::registrationRequested);
    _items.append(registerItem);

    // Slide animation
    _slideAnimation = new QPropertyAnimation(this, "slideOffset");
    _slideAnimation->setDuration(300);
    _slideAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    connect(_backend, &LinboBackend::stateChanged, this, &LinboAdminSidebar::_handleLinboStateChanged);

    // Initially hidden
    this->setVisible(false);
}

void LinboAdminSidebar::setSlideOffset(int offset) {
    _slideOffset = offset;
    if(parentWidget()) {
        int parentW = parentWidget()->width();
        int totalW = this->width();
        int baseX = parentW - totalW;
        this->move(baseX + _slideOffset, this->y());
    }
}

void LinboAdminSidebar::resizeToParent() {
    if(!parentWidget()) return;

    int parentH = parentWidget()->height();
    int parentW = parentWidget()->width();

    int collapsedW = qMax(1, (int)(parentH * 0.059));
    int sidebarY = parentH * 0.12;
    int sidebarH = parentH * 0.76;

    // Calculate item dimensions
    int visibleCount = 0;
    for(auto* item : _items)
        if(item->isVisible()) visibleCount++;
    if(visibleCount == 0) visibleCount = 1;

    int itemH = sidebarH / qMax(visibleCount + 2, 5);
    int pillH = itemH * 48 / 100;
    int maxPillW = parentW * 20 / 100;
    int expandedW = qMin(maxPillW, collapsedW + pillH + parentW * 12 / 100);

    // Sidebar widget is full expanded width so pills are not clipped
    int totalW = expandedW;
    int sidebarX = parentW - totalW;

    this->setGeometry(sidebarX + _slideOffset, sidebarY, totalW, sidebarH);

    // Position items: full width, right-aligned within sidebar
    int totalItemsH = visibleCount * itemH;
    int startY = (sidebarH - totalItemsH) / 2;
    int currentY = startY;

    for(auto* item : _items) {
        if(!item->isVisible()) continue;

        item->setCollapsedWidth(collapsedW);
        item->setFullExpandedWidth(expandedW);
        item->setPillHeight(pillH);
        item->setGeometry(0, currentY, totalW, itemH);
        currentY += itemH;
    }
}

void LinboAdminSidebar::setVisibleAnimated(bool visible) {
    if(visible == _animatingVisible) return;
    _animatingVisible = visible;

    _slideAnimation->stop();
    // Slide by the collapsed width (icon strip), not full widget width
    int parentH = parentWidget() ? parentWidget()->height() : 800;
    int collapsedW = qMax(1, (int)(parentH * 0.059));

    if(visible) {
        this->setVisible(true);
        this->resizeToParent();
        _slideAnimation->setStartValue(collapsedW);
        _slideAnimation->setEndValue(0);
        _slideAnimation->start();
    } else {
        _slideAnimation->setStartValue(_slideOffset);
        _slideAnimation->setEndValue(collapsedW);
        connect(_slideAnimation, &QPropertyAnimation::finished, this, [this]() {
            if(!_animatingVisible)
                this->setVisible(false);
            disconnect(_slideAnimation, &QPropertyAnimation::finished, nullptr, nullptr);
        });
        _slideAnimation->start();
    }
}

void LinboAdminSidebar::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    // No background - fully transparent, only items are visible
}

void LinboAdminSidebar::_handleLinboStateChanged(LinboBackend::LinboState newState) {
    switch(newState) {
    case LinboBackend::Root:
        this->resizeToParent();
        this->setVisibleAnimated(true);
        // Enable all items
        for(auto* item : _items)
            item->setItemDisabled(false);
        break;

    case LinboBackend::RootTimeout:
    case LinboBackend::Idle:
        this->setVisibleAnimated(false);
        break;

    case LinboBackend::Partitioning:
    case LinboBackend::UpdatingCache:
    case LinboBackend::Registering:
        // Disable items during operations
        for(auto* item : _items)
            item->setItemDisabled(true);
        break;

    case LinboBackend::RootActionError:
    case LinboBackend::RootActionSuccess:
        // Re-enable after operation completes
        for(auto* item : _items)
            item->setItemDisabled(false);
        break;

    default:
        break;
    }
}
