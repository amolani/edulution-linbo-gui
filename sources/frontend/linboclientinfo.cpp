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

#include "linboclientinfo.h"

LinboClientInfo::LinboClientInfo(LinboConfig* config, QWidget *parent) : QWidget(parent)
{

    this->_mainWidget = new QWidget(this);
    this->_mainWidget->setStyleSheet(QString(
        "QWidget#clientInfoContainer {"
        "  background-color: rgba(255,255,255,0.04);"
        "  border: 1px solid rgba(255,255,255,0.07);"
        "  border-radius: 6px;"
        "}"
    ));
    this->_mainWidget->setObjectName("clientInfoContainer");
    this->_mainWidget->setAttribute(Qt::WA_StyledBackground, true);

    this->_mainLayout = new QGridLayout(this->_mainWidget);
    this->_mainLayout->setAlignment(Qt::AlignCenter);
    this->_mainLayout->setContentsMargins(0,0,0,0);

    this->_networkIconWidget = new QSvgWidget(gTheme->iconPath(LinboTheme::NetworkIcon));
    this->_mainLayout->addWidget(this->_networkIconWidget, 0, 0, 1, 1);

    //% "Hostname"
    this->_mainLayout->addWidget(new QLabel("<b>" + qtTrId("hostname") + ":</b>  " + config->hostname()), 0, 1, 1, 1);

    //% "Host group"
    this->_mainLayout->addWidget(new QLabel("<b>" + qtTrId("group") + ":</b>  " + config->hostGroup()), 0, 2, 1, 1);

    //% "IP-Address"
    this->_mainLayout->addWidget(new QLabel("<b>" + qtTrId("ip") + ":</b>  " + config->ipAddress()), 0, 3, 1, 1);

    //% "Mac"
    this->_mainLayout->addWidget(new QLabel("<b>" + qtTrId("client_info_mac") + ":</b>  " + config->macAddress()), 0, 4, 1, 1);

    this->_desktopIconWidget = new QSvgWidget(gTheme->iconPath(LinboTheme::DesktopIcon));
    this->_mainLayout->addWidget(this->_desktopIconWidget, 1, 0, 1, 1);

    //% "HDD"
    this->_mainLayout->addWidget(new QLabel("<b>" + qtTrId("client_info_hdd") + ":</b>  " + config->diskSize()), 1, 1, 1, 1);

    //% "Cache"
    this->_mainLayout->addWidget(new QLabel("<b>" + qtTrId("client_info_cache") + ":</b>  " + config->cacheSize()), 1, 2, 1, 1);

    //% "CPU"
    this->_mainLayout->addWidget(new QLabel("<b>" + qtTrId("client_info_cpu") + ":</b>  " + config->cpuModel()), 1, 3, 1, 1);

    //% "RAM"
    this->_mainLayout->addWidget(new QLabel("<b>" + qtTrId("client_info_ram") + ":</b>  " + config->ramSize()), 1, 4, 1, 1);


    // Frosted container styling applied via _mainWidget objectName
}


void LinboClientInfo::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);

    int iconHeight = this->height() * 0.3;

    this->_mainWidget->setGeometry(0,0,this->width(), this->height());
    this->_mainLayout->setSpacing(this->height() * 0.15);

    this->_networkIconWidget->setFixedSize(iconHeight, iconHeight);
    this->_desktopIconWidget->setFixedSize(iconHeight, iconHeight);

    // set font size and glass-style label colors
    // Labels brighter (77→120), values brighter (153→239)
    QString keyColor = gTheme->textAt(120).name(QColor::HexArgb);
    QString valueColor = gTheme->textAt(239).name(QColor::HexArgb);
    for(int i = 0; i < 10; i++) {
        // skip svg icons
        if(i == 0 || i == 5)
            continue;

        QLayoutItem* labelItem = this->_mainLayout->itemAt(i);

        QLabel* label = static_cast<QLabel*>(labelItem->widget());
        label->setTextFormat(Qt::RichText);
        label->setStyleSheet(QString("QLabel { color: %1; }").arg(valueColor));

        label->setMaximumWidth(this->width() * 0.2);

        QFont labelFont = label->font();
        // 40% larger font (0.17 → 0.24)
        labelFont.setPixelSize(gTheme->toFontSize(this->height() * 0.24));
        label->setFont(labelFont);
    }
}
