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

#include "linboclientinfosidebar.h"

// ============================================================================
// LinboInfoIndicator  — small "i" circle on left edge
// ============================================================================

LinboInfoIndicator::LinboInfoIndicator(QWidget* parent)
    : QWidget(parent), _hovered(false), _pressed(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::PointingHandCursor);
    setFixedSize(32, 32);
}

void LinboInfoIndicator::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int s = qMin(width(), height());
    int margin = 2;
    QRect circle(margin, margin, s - 2 * margin, s - 2 * margin);

    // Background circle
    QColor bg = _hovered ? QColor(255, 255, 255, 25) : QColor(255, 255, 255, 13);
    if(_pressed) bg = QColor(255, 255, 255, 40);
    p.setBrush(bg);
    p.setPen(QPen(QColor(255, 255, 255, 35), 1));
    p.drawEllipse(circle);

    // "i" letter
    QFont font;
    font.setPixelSize(gTheme->toFontSize(s * 45 / 100));
    font.setBold(true);
    font.setItalic(true);
    p.setFont(font);
    p.setPen(gTheme->textAt(180));
    p.drawText(circle, Qt::AlignCenter, "i");
}

void LinboInfoIndicator::mousePressEvent(QMouseEvent* event) {
    Q_UNUSED(event)
    _pressed = true;
    update();
}

void LinboInfoIndicator::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event)
    if(_pressed) {
        _pressed = false;
        update();
        emit clicked();
    }
}

void LinboInfoIndicator::enterEvent(QEnterEvent* event) {
    Q_UNUSED(event)
    _hovered = true;
    update();
}

void LinboInfoIndicator::leaveEvent(QEvent* event) {
    Q_UNUSED(event)
    _hovered = false;
    _pressed = false;
    update();
}


// ============================================================================
// LinboClientInfoDrawer  — glass panel with host details
// ============================================================================

LinboClientInfoDrawer::LinboClientInfoDrawer(LinboConfig* config, QWidget* parent)
    : QWidget(parent),
    _slideX(0),
    _panelWidth(260),
    _open(false)
{
    setAttribute(Qt::WA_TranslucentBackground);

    // Network rows
    //% "Hostname"
    _rows.append({qtTrId("hostname"), config->hostname(), false});
    //% "Host group"
    _rows.append({qtTrId("group"), config->hostGroup(), false});
    //% "IP-Address"
    _rows.append({qtTrId("ip"), config->ipAddress(), false});
    //% "Mac"
    _rows.append({qtTrId("client_info_mac"), config->macAddress(), false});

    // Hardware rows (separator before first)
    //% "HDD"
    _rows.append({qtTrId("client_info_hdd"), config->diskSize(), true});
    //% "CPU"
    _rows.append({qtTrId("client_info_cpu"), config->cpuModel(), false});
    //% "RAM"
    _rows.append({qtTrId("client_info_ram"), config->ramSize(), false});

    _slideAnimation = new QPropertyAnimation(this, "slideX");
    _slideAnimation->setDuration(250);
    _slideAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    this->setVisible(false);
}

void LinboClientInfoDrawer::setSlideX(int x) {
    _slideX = x;
    this->move(_slideX, this->y());
}

void LinboClientInfoDrawer::toggle() {
    _slideAnimation->stop();

    if(!_open) {
        _open = true;
        this->setVisible(true);
        _slideAnimation->setStartValue(-_panelWidth);
        _slideAnimation->setEndValue(0);
        _slideAnimation->start();
    } else {
        _open = false;
        _slideAnimation->setStartValue(_slideX);
        _slideAnimation->setEndValue(-_panelWidth);
        connect(_slideAnimation, &QPropertyAnimation::finished, this, [this]() {
            if(!_open)
                this->setVisible(false);
            disconnect(_slideAnimation, &QPropertyAnimation::finished, nullptr, nullptr);
        });
        _slideAnimation->start();
    }
}

void LinboClientInfoDrawer::resizeToParent() {
    if(!parentWidget()) return;

    int parentH = parentWidget()->height();
    int parentW = parentWidget()->width();

    _panelWidth = qBound(220, parentW * 20 / 100, 300);
    int panelH = parentH * 55 / 100;
    int panelY = (parentH - panelH) / 2;

    int x = _open ? 0 : -_panelWidth;
    this->setGeometry(x, panelY, _panelWidth, panelH);
}

