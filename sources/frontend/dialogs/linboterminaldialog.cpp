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
    //% "Terminal"
    this->setTitle(qtTrId("dialog_terminal_title"));
    this->_terminal = new LinboTerminal(this);
    connect(this->_terminal, &LinboTerminal::processExited, this, &LinboDialog::autoClose);
    connect(this, &LinboDialog::closedByUser, this->_terminal, &LinboTerminal::clearAndRestart);
}

void LinboTerminalDialog::resizeEvent(QResizeEvent *event) {
    LinboDialog::resizeEvent(event);

    int margins = gTheme->size(LinboTheme::Margins);
    int headerHeight = margins * 2;
    this->_terminal->setGeometry(margins, margins + headerHeight, this->width() - 2 * margins, this->height() - 2 * margins - headerHeight);
}

void LinboTerminalDialog::paintEvent(QPaintEvent *event) {
    LinboDialog::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int margins = gTheme->size(LinboTheme::Margins);
    int headerHeight = margins * 2;

    // Green accent line at top of terminal area
    painter.setPen(QPen(QColor(143, 192, 70, 40), 1));
    painter.drawLine(margins, margins + headerHeight - 1, this->width() - margins, margins + headerHeight - 1);

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
    painter.drawText(textX, textY, this->width() / 2, headerHeight, Qt::AlignVCenter | Qt::AlignLeft, "Terminal");

    // "root@linbo" right-aligned
    painter.setPen(QColor(143, 192, 70, 100));
    painter.drawText(this->width() / 2, textY, this->width() / 2 - margins, headerHeight, Qt::AlignVCenter | Qt::AlignRight, "root@linbo");

    Q_UNUSED(event)
}
