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

#ifndef PREVIEWDIALOG_H
#define PREVIEWDIALOG_H

#include <QDialog>

namespace Ui {
class PreviewDialog;
}

class MediaPlayerWidget;

class PreviewDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit PreviewDialog(QWidget *parent = nullptr);
    ~PreviewDialog();

    static bool available();

public slots:
    int exec(const QString &filename,
             bool from_begin, int begin_sec,
             bool to_end, int end_sec);

private slots:
    int exec();
    void playSelectedRange();
    void refreshRange();
    
private:
    Ui::PreviewDialog *ui;
    MediaPlayerWidget *m_player;
    int m_beginTime;
    int m_endTime;
    void load_settings();
    void save_settings();
};

#endif // PREVIEWDIALOG_H