void LinboClientInfoDrawer::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = this->width();
    int h = this->height();
    int pad = w * 7 / 100;
    int radius = 14;

    // Glass panel background
    QPainterPath panelPath;
    panelPath.addRoundedRect(QRectF(0, 0, w, h), radius, radius);
    p.setClipPath(panelPath);

    // Dark glass fill
    p.fillRect(rect(), QColor(10, 14, 20, 220));

    // Subtle border
    p.setClipping(false);
    p.setPen(QPen(QColor(255, 255, 255, 20), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(QRectF(0.5, 0.5, w - 1, h - 1), radius, radius);

    // Title
    int titleFontPx = qMax(10, h * 4 / 100);
    QFont titleFont;
    titleFont.setPixelSize(gTheme->toFontSize(titleFontPx));
    titleFont.setBold(true);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.5);
    p.setFont(titleFont);
    p.setPen(gTheme->textAt(200));

    int titleY = pad;
    int titleH = titleFontPx + 4;
    QRect titleRect(pad, titleY, w - 2 * pad, titleH);
    p.drawText(titleRect, Qt::AlignVCenter | Qt::AlignLeft, "CLIENT INFO");

    // Divider under title
    int divY = titleY + titleH + pad / 2;
    p.setPen(QPen(QColor(255, 255, 255, 20), 1));
    p.drawLine(pad, divY, w - pad, divY);

    // Rows
    int rowStartY = divY + pad / 2;
    int availH = h - rowStartY - pad;
    int rowCount = _rows.count();
    int rowH = qMin(availH / qMax(rowCount, 1), h * 10 / 100);
    int labelFontPx = qMax(8, rowH * 34 / 100);
    int valueFontPx = qMax(8, rowH * 36 / 100);

    QFont labelFont;
    labelFont.setPixelSize(gTheme->toFontSize(labelFontPx));
    labelFont.setBold(false);

    QFont valueFont;
    valueFont.setPixelSize(gTheme->toFontSize(valueFontPx));
    valueFont.setBold(true);

    int contentX = pad;
    int contentW = w - 2 * pad;
    int currentY = rowStartY;

    for(int i = 0; i < rowCount; i++) {
        const auto& row = _rows[i];

        // Draw separator line before hardware section
        if(row.isSeparator) {
            int sepY = currentY + 2;
            p.setPen(QPen(QColor(255, 255, 255, 15), 1));
            p.drawLine(contentX, sepY, contentX + contentW, sepY);
            currentY += pad / 2;
        }

        int halfRow = rowH / 2;

        // Label (dim, smaller)
        p.setFont(labelFont);
        p.setPen(gTheme->textAt(90));
        QRect labelRect(contentX, currentY, contentW, halfRow);
        p.drawText(labelRect, Qt::AlignBottom | Qt::AlignLeft, row.label);

        // Value (bright, bold)
        p.setFont(valueFont);
        p.setPen(gTheme->textAt(210));
        QRect valueRect(contentX, currentY + halfRow, contentW, halfRow);
        QString elidedValue = QFontMetrics(valueFont).elidedText(row.value, Qt::ElideRight, contentW);
        p.drawText(valueRect, Qt::AlignTop | Qt::AlignLeft, elidedValue);

        currentY += rowH;
    }
}


// ============================================================================
// LinboClientInfoSidebar  — container for indicator + drawer
// ============================================================================

LinboClientInfoSidebar::LinboClientInfoSidebar(LinboConfig* config, QWidget* parent)
    : QWidget(parent),
    _showClientInfo(config->clientDetailsVisibleByDefault())
{
    setAttribute(Qt::WA_TranslucentBackground);

    // "i" indicator button
    _indicator = new LinboInfoIndicator(parent);
    _indicator->setVisible(true);
    _indicator->raise();

    // Drawer panel
    _drawer = new LinboClientInfoDrawer(config, parent);
    _drawer->raise();

    connect(_indicator, &LinboInfoIndicator::clicked, _drawer, &LinboClientInfoDrawer::toggle);

    // Show drawer initially if config says so
    if(_showClientInfo) {
        _drawer->setVisible(true);
        // Will be positioned properly by resizeToParent
    }
}

void LinboClientInfoSidebar::setVisibleAnimated(bool visible) {
    Q_UNUSED(visible)
    _drawer->toggle();
}

void LinboClientInfoSidebar::resizeToParent() {
    if(!parentWidget()) return;

    int parentH = parentWidget()->height();

    // Position indicator: left edge, vertically centered
    int indicatorSize = qMax(24, parentH * 4 / 100);
    _indicator->setFixedSize(indicatorSize, indicatorSize);
    int indicatorX = parentH * 1 / 100;
    int indicatorY = (parentH - indicatorSize) / 2;
    _indicator->move(indicatorX, indicatorY);

    _drawer->resizeToParent();

    if(_showClientInfo && !_drawer->isOpen()) {
        _drawer->toggle();
        _showClientInfo = false;  // only auto-open once
    }
}
