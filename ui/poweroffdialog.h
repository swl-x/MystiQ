/*  MystiQ - a C++/Qt5 gui frontend for ffmpeg
 *  Copyright (C) 2011-2019 Maikel Llamaret Heredia <llamaret@webmisolutions.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef POWEROFFDIALOG_H
#define POWEROFFDIALOG_H

#include <QDialog>

namespace Ui {
    class PoweroffDialog;
}

class QTimer;

class PoweroffDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PoweroffDialog(QWidget *parent = nullptr);
    ~PoweroffDialog();

public slots:
    int exec(int action);

private slots:
    int exec();
    void show();
    void btnExecute_click();
    void btnCancel_click();
    void dialog_accepted();
    void dialog_rejected();
    void timer_event();
    void refresh_message();

private:
    Ui::PoweroffDialog *ui;
    QTimer *m_timer;
    int m_action;
    bool m_success;
    int m_time;
};

#endif // POWEROFFDIALOG_H
