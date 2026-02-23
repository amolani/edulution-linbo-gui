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

#include "linbomainpage.h"

LinboMainPage::LinboMainPage(LinboBackend* backend, QWidget *parent) : QWidget(parent)
{
    this->setAutoFillBackground(false);
    this->_inited = false;
    this->_showClientInfo = backend->config()->clientDetailsVisibleByDefault();

    this->_backend = backend;

#ifdef TEST_ENV
    //this->backend->login("Muster!");
#endif

    connect(this->_backend, &LinboBackend::stateChanged, this, &LinboMainPage::_handleLinboStateChanged);

    this->setGeometry(QRect(0,0,parent->width(), parent->height()));

    // create the main layout
    // main layout
    QWidget* mainLayoutWidget = new QWidget(this);
    mainLayoutWidget->setAttribute(Qt::WA_TranslucentBackground);
    mainLayoutWidget->setGeometry(this->geometry());
    QVBoxLayout* mainLayout = new QVBoxLayout(mainLayoutWidget);
    mainLayout->setSpacing(this->height()*0.03);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addSpacerItem(new QSpacerItem(this->width(), mainLayout->spacing()));

    // Header: LINBO text + subtitle
    QWidget* headerWidget = new QWidget();
    headerWidget->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout* headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(2);

    this->_titleLabel = new QLabel("LINBO");
    QFont titleFont;
    titleFont.setPixelSize(this->height() * 0.06);
    titleFont.setBold(true);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, this->height() * 0.012);
    this->_titleLabel->setFont(titleFont);
    this->_titleLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(gTheme->textAt(230).name(QColor::HexArgb)));
    this->_titleLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(this->_titleLabel);

    // Load edulution logo for top-right branding
    this->_edulutionLogoRenderer = new QSvgRenderer(QString(":/images/edulution_logo.svg"), this);

    mainLayout->addWidget(headerWidget);
    mainLayout->setAlignment(headerWidget, Qt::AlignCenter);

    mainLayout->addStretch();

    // OS Buttons
    _osSelectionRow = new LinboOsSelectionRow(this->_backend);
    mainLayout->addWidget(_osSelectionRow);

    mainLayout->addStretch();

    // action buttons
    this->_mainActions = new LinboMainActions(this->_backend, this);
    this->_mainActions->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mainLayout->addWidget(this->_mainActions);

    this->_startActionWidgetAnimation = new QPropertyAnimation(this->_mainActions, "minimumSize");
    this->_startActionWidgetAnimation->setDuration(400);
    this->_startActionWidgetAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    // Admin Sidebar (absolutely positioned, not in layout)
    _adminSidebar = new LinboAdminSidebar(this->_backend, this);
    _adminSidebar->raise();

    // Client Info Sidebar (absolutely positioned, left side)
    _clientInfoSidebar = new LinboClientInfoSidebar(this->_backend->config(), this);
    _clientInfoSidebar->resizeToParent();
    _clientInfoSidebar->raise();

    mainLayout->addStretch();

    // Footer: version+IP left, power/settings buttons right
    QWidget* footerWidget = new QWidget();
    footerWidget->setAttribute(Qt::WA_TranslucentBackground);
    footerWidget->setFixedHeight(this->height() * 0.07);
    QHBoxLayout* footerLayout = new QHBoxLayout(footerWidget);
    int footerMargins = this->width() * 0.05;
    footerLayout->setContentsMargins(footerMargins, 0, footerMargins, 0);
    footerLayout->setSpacing(this->width() * 0.015);

    QLabel* footerInfoLabel = new QLabel(backend->config()->linboVersion() + " \xC2\xB7 " + this->_backend->config()->ipAddress());
    QFont footerFont;
    footerFont.setPixelSize(gTheme->size(LinboTheme::RowFontSize) * 0.8);
    footerInfoLabel->setFont(footerFont);
    footerInfoLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(gTheme->textAt(77).name(QColor::HexArgb)));
    footerLayout->addWidget(footerInfoLabel);

    footerLayout->addStretch();

    int footerButtonSize = this->height() * 0.0525;

    _rootActionButton = new LinboToolButton(LinboTheme::SettingsIcon);
    this->_powerActionButtons.append(_rootActionButton);
    _rootActionButton->setFixedHeight(footerButtonSize);
    _rootActionButton->setFixedWidth(footerButtonSize);
    //% "Settings"
    _rootActionButton->setToolTip(qtTrId("settings"));
    footerLayout->addWidget(_rootActionButton);

    _logoutActionButton = new LinboToolButton(LinboTheme::LogoutIcon);
    connect(_logoutActionButton, &LinboToolButton::clicked, this->_backend, &LinboBackend::logout);
    this->_powerActionButtons.append(_logoutActionButton);
    _logoutActionButton->setFixedHeight(footerButtonSize);
    _logoutActionButton->setFixedWidth(footerButtonSize);
    _logoutActionButton->setVisible(false);
    //% "Log out"
    _logoutActionButton->setToolTip(qtTrId("logout"));
    footerLayout->addWidget(_logoutActionButton);

    LinboPushButton* rebootActionButton = new LinboToolButton(LinboTheme::RebootIcon);
    connect(rebootActionButton, &LinboToolButton::clicked, this->_backend, &LinboBackend::reboot);
    this->_powerActionButtons.append(rebootActionButton);
    rebootActionButton->setFixedHeight(footerButtonSize);
    rebootActionButton->setFixedWidth(footerButtonSize);
    //% "Reboot"
    rebootActionButton->setToolTip(qtTrId("reboot"));
    footerLayout->addWidget(rebootActionButton);

    LinboPushButton* shutdownActionButton = new LinboToolButton(LinboTheme::ShutdownIcon);
    connect(shutdownActionButton, &LinboToolButton::clicked, this->_backend, &LinboBackend::shutdown);
    this->_powerActionButtons.append(shutdownActionButton);
    shutdownActionButton->setFixedHeight(footerButtonSize);
    shutdownActionButton->setFixedWidth(footerButtonSize);
    //% "Shutdown"
    shutdownActionButton->setToolTip(qtTrId("shutdown"));
    footerLayout->addWidget(shutdownActionButton);

    mainLayout->addWidget(footerWidget);

    // Dialogs (for imaging stuff)
    this->_loginDialog = new LinboLoginDialog(this->_backend, this);
    int dialogHeight = gTheme->size(LinboTheme::DialogHeight);
    int dialogWidth = gTheme->size(LinboTheme::DialogWidth);
    // Slim pill: ~380px wide, ~44px tall
    int loginW = qBound(300, (int)(dialogWidth * 0.5), 420);
    int loginH = qBound(38, (int)(this->height() * 0.055), 50);
    this->_loginDialog->setGeometry(0, 0, loginW, loginH);
    this->_loginDialog->centerInParent();
    connect(this->_powerActionButtons[0], &LinboToolButton::clicked, this->_loginDialog, &LinboLoginDialog::open);

    this->_imageCreationDialog = new LinboImageCreationDialog(backend, parent);
    this->_allDialogs.append(this->_imageCreationDialog);
    this->_imageCreationDialog->setGeometry(0, 0, dialogWidth, dialogHeight);
    this->_imageCreationDialog->centerInParent();
    connect(this->_osSelectionRow, &LinboOsSelectionRow::imageCreationRequested,
            this->_imageCreationDialog, &LinboImageCreationDialog::open);

    this->_imageUploadDialog = new LinboImageUploadDialog(backend, parent);
    this->_allDialogs.append(this->_imageUploadDialog);
    this->_imageUploadDialog->setGeometry(0, 0, dialogWidth, dialogHeight * 0.3);
    this->_imageUploadDialog->centerInParent();
    connect(this->_osSelectionRow, &LinboOsSelectionRow::imageUploadRequested,
            this->_imageUploadDialog, &LinboImageUploadDialog::open);

    this->_terminalDialog = new LinboTerminalDialog(parent);
    this->_allDialogs.append(this->_terminalDialog);
    this->_terminalDialog->setGeometry(0, 0, std::min(dialogWidth * 2, int(this->width() * 0.9)), dialogHeight);
    this->_terminalDialog->centerInParent();
    connect(this->_adminSidebar, &LinboAdminSidebar::terminalRequested,
            this->_terminalDialog, &LinboTerminalDialog::open);

    this->_confirmationDialog = new LinboConfirmationDialog(
        //% "Partition drive"
        qtTrId("dialog_partition_title"),
        //% "Are you sure? This will delete all data on your drive!"
        qtTrId("dialog_partition_question"),
        parent);
    this->_allDialogs.append(this->_confirmationDialog);

    this->_confirmationDialog->setGeometry(0, 0, dialogWidth, dialogHeight * 0.2);
    this->_confirmationDialog->centerInParent();
    connect(this->_confirmationDialog, &LinboConfirmationDialog::accepted, this->_backend, &LinboBackend::partitionDrive);
    connect(this->_adminSidebar, &LinboAdminSidebar::drivePartitioningRequested,
            this->_confirmationDialog, &LinboDialog::open);

    this->_registerDialog = new LinboRegisterDialog(backend, parent);
    this->_allDialogs.append(this->_registerDialog);
    this->_registerDialog->setGeometry(0, 0, dialogWidth, dialogHeight * 0.7);
    this->_registerDialog->centerInParent();
    connect(this->_adminSidebar, &LinboAdminSidebar::registrationRequested,
            this->_registerDialog, &LinboRegisterDialog::open);

    this->_updateCacheDialog = new LinboUpdateCacheDialog(backend, parent);
    this->_allDialogs.append(this->_updateCacheDialog);
    this->_updateCacheDialog->setGeometry(0, 0, dialogWidth * 0.5, dialogHeight * 0.3);
    this->_updateCacheDialog->centerInParent();
    connect(this->_adminSidebar, &LinboAdminSidebar::cacheUpdateRequested,
            this->_updateCacheDialog, &LinboUpdateCacheDialog::open);

    // attach eventFilter
    qApp->installEventFilter(this);
    this->_handleLinboStateChanged(this->_backend->state());
}

