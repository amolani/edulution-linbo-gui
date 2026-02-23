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

#include "linboosselectionrow.h"

LinboOsSelectionRow::LinboOsSelectionRow(LinboBackend* backend, QWidget *parent) : QWidget(parent)
{
    this->_inited = false;
    this->_sizeOverride = nullptr;

    this->_backend = backend;
    connect(this->_backend, &LinboBackend::stateChanged, this, &LinboOsSelectionRow::_handleLinboStateChanged);

    this->_showOnlyButtonOfOsOfCurrentAction = false;

    this->_sizeAnimation = new QPropertyAnimation(this, "minimumSize", this);
    this->_sizeAnimation->setDuration(100);
    this->_sizeAnimation->setEasingCurve(QEasingCurve::OutQuad);

    for(LinboOs* os : backend->config()->operatingSystems()) {
        if(this->_osButtons.length() >= 4)
            break;

#ifdef TEST_ENV
        LinboOsSelectButton* osButton = new LinboOsSelectButton(TEST_ENV"/gui/icons/" + os->iconName(), os, backend, this);
#else
        LinboOsSelectButton* osButton = new LinboOsSelectButton("/icons/" + os->iconName(), os, backend, this);
#endif
        connect(osButton, &LinboOsSelectButton::imageCreationRequested, this, &LinboOsSelectionRow::imageCreationRequested);
        connect(osButton, &LinboOsSelectButton::imageUploadRequested, this, &LinboOsSelectionRow::imageUploadRequested);

        this->_osButtons.append(osButton);
    }

    if(this->_osButtons.length() == 0) {
        //% "No Operating system configured in start.conf"
        this->_noOsLabel = new QLabel(qtTrId("osSelection_noOperatingSystems"), this);
        this->_noOsLabel->hide();
        this->_noOsLabel->setAlignment(Qt::AlignCenter);
        this->_noOsLabelFont.setBold(true);
        this->_noOsLabel->setFont(this->_noOsLabelFont);
        this->_noOsLabel->setStyleSheet(gTheme->insertValues("QLabel { color: %TextColor; }"));

        QString environmentValuesText;
        //% "Hostname"
        environmentValuesText += qtTrId("hostname") + ":  " + this->_backend->config()->hostname() + "\n";
        //% "IP-Address"
        environmentValuesText += qtTrId("ip") + ":  " + this->_backend->config()->ipAddress() + "\n";
        //% "Mac"
        environmentValuesText += qtTrId("client_info_mac") + ":  " + this->_backend->config()->macAddress() + "\n";

        this->_environmentValuesLabel = new QLabel(environmentValuesText, this);
        this->_environmentValuesLabel->hide();
        this->_environmentValuesLabel->setAlignment(Qt::AlignCenter);
        this->_environmentValuesLabel->setFont(this->_environmentValuesLabelFont);
        this->_environmentValuesLabel->setStyleSheet(gTheme->insertValues("QLabel { color: %TextColor; }"));
    }
    else {
        this->_osButtons[0]->_button->setChecked(true);
    }

    this->_handleLinboStateChanged(this->_backend->state());
}

