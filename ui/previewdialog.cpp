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

#include <QMessageBox>
#include <QSettings>
#include "mediaplayerwidget.h"
#include "previewdialog.h"
#include "converter/exepath.h"
#include "ui_previewdialog.h"

namespace {
QString sec2hms(int seconds)
{
    int h = seconds / 3600;
    int m = (seconds % 3600) / 60;
    int s = (seconds % 60);
    QString result;
    result.asprintf("%02d:%02d:%02d", h, m, s);
    return result;
}
}

PreviewDialog::PreviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreviewDialog),
    m_player(new MediaPlayerWidget(this))
{
    ui->setupUi(this);
    ui->layoutPlayer->addWidget(m_player);
    connect(ui->btnPlayRange, SIGNAL(clicked()), SLOT(playSelectedRange()));
}

PreviewDialog::~PreviewDialog()
{
    delete ui;
}

bool PreviewDialog::available()
{
    return true;
}

int PreviewDialog::exec(const QString &filename,
                               bool from_begin, int begin_sec,
                               bool to_end, int end_sec)
{
    m_beginTime = from_begin ? -1 : begin_sec;
    m_endTime = to_end ? -1 : end_sec;

    m_player->load(filename, m_beginTime, m_endTime);

    playSelectedRange();
    refreshRange();

    return exec();
}

int PreviewDialog::exec()
{
    load_settings();
    int status = QDialog::exec();
    save_settings();

    return status;
}

void PreviewDialog::playSelectedRange()
{
    m_player->playRange(m_beginTime, m_endTime);
}

void PreviewDialog::refreshRange()
{
    //: noun, the beginning of the video
    QString begin_time = tr("Begin");
    //: noun, the end of the video
    QString end_time = tr("End");

    if (m_beginTime >= 0)
        begin_time = sec2hms(m_beginTime);
    if (m_endTime >= 0)
        end_time = sec2hms(m_endTime);

    //: play the video from time %1 to time %2. %1 and %2 are time in hh:mm:ss format.
    ui->btnPlayRange->setText(tr("Play %1~%2").arg(begin_time).arg(end_time));
}

void PreviewDialog::load_settings()
{
    QSettings settings;
    restoreGeometry(settings.value("preview_dialog/geometry").toByteArray());
}

void PreviewDialog::save_settings()
{
    QSettings settings;
    settings.setValue("preview_dialog/geometry", saveGeometry());
}