void LinboMainPage::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Edulution Dark Mode: near-black base with subtle blue-gray gradient
    QLinearGradient baseGrad(0, 0, this->width(), this->height());
    baseGrad.setColorAt(0.0, QColor("#0a0a0a"));
    baseGrad.setColorAt(0.5, QColor("#0d1117"));
    baseGrad.setColorAt(1.0, QColor("#0a0f14"));
    painter.fillRect(this->rect(), baseGrad);

    if(!gTheme->lowFxMode()) {
        // Subtle Edulution Blue glow top-right
        QRadialGradient meshBlue(this->width() * 0.85, this->height() * 0.1, this->width() * 0.6);
        meshBlue.setColorAt(0.0, QColor(0, 129, 198, 12));
        meshBlue.setColorAt(1.0, QColor(0, 129, 198, 0));
        painter.fillRect(this->rect(), meshBlue);

        // Subtle green glow bottom-left
        QRadialGradient meshGreen(this->width() * 0.15, this->height() * 0.9, this->width() * 0.5);
        meshGreen.setColorAt(0.0, QColor(143, 192, 70, 8));
        meshGreen.setColorAt(1.0, QColor(143, 192, 70, 0));
        painter.fillRect(this->rect(), meshGreen);
    }

    // Edulution logo, top-right, subtle
    if(this->_edulutionLogoRenderer && this->_edulutionLogoRenderer->isValid()) {
        int logoH = this->height() * 0.051;
        // SVG aspect ratio ~841.9:228.4 ≈ 3.69:1
        int logoW = logoH * 841.9 / 228.4;
        int margin = this->height() * 0.025;
        painter.setOpacity(0.5);
        this->_edulutionLogoRenderer->render(&painter, QRectF(this->width() - logoW - margin, margin, logoW, logoH));
        painter.setOpacity(1.0);
    }
}

