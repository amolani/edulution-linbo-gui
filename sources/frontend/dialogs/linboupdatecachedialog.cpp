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

#include "linboupdatecachedialog.h"

LinboUpdateCacheDialog::LinboUpdateCacheDialog(LinboBackend* backend, QWidget* parent) : LinboDialog(parent)
{
    this->_backend = backend;

    //% "Update cache"
    this->setTitle(qtTrId("dialog_updateCache_title"));

    this->_mainLayout = new QVBoxLayout(this);

    // --- Section: Transfer method ---
    this->_methodLabel = new QLabel(QStringLiteral("ÜBERTRAGUNGSMETHODE"));
    this->_mainLayout->addWidget(this->_methodLabel);

    this->_updateTypeButtonGroup = new QButtonGroup(this);

    //% "Update using rsync"
    LinboRadioButton* rsyncButton = new LinboRadioButton(qtTrId("dialog_updateCache_updateType_rsync"));
    rsyncButton->setChecked(backend->config()->downloadMethod() == LinboConfig::Rsync);
    this->_mainLayout->addWidget(rsyncButton);
    this->_updateTypeButtonGroup->addButton(rsyncButton, int(LinboConfig::Rsync));

    //% "Update using multicast"
    LinboRadioButton* multicastButton = new LinboRadioButton(qtTrId("dialog_updateCache_updateType_multicast"));
    multicastButton->setChecked(backend->config()->downloadMethod() == LinboConfig::Multicast);
    this->_mainLayout->addWidget(multicastButton);
    this->_updateTypeButtonGroup->addButton(multicastButton, int(LinboConfig::Multicast));

    //% "Update using torrent"
    LinboRadioButton* torrentButton = new LinboRadioButton(qtTrId("dialog_updateCache_updateType_torrent"));
    torrentButton->setChecked(backend->config()->downloadMethod() == LinboConfig::Torrent);
    this->_mainLayout->addWidget(torrentButton);
    this->_updateTypeButtonGroup->addButton(torrentButton, int(LinboConfig::Torrent));

    // --- Section: Advanced ---
    this->_advancedLabel = new QLabel(QStringLiteral("ERWEITERT"));
    this->_mainLayout->addWidget(this->_advancedLabel);

    //% "Format cache partition"
    _formatCheckBox = new LinboCheckBox(qtTrId("dialog_updateCache_formatPartition"));
    this->_mainLayout->addWidget(_formatCheckBox);

    this->_formatWarning = new QLabel(QStringLiteral("Achtung: Löscht alle gecachten Images"));
    this->_formatWarning->setVisible(false);
    this->_mainLayout->addWidget(this->_formatWarning);
    connect(this->_formatCheckBox, &QCheckBox::toggled, this->_formatWarning, &QLabel::setVisible);

    this->_mainLayout->addStretch();

    // --- Bottom toolbar: Update left, Cancel right (same order as all other dialogs) ---
    //% "update"
    LinboToolButton* updateButton = new LinboToolButton(qtTrId("dialog_updateCache_button_update"));
    this->addToolButton(updateButton);
    connect(updateButton, &LinboToolButton::clicked, this, &LinboUpdateCacheDialog::_updateCache);

    //% "cancel"
    LinboToolButton* cancelButton = new LinboToolButton(qtTrId("cancel"));
    this->addToolButton(cancelButton);
    connect(cancelButton, &LinboToolButton::clicked, this, &LinboUpdateCacheDialog::autoClose);
}

void LinboUpdateCacheDialog::_updateCache() {
    this->_backend->updateCache(LinboConfig::DownloadMethod(this->_updateTypeButtonGroup->checkedId()), this->_formatCheckBox->isChecked());
    this->close();
}

void LinboUpdateCacheDialog::resizeEvent(QResizeEvent *event) {
    LinboDialog::resizeEvent(event);

    int margins = gTheme->size(LinboTheme::Margins);
    int rowHeight = gTheme->size(LinboTheme::RowHeight);
    int rowFontSize = gTheme->size(LinboTheme::RowFontSize);
    int padding = gTheme->size(LinboTheme::RowPaddingSize);

    this->_mainLayout->setContentsMargins(padding, margins, padding, padding);
    this->_mainLayout->setSpacing(padding);

    // Section labels — dim white, uppercase, small, bold
    QFont sectionFont;
    sectionFont.setPixelSize(std::max(8, rowFontSize * 7 / 10));
    sectionFont.setBold(true);
    sectionFont.setLetterSpacing(QFont::AbsoluteSpacing, 1);

    this->_methodLabel->setFont(sectionFont);
    this->_methodLabel->setStyleSheet(QString("color: %1;").arg(gTheme->textAt(120).name(QColor::HexArgb)));

    this->_advancedLabel->setFont(sectionFont);
    this->_advancedLabel->setStyleSheet(QString("color: %1;").arg(gTheme->textAt(120).name(QColor::HexArgb)));
    this->_advancedLabel->setContentsMargins(0, margins, 0, 0);

    // Radio buttons + checkbox
    QFont buttonFont;
    buttonFont.setPixelSize(rowFontSize);
    int contentWidth = this->width() - padding * 2;

    for(QAbstractButton* button : this->_updateTypeButtonGroup->buttons()) {
        button->setFixedSize(contentWidth, rowHeight);
        button->setFont(buttonFont);
    }

    this->_formatCheckBox->setFixedSize(contentWidth, rowHeight);
    this->_formatCheckBox->setFont(buttonFont);

    // Warning label — orange, italic, smaller, indented under checkbox text
    QFont warningFont;
    warningFont.setPixelSize(std::max(8, rowFontSize * 3 / 4));
    warningFont.setItalic(true);
    this->_formatWarning->setFont(warningFont);
    this->_formatWarning->setStyleSheet("color: #e65100;");
    this->_formatWarning->setContentsMargins(rowFontSize + 6, 0, 0, 0);
}
