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

#ifndef LINBOOSSELECTBUTTON_H
#define LINBOOSSELECTBUTTON_H

#include <QObject>
#include <QWidget>
#include <QButtonGroup>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QEventLoop>
#include <QLabel>
#include <QPainter>
#include <QSvgRenderer>
#include <QEnterEvent>

#include "linbopushbutton.h"
#include "linboos.h"
#include "linbobackend.h"

class LinboOsSelectButton : public QWidget
{
    Q_OBJECT
public:
    friend class LinboOsSelectionRow;

protected:
    LinboOsSelectButton(QString icon, LinboOs* os, LinboBackend* backend, QWidget* parent = nullptr);
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:

    LinboOs* _getOs();
    void _setVisibleAnimated(bool visible);
    void setVisible(bool visible) override;

    bool _inited;
    bool _shouldBeVisible;
    bool _showDefaultAction;
    bool _isCardHovered;

    LinboBackend* _backend;
    LinboOs* _os;
    LinboPushButton* _button;
    QList<LinboPushButton*> _startActionButtons;
    QList<LinboPushButton*> _rootActionButtons;
    LinboPushButton* _primaryStartPill;
    LinboPushButton* _primaryRootPill;
    QSvgRenderer* _iconRenderer;
    QLabel* _osNameLabel;

private slots:
    void _handleBackendStateChange(LinboBackend::LinboState state);
    void _updateActionButtonVisibility(bool doNotAnimate = false);
    void _handlePrimaryButtonClicked();
    QString _getTooltipContentForAction(LinboOs::LinboOsStartAction action);

signals:
    void imageCreationRequested(LinboOs* os);
    void imageUploadRequested(LinboOs* os);

};

#endif // LINBOOSSELECTBUTTON_H
