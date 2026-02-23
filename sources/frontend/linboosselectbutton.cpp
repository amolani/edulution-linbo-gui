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

#include "linboosselectbutton.h"

LinboOsSelectButton::LinboOsSelectButton(QString icon, LinboOs* os, LinboBackend* backend, QWidget* parent) : QWidget(parent)
{
    this->_inited = false;
    this->_os = os;
    this->_backend = backend;
    this->_shouldBeVisible = true;
    this->_showDefaultAction = true;
    this->_isCardHovered = false;
    this->_osNameLabel = nullptr;

    if(!QFile::exists(icon) || !icon.endsWith(".svg")) {
        icon = gTheme->iconPath(LinboTheme::DefaultOsIcon);
    }

    // OS icon renderer (drawn in paintEvent)
    this->_iconRenderer = new QSvgRenderer(icon, this);

    connect(this->_backend, &LinboBackend::stateChanged, this, &LinboOsSelectButton::_handleBackendStateChange);

    // Transparent click target covering entire card
    this->_button = new LinboPushButton("", "", this);
    this->_button->setAttribute(Qt::WA_StyledBackground, false);
    this->_button->setAttribute(Qt::WA_TranslucentBackground, true);
    this->_button->setAutoFillBackground(false);
    // Mute all overlays so nothing renders
    this->_button->setOverlayTypeMuted(LinboPushButtonOverlay::OnHover, true);
    this->_button->setOverlayTypeMuted(LinboPushButtonOverlay::OnPressed, true);
    this->_button->setOverlayTypeMuted(LinboPushButtonOverlay::OnChecked, true);
    this->_button->setOverlayTypeMuted(LinboPushButtonOverlay::OnKeyboardFocus, true);

    this->setToolTip(this->_os->description());
    connect(this->_button, &LinboPushButton::clicked, this, &LinboOsSelectButton::_handlePrimaryButtonClicked);

    // Action labels
    QMap<LinboOs::LinboOsStartAction, QString> actionLabels = {
        {LinboOs::StartOs, "Start"},
        {LinboOs::SyncOs, "Sync"},
        {LinboOs::ReinstallOs, "Reinstall"}
    };

    // Primary start pill (default action) - solid blue
    QString defaultLabel = actionLabels.value(this->_os->defaultAction(), "Start");
    if(!this->_os->actionEnabled(this->_os->defaultAction()))
        defaultLabel = "";

    this->_primaryStartPill = new LinboPushButton("", defaultLabel, this);
    this->_primaryStartPill->setPillGradient(QColor("#8fc046"), QColor("#1084c0"));
    this->_primaryStartPill->setToolTip(this->_getTooltipContentForAction(this->_os->defaultAction()));
    this->_primaryStartPill->setVisible(false);
    connect(this->_primaryStartPill, &LinboPushButton::clicked, this, &LinboOsSelectButton::_handlePrimaryButtonClicked);

    // Secondary start action pills (non-default enabled actions) - ghost blue
    for(auto action : {LinboOs::StartOs, LinboOs::SyncOs, LinboOs::ReinstallOs}) {
        bool disabled = !this->_os->actionEnabled(action) || this->_os->defaultAction() == action;
        if(disabled)
            continue;

        LinboPushButton* pill = new LinboPushButton("", actionLabels.value(action, "?"), this);
        pill->setPillColor(QColor("#0081c6"));
        pill->setGhostPill(true);
        pill->setToolTip(this->_getTooltipContentForAction(action));
        pill->setVisible(false);

        switch (action) {
        case LinboOs::StartOs:
            connect(pill, &LinboPushButton::clicked, this->_os, &LinboOs::executeStart);
            break;
        case LinboOs::SyncOs:
            connect(pill, &LinboPushButton::clicked, this->_os, &LinboOs::executeSync);
            break;
        case LinboOs::ReinstallOs:
            connect(pill, &LinboPushButton::clicked, this->_os, &LinboOs::executeReinstall);
            break;
        default:
            break;
        }

        this->_startActionButtons.append(pill);
    }

    // Root mode: Image creation pill (solid blue)
    this->_primaryRootPill = new LinboPushButton("", "Image", this);
    this->_primaryRootPill->setPillGradient(QColor("#8fc046"), QColor("#1084c0"));
    //% "Create image of %1"
    this->_primaryRootPill->setToolTip(qtTrId("createImageOfOS").arg(this->_os->name()));
    this->_primaryRootPill->setVisible(false);
    connect(this->_primaryRootPill, &LinboPushButton::clicked, this, [=] {
        emit this->imageCreationRequested(this->_os);
    });

    // Root mode: Upload pill (ghost blue)
    LinboPushButton* uploadPill = new LinboPushButton("", "Upload", this);
    uploadPill->setPillColor(QColor("#0081c6"));
    uploadPill->setGhostPill(true);
    //% "Upload image of %1"
    uploadPill->setToolTip(qtTrId("uploadImageOfOS").arg(this->_os->name()));
    uploadPill->setVisible(false);
    connect(uploadPill, &LinboPushButton::clicked, this, [=] {
        emit this->imageUploadRequested(this->_os);
    });
    this->_rootActionButtons.append(uploadPill);

    // OS name label
    this->_osNameLabel = new QLabel(this);
    QString displayName = this->_os->name();
    if(!this->_os->version().isEmpty())
        displayName += "  " + this->_os->version();

    if(this->_os->baseImage() != nullptr) {
        this->_osNameLabel->setText(displayName);
        this->_osNameLabel->setStyleSheet("QLabel { color: white; background: transparent; }");
    }
    else {
        //% "No baseimage defined"
        this->_osNameLabel->setText(qtTrId("main_noBaseImage"));
        this->_osNameLabel->setStyleSheet("QLabel { color: red; background: transparent; }");
    }
    this->_osNameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    this->_button->setCheckable(true);
    this->_handleBackendStateChange(this->_backend->state());

    QWidget::setVisible(true);
}

void LinboOsSelectButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int h = this->height();
    int w = this->width();
    int radius = std::max(8, h / 7);

    // No card background — elements float on the page background

    // OS icon (centered vertically, fixed position)
    int pad = h * 0.15;
    int iconSize = h - pad * 2;
    int iconX = pad;
    QRectF iconRect(iconX, pad, iconSize, iconSize);
    if(this->_iconRenderer && this->_iconRenderer->isValid()) {
        this->_iconRenderer->render(&painter, iconRect);
    }
}

void LinboOsSelectButton::enterEvent(QEnterEvent *event) {
    Q_UNUSED(event)
    this->_isCardHovered = true;
    this->update();
}

void LinboOsSelectButton::leaveEvent(QEvent *event) {
    Q_UNUSED(event)
    this->_isCardHovered = false;
    this->update();
}

void LinboOsSelectButton::_handlePrimaryButtonClicked() {
    if (this->_backend->state() == LinboBackend::Autostarting) {
        this->_backend->cancelCurrentAction();
    }

    if(this->_backend->state() == LinboBackend::Idle)
        switch (this->_os->defaultAction()) {
        case LinboOs::StartOs:
            this->_os->executeStart();
            break;
        case LinboOs::SyncOs:
            this->_os->executeSync();
            break;
        case LinboOs::ReinstallOs:
            this->_os->executeReinstall();
            break;
        default:
            break;
        }
    else if (this->_backend->state() == LinboBackend::Root) {
        emit this->imageCreationRequested(this->_os);
    }
}

LinboOs* LinboOsSelectButton::_getOs() {
    return this->_os;
}

void LinboOsSelectButton::_setVisibleAnimated(bool visible) {
    this->_shouldBeVisible = visible;
    if(this->_osNameLabel != nullptr)
        this->_osNameLabel->setVisible(visible);

    this->_updateActionButtonVisibility();

    if(!visible) {
        QTimer::singleShot(300, this, [=] {
            if(!this->_shouldBeVisible)
                QWidget::setVisible(false);
        });
    } else {
        QWidget::setVisible(true);
    }
}

void LinboOsSelectButton::setVisible(bool visible) {
    this->_shouldBeVisible = visible;
    QWidget::setVisible(visible);
    if(this->_osNameLabel != nullptr)
        this->_osNameLabel->setVisible(visible);

    this->_updateActionButtonVisibility(true);
}

