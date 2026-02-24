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

#include "linboradiobutton.h"

LinboRadioButton::LinboRadioButton(QString label, QWidget* parent) : QRadioButton(label, parent)
{

}

void LinboRadioButton::paintEvent(QPaintEvent *e) {
    int sz = std::max(10, this->font().pixelSize());
    this->setStyleSheet(
        QString(
            "QRadioButton {"
            "    color: #fafafa;"
            "    spacing: 6px;"
            "}"
            "QRadioButton::indicator {"
            "    width: %1px;"
            "    height: %1px;"
            "    border-radius: %2px;"
            "    border: 2px solid rgba(255,255,255,40);"
            "    background: transparent;"
            "}"
            "QRadioButton::indicator:checked {"
            "    border: 2px solid #0081c6;"
            "    background: qradialgradient(cx:0.5, cy:0.5, radius:0.35,"
            "                fx:0.5, fy:0.5,"
            "                stop:0 #0081c6, stop:1 #0081c6);"
            "}"
            "QRadioButton::indicator:disabled {"
            "    border: 2px solid rgba(255,255,255,20);"
            "    background: rgba(255,255,255,5);"
            "}"
        )
        .arg(sz)
        .arg(sz / 2));

    QRadioButton::paintEvent(e);
}