void LinboMainPage::_handleLinboStateChanged(LinboBackend::LinboState newState) {
    bool powerActionButtonsVisible = false;
    int startActionsWidgetHeight;
    int osSelectionRowHeight;

    switch (newState) {
    case LinboBackend::StartActionError:
    case LinboBackend::RootActionError:
        osSelectionRowHeight = this->height() * 0.15;
        startActionsWidgetHeight = this->height() * 0.45;
        break;

    case LinboBackend::Idle:
        osSelectionRowHeight = this->height() * 0.35;
        startActionsWidgetHeight = this->height() * 0;

        powerActionButtonsVisible = true;
        break;

    case LinboBackend::Root:
        osSelectionRowHeight = this->height() * 0.36;
        startActionsWidgetHeight = this->height() * 0;

        powerActionButtonsVisible = true;
        break;

    case LinboBackend::RootTimeout:
        for(LinboDialog* dialog : this->_allDialogs)
            dialog->autoClose();
    // fall through
    case LinboBackend::Partitioning:
    case LinboBackend::UpdatingCache:
    case LinboBackend::Registering:
    case LinboBackend::Disabled:
        osSelectionRowHeight = this->height() * 0;
        startActionsWidgetHeight = this->height() * 0.2;
        break;

    case LinboBackend::RootActionSuccess:
        osSelectionRowHeight = this->height() * 0;
        startActionsWidgetHeight = this->height() * 0.3;
        break;

    default:
        osSelectionRowHeight = this->height() * 0.2;
        startActionsWidgetHeight = this->height() * 0.2;
        break;
    }

    for(LinboPushButton* powerActionButton : this->_powerActionButtons)
        if(powerActionButton == _logoutActionButton && newState < LinboBackend::Root)
            powerActionButton->setVisible(false);
        else if(powerActionButton == _logoutActionButton && newState >= LinboBackend::Root)
            powerActionButton->setVisible(powerActionButtonsVisible);
        else if(powerActionButton == _rootActionButton && newState >= LinboBackend::Root)
            powerActionButton->setVisible(false);
        else
            powerActionButton->setVisible(powerActionButtonsVisible);

    if(this->_inited) {
        this->_startActionWidgetAnimation->setStartValue(QSize(this->width(), this->_mainActions->height()));
        this->_startActionWidgetAnimation->setEndValue(QSize(this->width(), startActionsWidgetHeight));
        this->_startActionWidgetAnimation->start();

        this->_osSelectionRow->setMinimumSizeAnimated(QSize(this->width(), osSelectionRowHeight));
    }
    else {
        this->_mainActions->setMinimumSize(this->width(), startActionsWidgetHeight);
        this->_osSelectionRow->setMinimumSize(this->width(), osSelectionRowHeight);
    }

    this->_inited = true;
}

