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

#include "linbotextbrowser.h"

LinboTextBrowser::LinboTextBrowser(QWidget* parent) : QTextBrowser(parent)
{
    this->setStyleSheet(
            "QTextBrowser {"
            "    border: 1px solid #333333;"
            "    border-radius: 4px;"
            "    background: #1a1a1a;"
            "    padding: 6px;"
            "    color: #fafafa;"
            "}"
            "QTextBrowser:focus {"
            "    border: 1px solid #0081c6;"
            "}"
        );

    this->verticalScrollBar()->setStyleSheet(
            "QScrollBar:vertical {"
            "    background: #1a1a1a;"
            "    width: 8px;"
            "    margin: 0px;"
            "}"
            "QScrollBar::handle:vertical {"
            "    background: #333333;"
            "    min-height: 20px;"
            "    border-radius: 4px;"
            "}"
            "QScrollBar::add-line:vertical { height: 0px; }"
            "QScrollBar::sub-line:vertical { height: 0px; }"
        );

    this->horizontalScrollBar()->setStyleSheet(
            "QScrollBar:horizontal {"
            "    background: #1a1a1a;"
            "    height: 8px;"
            "    margin: 0px;"
            "}"
            "QScrollBar::handle:horizontal {"
            "    background: #333333;"
            "    min-width: 20px;"
            "    border-radius: 4px;"
            "}"
            "QScrollBar::add-line:horizontal { width: 0px; }"
            "QScrollBar::sub-line:horizontal { width: 0px; }"
        );
}
