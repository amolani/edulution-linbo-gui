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

#include "linboterminaldialog.h"

LinboTerminalDialog::LinboTerminalDialog(QWidget* parent) : LinboDialog(parent)
{
    this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
    //% "Terminal"
    this->setTitle(qtTrId("dialog_terminal_title"));
    this->_terminal = new LinboTerminal(this);
    connect(this->_terminal, &LinboTerminal::processExited, this, &LinboDialog::autoClose);
    connect(this, &LinboDialog::closedByUser, this->_terminal, &LinboTerminal::clearAndRestart);

    // Close button — same LinboToolButton(CancelIcon) as all other dialogs
    this->_termCloseButton = new LinboToolButton(LinboTheme::CancelIcon, this);
    connect(this->_termCloseButton, &LinboToolButton::clicked, this, &LinboDialog::autoClose);
    connect(this->_termCloseButton, &LinboToolButton::clicked, this, &LinboDialog::closedByUser);
}

void LinboTerminalDialog::resizeEvent(QResizeEvent *event) {
    LinboDialog::resizeEvent(event);

    int margins = gTheme->size(LinboTheme::Margins);
    int headerHeight = margins * 2;
    this->_terminal->setGeometry(margins, margins + headerHeight, this->width() - 2 * margins, this->height() - 2 * margins - headerHeight);

    // Position close button in top-right of header
    int btnSize = headerHeight * 0.91;
    int btnY = margins + (headerHeight - btnSize) / 2;
    int btnX = this->width() - margins - btnSize;
    this->_termCloseButton->setGeometry(btnX, btnY, btnSize, btnSize);
    this->_termCloseButton->raise();
}

void LinboTerminalDialog::paintEvent(QPaintEvent *event) {
    // Base: solid bg + border + _updateTabOrder
    LinboDialog::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = this->width();
    int h = this->height();
    int radius = 6;
    int margins = gTheme->size(LinboTheme::Margins);
    int headerHeight = margins * 2;

    // === GLASS EFFECT (painted over solid base) ===

    // Clip to rounded rect
    QPainterPath clipPath;
    clipPath.addRoundedRect(QRectF(0, 0, w, h), radius, radius);
    painter.setClipPath(clipPath);

    // Glass base: dark diagonal gradient with subtle variation
    QLinearGradient glassFill(0, 0, w * 0.3, h);
    glassFill.setColorAt(0.0, QColor(16, 20, 26));
    glassFill.setColorAt(0.3, QColor(12, 14, 18));
    glassFill.setColorAt(0.7, QColor(10, 12, 16));
    glassFill.setColorAt(1.0, QColor(8, 10, 14));
    painter.fillRect(rect(), glassFill);

    // Top highlight (simulates light reflection on glass surface)
    QLinearGradient topHL(0, 0, 0, h * 0.12);
    topHL.setColorAt(0.0, QColor(255, 255, 255, 10));
    topHL.setColorAt(1.0, QColor(255, 255, 255, 0));
    painter.fillRect(rect(), topHL);

    // Subtle green glow at bottom (reflects terminal green)
    QRadialGradient greenGlow(w * 0.5, h * 0.9, w * 0.6);
    greenGlow.setColorAt(0.0, QColor(143, 192, 70, 8));
    greenGlow.setColorAt(1.0, QColor(143, 192, 70, 0));
    painter.fillRect(rect(), greenGlow);

    painter.setClipping(false);

    // Glass border (brighter than normal dialog border)
    painter.setPen(QPen(QColor(255, 255, 255, 35), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(QRectF(0.5, 0.5, w - 1, h - 1), radius, radius);

    // Inner top highlight line (glass edge catch)
    painter.setPen(QPen(QColor(255, 255, 255, 18), 1));
    painter.drawLine(radius + 1, 1, w - radius - 1, 1);

    // === HEADER ===

    // Green accent line at top of terminal area
    painter.setPen(QPen(QColor(143, 192, 70, 40), 1));
    painter.drawLine(margins, margins + headerHeight - 1, w - margins, margins + headerHeight - 1);

    // Green dot indicator
    int dotSize = headerHeight * 0.35;
    int dotY = margins + (headerHeight - dotSize) / 2;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(143, 192, 70, 180));
    painter.drawEllipse(margins + 2, dotY, dotSize, dotSize);

    // "Terminal" label
    QFont labelFont("Ubuntu Mono");
    labelFont.setPixelSize(headerHeight * 0.5);
    painter.setFont(labelFont);
    painter.setPen(QColor(143, 192, 70, 153));
    int textX = margins + 2 + dotSize + margins * 0.5;
    int textY = margins;
    painter.drawText(textX, textY, w / 2, headerHeight, Qt::AlignVCenter | Qt::AlignLeft, "Terminal");

    // "root@linbo" after Terminal label
    painter.setPen(QColor(143, 192, 70, 100));
    QFontMetrics fm(labelFont);
    int termLabelW = fm.horizontalAdvance("Terminal");
    int rootX = textX + termLabelW + margins;
    int closeBtnW = this->_termCloseButton->width();
    painter.drawText(rootX, textY, w - rootX - margins - closeBtnW - margins, headerHeight, Qt::AlignVCenter | Qt::AlignLeft, "root@linbo");

    Q_UNUSED(event)
}
