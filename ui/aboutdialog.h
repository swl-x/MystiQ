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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
    class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private slots:
    void on_github_button_clicked();

    void on_gitter_button_clicked();

    void on_facebook_button_clicked();

    void on_twitter_button_clicked();

    void on_transifex_button_clicked();

    void on_liberapay_button_clicked();

    void on_opencollective_button_clicked();

    void on_patreon_button_clicked();

private:
    Ui::AboutDialog *ui;
    QString getTranslators();
    QString getDonations();
    QString trad(const QString& lang, const QString& author);
    QString trad(const QString & lang, const QStringList & authors);
};

#endif // ABOUTDIALOG_H
