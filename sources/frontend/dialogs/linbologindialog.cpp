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

#include "linbologindialog.h"

LinboLoginDialog::LinboLoginDialog(LinboBackend* backend, QWidget* parent) : LinboDialog(parent)
{
    this->_backend = backend;
    this->_wrongPassword = false;
    this->_updatingDisplay = false;
    this->_shakeStep = 0;
    this->_shakeOriginX = 0;

    // Frameless: no toolbar, no bottom bar — just a floating pill
    this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setAutoFillBackground(false);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    // Lock icon renderer (painted in paintEvent)
    this->_lockRenderer = new QSvgRenderer(QStringLiteral(":/icons/universal/lock.svg"), this);

    // Password input — Normal mode, we handle masking manually with bullets
    this->_passwordInput = new QLineEdit(this);
    this->_passwordInput->setEchoMode(QLineEdit::Normal);
    this->_passwordInput->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    //% "Please enter password:"
    this->_passwordInput->setPlaceholderText(qtTrId("dialog_login_title"));
    this->_passwordInput->setFrame(false);
    this->_passwordInput->setStyleSheet(
        "QLineEdit {"
        "  background: transparent;"
        "  border: none;"
        "  color: #888888;"
        "  selection-background-color: #0081c6;"
        "}"
    );
    connect(this->_passwordInput, &QLineEdit::returnPressed, this, &LinboLoginDialog::inputFinished);
    connect(this->_passwordInput, &QLineEdit::textChanged, this, &LinboLoginDialog::_onTextChanged);

    // Layout: lock icon space | input | (submit is via Enter key)
    this->_mainLayout = new QHBoxLayout(this);
    this->_mainLayout->setContentsMargins(0, 0, 0, 0);
    this->_mainLayout->setSpacing(0);
    // Space for lock icon is handled by left margin in resizeEvent
    this->_mainLayout->addWidget(this->_passwordInput);

    // Shake animation timer for wrong password
    this->_shakeTimer = new QTimer(this);
    this->_shakeTimer->setInterval(30);
    connect(this->_shakeTimer, &QTimer::timeout, this, &LinboLoginDialog::_shakeStep_slot);
}

void LinboLoginDialog::_onTextChanged(const QString &text) {
    if(this->_updatingDisplay)
        return;

    // Reset wrong-password state on any typing
    if(this->_wrongPassword) {
        this->_wrongPassword = false;
        this->update();
    }

    // Determine what the user actually typed/deleted
    int cursorPos = this->_passwordInput->cursorPosition();
    int bulletLen = text.length();
    int oldLen = this->_realPassword.length();

    if(bulletLen > oldLen) {
        // Characters were added — find the non-bullet chars in text
        // They'll be at cursorPos - (bulletLen - oldLen) .. cursorPos - 1
        int addedCount = bulletLen - oldLen;
        int insertAt = cursorPos - addedCount;
        if(insertAt < 0) insertAt = 0;
        QString added;
        for(int i = insertAt; i < cursorPos && i < text.length(); i++) {
            QChar ch = text.at(i);
            if(ch != QChar(0x2022))
                added.append(ch);
        }
        this->_realPassword.insert(insertAt, added);
    }
    else if(bulletLen < oldLen) {
        // Characters were deleted
        int removedCount = oldLen - bulletLen;
        int removeAt = cursorPos;
        if(removeAt < 0) removeAt = 0;
        if(removeAt + removedCount > this->_realPassword.length())
            removedCount = this->_realPassword.length() - removeAt;
        this->_realPassword.remove(removeAt, removedCount);
    }

    // Update display to all bullets
    this->_updatingDisplay = true;
    QString bullets(this->_realPassword.length(), QChar(0x2022));
    this->_passwordInput->setText(bullets);
    this->_passwordInput->setCursorPosition(cursorPos);
    this->_updatingDisplay = false;
}

void LinboLoginDialog::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int h = this->height();
    int radius = h / 2;  // perfect pill shape

    // Soft shadow
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 60));
    painter.drawRoundedRect(QRectF(rect()).adjusted(1, 3, 1, 3), radius, radius);

    // Background pill
    QColor bgColor = QColor("#111111");
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(QRectF(rect()), radius, radius);

    // Border — red on wrong password, subtle gray otherwise
    QColor borderColor = this->_wrongPassword ? QColor("#dc2626") : QColor("#333333");
    if(this->_passwordInput->hasFocus() && !this->_wrongPassword)
        borderColor = QColor("#0081c6");
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(borderColor, 1.5));
    painter.drawRoundedRect(QRectF(rect()).adjusted(0.75, 0.75, -0.75, -0.75), radius, radius);

    // Lock icon (left side)
    if(this->_lockRenderer && this->_lockRenderer->isValid()) {
        int iconSize = h * 0.4;
        int iconX = h * 0.35;
        int iconY = (h - iconSize) / 2;
        // Tint: use opacity to indicate state
        painter.setOpacity(this->_wrongPassword ? 0.5 : 0.35);
        this->_lockRenderer->render(&painter, QRectF(iconX, iconY, iconSize, iconSize));
        painter.setOpacity(1.0);
    }

    // Subtle arrow hint on the right (Enter to submit)
    int arrowSize = h * 0.2;
    int arrowX = this->width() - h * 0.45;
    int arrowY = (h - arrowSize) / 2;
    painter.setPen(QPen(QColor(255, 255, 255, 40), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(arrowX, arrowY, arrowX + arrowSize * 0.7, arrowY + arrowSize / 2);
    painter.drawLine(arrowX + arrowSize * 0.7, arrowY + arrowSize / 2, arrowX, arrowY + arrowSize);
}

void LinboLoginDialog::resizeEvent(QResizeEvent *event) {
    LinboDialog::resizeEvent(event);

    int h = this->height();
    // Left margin: space for lock icon; right margin: space for arrow hint
    int leftMargin = h * 0.75;
    int rightMargin = h * 0.55;
    this->_mainLayout->setContentsMargins(leftMargin, 0, rightMargin, 0);

    // Font size proportional to height
    QFont inputFont = this->_passwordInput->font();
    inputFont.setPixelSize(qBound(12, h * 35 / 100, 18));
    this->_passwordInput->setFont(inputFont);
}

void LinboLoginDialog::inputFinished() {
    if(this->_backend->login(this->_realPassword)) {
        this->_realPassword.clear();
        this->_passwordInput->clear();
        this->_wrongPassword = false;
        this->close();
    }
    else {
        this->_realPassword.clear();
        this->_passwordInput->clear();
        this->_wrongPassword = true;
        this->update();

        // Shake animation
        this->_shakeStep = 0;
        this->_shakeOriginX = this->x();
        this->_shakeTimer->start();
    }
}

void LinboLoginDialog::_shakeStep_slot() {
    // 8 steps of shake: left-right-left-right...
    static const int offsets[] = {-6, 6, -4, 4, -2, 2, -1, 1, 0};
    if(this->_shakeStep >= 9) {
        this->_shakeTimer->stop();
        this->move(this->_shakeOriginX, this->y());
        return;
    }
    this->move(this->_shakeOriginX + offsets[this->_shakeStep], this->y());
    this->_shakeStep++;
}

void LinboLoginDialog::setVisibleAnimated(bool visible) {
    if(!visible) {
        this->_realPassword.clear();
        this->_passwordInput->clear();
        this->_wrongPassword = false;
    }
    else {
        this->_passwordInput->setFocus();
    }
    LinboDialog::setVisibleAnimated(visible);
}
