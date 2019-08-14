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

#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>

namespace Ui {
class UpdateDialog;
}

class UpdateChecker;

class UpdateDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit UpdateDialog(QWidget *parent = nullptr);
    ~UpdateDialog();

public slots:
    int exec(UpdateChecker& uc);

private slots:
    int exec();
    void checkUpdate();
    void updateFound();
    void slotReceivedUpdateResult(int result);
    void slotToggleReleaseNotes(bool checked);
    void slotToggleCheckUpdateOnStartup(bool checked);
    void resizeToFit();
    
private:
    UpdateChecker *m_updateChecker;
    Ui::UpdateDialog *ui;
    QString get_status();
    QString link(const QString &s);
};

#endif // UPDATEDIALOG_H