void LinboOsSelectionRow::_resizeAndPositionAllButtons(int heightOverride, int widthOverride) {

    heightOverride = this->height();
    widthOverride = this->width();

    if(this->_sizeOverride != nullptr)
        heightOverride = this->_sizeOverride->height();

    if(this->_sizeOverride != nullptr)
        widthOverride = this->_sizeOverride->width();

    if(this->_osButtons.length() > 0) {

        int buttonCount = this->_osButtons.length();

        int spacing = heightOverride * 0.04;
        int buttonWidth;
        int buttonHeight;

        if (buttonCount <= 2) {
            // Single column, stacked vertically, horizontal row cards
            buttonWidth = std::min((int)(widthOverride * 0.475), widthOverride - spacing * 2);
            buttonHeight = std::min((heightOverride - spacing * (buttonCount + 1)) / std::max(buttonCount, 1),
                                    (int)(heightOverride * 0.35));

            int totalHeight = buttonHeight * buttonCount + spacing * (buttonCount - 1);
            int startY = (heightOverride - totalHeight) / 2;
            int startX = (widthOverride - buttonWidth) / 2;

            for(int i = 0; i < this->_osButtons.length(); i++) {
                bool visible = true;
                QRect geometry = this->_osButtons[i]->geometry();

                if(this->_osButtons[i]->_os != this->_backend->osOfCurrentAction() || !this->_showOnlyButtonOfOsOfCurrentAction) {
                    visible = !this->_showOnlyButtonOfOsOfCurrentAction;
                    geometry = QRect(startX, startY + i * (buttonHeight + spacing), buttonWidth, buttonHeight);
                }
                else {
                    visible = true;
                    int singularWidth = std::min(buttonWidth, (int)(heightOverride * 1.5));
                    geometry = QRect((widthOverride - singularWidth) / 2, (heightOverride - buttonHeight) / 2, singularWidth, buttonHeight);
                }

                if(this->_inited) {
                    this->_osButtons[i]->_setVisibleAnimated(visible);
                    QPropertyAnimation* moveAnimation = new QPropertyAnimation(this);
                    moveAnimation->setPropertyName("geometry");
                    moveAnimation->setEasingCurve(QEasingCurve::InOutQuad);
                    moveAnimation->setDuration(300);
                    moveAnimation->setTargetObject(this->_osButtons[i]);
                    moveAnimation->setStartValue(this->_osButtons[i]->geometry());
                    moveAnimation->setEndValue(geometry);
                    moveAnimation->start();
                    connect(moveAnimation, &QPropertyAnimation::finished, moveAnimation, &QPropertyAnimation::deleteLater);
                }
                else {
                    this->_osButtons[i]->setVisible(visible);
                    this->_osButtons[i]->setGeometry(geometry);
                }
            }
        }
        else {
            // 2-column grid for 3-4 OS entries
            int rows = (buttonCount + 1) / 2;
            buttonWidth = std::min((int)(widthOverride * 0.475), (widthOverride - spacing * 3) / 2);
            buttonHeight = std::min((heightOverride - spacing * (rows + 1)) / rows,
                                    (int)(heightOverride * 0.35));

            int totalWidth = buttonWidth * 2 + spacing;
            int totalHeight = buttonHeight * rows + spacing * (rows - 1);
            int startX = (widthOverride - totalWidth) / 2;
            int startY = (heightOverride - totalHeight) / 2;

            for(int i = 0; i < this->_osButtons.length(); i++) {
                bool visible = true;
                QRect geometry = this->_osButtons[i]->geometry();

                if(this->_osButtons[i]->_os != this->_backend->osOfCurrentAction() || !this->_showOnlyButtonOfOsOfCurrentAction) {
                    visible = !this->_showOnlyButtonOfOsOfCurrentAction;
                    int col = i % 2;
                    int row = i / 2;
                    geometry = QRect(startX + col * (buttonWidth + spacing),
                                     startY + row * (buttonHeight + spacing),
                                     buttonWidth, buttonHeight);
                }
                else {
                    visible = true;
                    int singularWidth = std::min(buttonWidth, (int)(heightOverride * 1.5));
                    geometry = QRect((widthOverride - singularWidth) / 2, (heightOverride - buttonHeight) / 2, singularWidth, buttonHeight);
                }

                if(this->_inited) {
                    this->_osButtons[i]->_setVisibleAnimated(visible);
                    QPropertyAnimation* moveAnimation = new QPropertyAnimation(this);
                    moveAnimation->setPropertyName("geometry");
                    moveAnimation->setEasingCurve(QEasingCurve::InOutQuad);
                    moveAnimation->setDuration(300);
                    moveAnimation->setTargetObject(this->_osButtons[i]);
                    moveAnimation->setStartValue(this->_osButtons[i]->geometry());
                    moveAnimation->setEndValue(geometry);
                    moveAnimation->start();
                    connect(moveAnimation, &QPropertyAnimation::finished, moveAnimation, &QPropertyAnimation::deleteLater);
                }
                else {
                    this->_osButtons[i]->setVisible(visible);
                    this->_osButtons[i]->setGeometry(geometry);
                }
            }
        }
    }
    else {
        int infoLabelHeight = heightOverride;
        int infoLabelWidth = widthOverride * 0.8;
        int noOsLabelHeight = heightOverride * 0.15;
        this->_noOsLabelFont.setPixelSize(gTheme->toFontSize(noOsLabelHeight * 0.8));
        this->_noOsLabel->setFont(this->_noOsLabelFont);
        this->_noOsLabel->setGeometry((widthOverride - infoLabelWidth) / 2, 0, infoLabelWidth, noOsLabelHeight);
        this->_noOsLabel->show();

        this->_environmentValuesLabelFont.setPixelSize(gTheme->toFontSize(infoLabelHeight * 0.1));
        this->_environmentValuesLabel->setFont(this->_environmentValuesLabelFont);
        this->_environmentValuesLabel->setGeometry((widthOverride - infoLabelWidth) / 2, noOsLabelHeight, infoLabelWidth, infoLabelHeight);
        this->_environmentValuesLabel->show();
    }

    this->_inited = true;
}

