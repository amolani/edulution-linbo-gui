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
    _slideY(0),
    _panelHeight(44),
    _open(false)
{
    setAttribute(Qt::WA_TranslucentBackground);

    // Row 1: Network info
    //% "Hostname"
    _rows.append({qtTrId("hostname"), config->hostname(), false});
    //% "Host group"
    _rows.append({qtTrId("group"), config->hostGroup(), false});
    //% "IP-Address"
    _rows.append({qtTrId("ip"), config->ipAddress(), false});
    //% "Mac"
    _rows.append({qtTrId("client_info_mac"), config->macAddress(), false});

    // Row 2: Hardware info (isSeparator=true marks start of second row)
    //% "HDD"
    _rows.append({qtTrId("client_info_hdd"), config->diskSize(), true});
    //% "CPU"
    _rows.append({qtTrId("client_info_cpu"), config->cpuModel(), false});

    // RAM: convert MB to GB
    QString ramStr = config->ramSize();
    bool ok = false;
    QString ramDisplay = ramStr;
    QStringList ramParts = ramStr.split(" ");
    if(!ramParts.isEmpty()) {
        double ramMb = ramParts.first().toDouble(&ok);
        if(ok) ramDisplay = QString::number(ramMb / 1024.0, 'f', 1) + " GB";
    }
    //% "RAM"
    _rows.append({qtTrId("client_info_ram"), ramDisplay, false});

    _slideAnimation = new QPropertyAnimation(this, "slideY");
    _slideAnimation->setDuration(250);
    _slideAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    this->setVisible(false);
}

void LinboClientInfoDrawer::setSlideY(int y) {
    _slideY = y;
    this->move(this->x(), _slideY);
}

void LinboClientInfoDrawer::toggle() {
    if(!parentWidget()) return;
    int parentH = parentWidget()->height();
    int footerH = parentH * 7 / 100;
    int targetY = parentH - footerH - _panelHeight;

    _slideAnimation->stop();

    if(!_open) {
        _open = true;
        this->setVisible(true);
        _slideAnimation->setStartValue(parentH);
        _slideAnimation->setEndValue(targetY);
        _slideAnimation->start();
    } else {
        _open = false;
        _slideAnimation->setStartValue(_slideY);
        _slideAnimation->setEndValue(parentH);
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

    _panelHeight = qBound(40, parentH * 6 / 100, 64);

    // Position above the footer (footer = 7% height + small margin)
    int footerH = parentH * 7 / 100;
    int targetY = parentH - footerH - _panelHeight;

    int y = _open ? targetY : parentH;
    this->setGeometry(0, y, parentW, _panelHeight);
}

void LinboClientInfoDrawer::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = this->width();
    int h = this->height();
    int padX = w * 5 / 100;
    int rowH = h / 2;

    // No background — completely transparent

    // Font setup
    int fontPx = qMax(8, rowH * 50 / 100);
    QFont labelFont;
    labelFont.setPixelSize(gTheme->toFontSize(fontPx));
    labelFont.setBold(false);

    QFont valueFont;
    valueFont.setPixelSize(gTheme->toFontSize(fontPx));
    valueFont.setBold(true);

    QFont dotFont;
    dotFont.setPixelSize(gTheme->toFontSize(fontPx));

    QFontMetrics labelFm(labelFont);
    QFontMetrics valueFm(valueFont);
    QFontMetrics dotFm(dotFont);

    QString sep = QString("  \u00b7  ");
    int sepW = dotFm.horizontalAdvance(sep);

    // Split rows into two lines: isSeparator=true starts line 2
    QList<QList<LinboInfoRow>> lines;
    lines.append(QList<LinboInfoRow>());
    for(const auto& row : _rows) {
        if(row.isSeparator)
            lines.append(QList<LinboInfoRow>());
        lines.last().append(row);
    }

    for(int line = 0; line < lines.count(); line++) {
        const auto& items = lines[line];
        int lineY = line * rowH;

        // Calculate total width for centering
        int totalW = 0;
        for(int i = 0; i < items.count(); i++) {
            totalW += labelFm.horizontalAdvance(items[i].label + ": ");
            totalW += valueFm.horizontalAdvance(items[i].value);
            if(i < items.count() - 1)
                totalW += sepW;
        }

        int currentX = qMax(padX, (w - totalW) / 2);

        for(int i = 0; i < items.count(); i++) {
            const auto& item = items[i];

            // Dot separator
            if(i > 0) {
                p.setFont(dotFont);
                p.setPen(gTheme->textAt(36));
                p.drawText(QRect(currentX, lineY, sepW, rowH), Qt::AlignVCenter, sep);
                currentX += sepW;
            }

            // Label
            QString labelStr = item.label + ": ";
            int labelW = labelFm.horizontalAdvance(labelStr);
            p.setFont(labelFont);
            p.setPen(gTheme->textAt(84));
            p.drawText(QRect(currentX, lineY, labelW, rowH), Qt::AlignVCenter, labelStr);
            currentX += labelW;

            // Value — elide if exceeding available width
            int availW = width() - currentX - 4;
            QString displayValue = valueFm.elidedText(item.value, Qt::ElideRight, availW);
            int valW = valueFm.horizontalAdvance(displayValue);
            p.setFont(valueFont);
            p.setPen(gTheme->textAt(168));
            p.drawText(QRect(currentX, lineY, valW, rowH), Qt::AlignVCenter, displayValue);
            currentX += valW;
        }
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

    // Position indicator: bottom-left corner
    int indicatorSize = qMax(24, parentH * 4 / 100);
    _indicator->setFixedSize(indicatorSize, indicatorSize);
    int margin = parentH * 1 / 100;
    int indicatorX = margin;
    int indicatorY = parentH - indicatorSize - margin;
    _indicator->move(indicatorX, indicatorY);

    _drawer->resizeToParent();

    if(_showClientInfo && !_drawer->isOpen()) {
        _drawer->toggle();
        _showClientInfo = false;  // only auto-open once
    }
}
