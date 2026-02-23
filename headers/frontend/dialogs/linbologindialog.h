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

#ifndef LINBOLOGINDIALOG_H
#define LINBOLOGINDIALOG_H

#include <QObject>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QSvgRenderer>
#include <QPropertyAnimation>
#include <QTimer>

#include "linbodialog.h"
#include "linbolineedit.h"
#include "linbobackend.h"

class LinboLoginDialog : public LinboDialog
{
    Q_OBJECT
public:
    LinboLoginDialog(LinboBackend* backend, QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    virtual void setVisibleAnimated(bool visible) override;

private:
    LinboBackend* _backend;
    QLineEdit* _passwordInput;
    QSvgRenderer* _lockRenderer;
    QHBoxLayout* _mainLayout;
    bool _wrongPassword;
    bool _updatingDisplay;
    QString _realPassword;
    QTimer* _shakeTimer;
    int _shakeStep;
    int _shakeOriginX;

private slots:
    void inputFinished();
    void _onTextChanged(const QString &text);
    void _shakeStep_slot();
};

#endif // LINBOLOGINDIALOG_H