bool LinboMainPage::eventFilter(QObject *obj, QEvent *event) {
    Q_UNUSED(obj)

    if (event->type() == QEvent::MouseMove)
    {
        this->_backend->restartRootTimeout();
    }
    else if(event->type() == QEvent::KeyPress) {
        this->_backend->restartRootTimeout();

        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->key() == Qt::Key_F1) {
            this->_showClientInfo = !this->_showClientInfo;
            this->_clientInfoSidebar->setVisibleAnimated(this->_showClientInfo);
        }
        else if(keyEvent->key() == Qt::Key_Escape) {
            switch (this->_backend->state()) {
            case LinboBackend::Autostarting:
            case LinboBackend::RootTimeout:
                this->_backend->cancelCurrentAction();
                break;

            case LinboBackend::StartActionError:
            case LinboBackend::RootActionError:
            case LinboBackend::RootActionSuccess:
                this->_backend->resetMessage();
                break;

            case LinboBackend::Root: {
                bool someDialogOpen = false;
                for(LinboDialog* dialog : this->_allDialogs) {
                    if(dialog->isVisible()) {
                        someDialogOpen = true;
                        break;
                    }
                }

                if(!someDialogOpen)
                    this->_backend->logout();
                break;
            }

            default:
                break;
            }
        }
    }
    return false;
}