void LinboOsSelectButton::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);

    int h = event->size().height();
    int w = event->size().width();
    int pad = h * 0.15;
    int iconSize = h - pad * 2;
    int gap = h * 0.153;
    int pillH = std::max(16, (int)(h * 0.374));
    int pillY = (h - pillH) / 2;
    int pillSpacing = std::max(4, (int)(h * 0.06));
    int textToPillGap = std::max(8, (int)(h * 0.491));

    // Transparent button covers entire card
    this->_button->setGeometry(0, 0, w, h);

    // Calculate pill widths using font metrics
    QFont pillFont;
    pillFont.setPixelSize(gTheme->toFontSize(pillH * 0.45));
    pillFont.setBold(true);
    QFontMetrics fm(pillFont);
    int hPad = std::max(8, pillH / 2);

    auto calcPillWidth = [&](LinboPushButton* pill) -> int {
        pill->resize(100, pillH);
        return std::max(pill->minimumWidth() + 10, pillH * 2);
    };

    // Label: position after icon, measure text width
    int iconX = pad;
    int labelX = iconX + iconSize + gap;

    QFont nameFont = this->_osNameLabel->font();
    nameFont.setPixelSize(gTheme->toFontSize(h * 0.26));
    this->_osNameLabel->setFont(nameFont);

    QFontMetrics nameFm(nameFont);
    int textW = nameFm.horizontalAdvance(this->_osNameLabel->text()) + pillSpacing;
    int labelW = std::min(textW, w - labelX - pad);
    this->_osNameLabel->setGeometry(labelX, 0, labelW, h);

    // Position pills left-to-right directly after the label text
    int x = labelX + labelW + textToPillGap;

    // Start action pills: [primary solid pill] [...secondary ghost pills...]
    if(this->_primaryStartPill) {
        int pillW = calcPillWidth(this->_primaryStartPill);
        this->_primaryStartPill->setGeometry(x, pillY, pillW, pillH);
        x += pillW + pillSpacing;
    }
    for(int i = 0; i < this->_startActionButtons.size(); i++) {
        int pillW = calcPillWidth(this->_startActionButtons[i]);
        this->_startActionButtons[i]->setGeometry(x, pillY, pillW, pillH);
        x += pillW + pillSpacing;
    }

    // Root action pills (same area, positioned independently)
    x = labelX + labelW + textToPillGap;
    if(this->_primaryRootPill) {
        int pillW = calcPillWidth(this->_primaryRootPill);
        this->_primaryRootPill->setGeometry(x, pillY, pillW, pillH);
        x += pillW + pillSpacing;
    }
    for(int i = 0; i < this->_rootActionButtons.size(); i++) {
        int pillW = calcPillWidth(this->_rootActionButtons[i]);
        this->_rootActionButtons[i]->setGeometry(x, pillY, pillW, pillH);
        x += pillW + pillSpacing;
    }

    this->_updateActionButtonVisibility();
}

void LinboOsSelectButton::_handleBackendStateChange(LinboBackend::LinboState state) {
    this->_showDefaultAction = false;

    switch (state) {
    case LinboBackend::Idle:
        this->_button->setToolTip(this->_getTooltipContentForAction(this->_os->defaultAction()));
        this->_showDefaultAction = true;
        break;
    case LinboBackend::Root:
        //% "Create image of %1"
        this->_button->setToolTip(qtTrId("createImageOfOS").arg(this->_os->name()));
        this->_showDefaultAction = true;
        break;
    case LinboBackend::Autostarting:
        this->_showDefaultAction = true;
        break;
    default:
        break;
    }

    this->_updateActionButtonVisibility();
}

QString LinboOsSelectButton::_getTooltipContentForAction(LinboOs::LinboOsStartAction action) {
    QMap<LinboOs::LinboOsStartAction, QString> startActionButtonIcons = {
        //% "Start %1"
        {LinboOs::StartOs, qtTrId("startOS")},
        //% "Sync and start %1"
        {LinboOs::SyncOs, qtTrId("syncOS")},
        //% "Reinstall %1"
        {LinboOs::ReinstallOs, qtTrId("reinstallOS")}
    };

    return startActionButtonIcons[action].arg(this->_os->name());
}

void LinboOsSelectButton::_updateActionButtonVisibility(bool doNotAnimate) {
    Q_UNUSED(doNotAnimate)

    bool startVisible = this->_shouldBeVisible && this->_backend->state() < LinboBackend::Root;
    bool rootVisible = this->_shouldBeVisible && this->_backend->state() >= LinboBackend::Root;

    // Primary pills
    if(this->_primaryStartPill)
        this->_primaryStartPill->setVisible(startVisible && this->_showDefaultAction);
    if(this->_primaryRootPill)
        this->_primaryRootPill->setVisible(rootVisible && this->_showDefaultAction);

    // Secondary pills
    for(LinboPushButton* btn : this->_startActionButtons)
        btn->setVisible(startVisible);

    for(LinboPushButton* btn : this->_rootActionButtons)
        btn->setVisible(rootVisible);

    this->_inited = true;
}