void LinboOsSelectionRow::setShowOnlyButtonOfOsOfCurrentAction(bool value) {
    // find selected button
    // set its x so it is in the middle (animated)
    // set Opacity of all other buttons to 0 (animated)
    if(value == this->_showOnlyButtonOfOsOfCurrentAction)
        return;

    this->_showOnlyButtonOfOsOfCurrentAction = value;

    if(this->_inited)
        this->_resizeAndPositionAllButtons();
}

void LinboOsSelectionRow::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    //qDebug() << "RESIZE EVENT: width: " << width() << " height: " << height();
    this->_resizeAndPositionAllButtons();
}

void LinboOsSelectionRow::_handleLinboStateChanged(LinboBackend::LinboState newState) {

    bool buttonsEnabled = true;
    bool showOnlyButtonOfOsOfCurrentAction = this->_showOnlyButtonOfOsOfCurrentAction;

    switch (newState) {
    case LinboBackend::Idle:
    case LinboBackend::Root:
        buttonsEnabled = true;
        showOnlyButtonOfOsOfCurrentAction = false;
        break;

    case LinboBackend::Autostarting:
        buttonsEnabled = true;
        showOnlyButtonOfOsOfCurrentAction = true;
        break;

    case LinboBackend::Starting:
    case LinboBackend::Syncing:
    case LinboBackend::Reinstalling:
    case LinboBackend::CreatingImage:
    case LinboBackend::UploadingImage:
    case LinboBackend::StartActionError:
        buttonsEnabled = false;
        showOnlyButtonOfOsOfCurrentAction = true;
        break;

    case LinboBackend::Partitioning:
    case LinboBackend::UpdatingCache:
    case LinboBackend::RootActionSuccess:
        buttonsEnabled = false;
        break;

    default:
        break;
    }


    for(LinboOsSelectButton* osButton : this->_osButtons) {
        if(showOnlyButtonOfOsOfCurrentAction && osButton->_os != this->_backend->osOfCurrentAction()) {
            osButton->setEnabled(false);
        } else {
            osButton->setEnabled(buttonsEnabled);
        }
    }

    this->setShowOnlyButtonOfOsOfCurrentAction(showOnlyButtonOfOsOfCurrentAction);
}

void LinboOsSelectionRow::setMinimumSizeAnimated(QSize size) {
    if(size.height() < this->height()) {
        this->_sizeOverride = new QSize(size);
        this->_resizeAndPositionAllButtons();
        this->_sizeAnimation->setStartValue(this->size());
        this->_sizeAnimation->setEndValue(size);
        connect(this->_sizeAnimation, &QPropertyAnimation::finished, this, [=] {this->setMinimumSize(size); delete this->_sizeOverride; this->_sizeOverride = nullptr;});
        QTimer::singleShot(300, this, [=] {this->_sizeAnimation->start();});
    }
    else {
        if(this->_sizeOverride != nullptr) {
            delete this->_sizeOverride;
            this->_sizeOverride = nullptr;
        }

        delete this->_sizeOverride;
        this->setFixedSize(size);
    }
}
