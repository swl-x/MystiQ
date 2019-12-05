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

#ifndef CONVERSIONPARAMETERDIALOG_H
#define CONVERSIONPARAMETERDIALOG_H

#include <QDialog>
#include "converter/conversionparameters.h"

namespace Ui {
    class ConversionParameterDialog;
}

class RangeSelector;
class TimeRangeEdit;
class AbstractPreviewer;

class ConversionParameterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConversionParameterDialog(QWidget *parent = nullptr);
    ~ConversionParameterDialog();

    /*! This function blocks until the dialog is closed.
     *  @param param If the user presses OK, the modified parameter is written back.
     *  @param single_file If @a single_file is true, the dialog will probe
     *    the file indicated by @c param.source and show some additional options.
     *    default: false.
     *  @return If the user presses OK, the function returns true.
     *  Otherwise, it returns false.
     */
    bool exec(ConversionParameters& param, bool single_file=false);

private slots:
    void preview_time_selection();
    void interactive_cutting();
    AbstractPreviewer *create_previewer();
    void audio_tab_update_enabled_widgets();
    void video_tab_update_enabled_widgets();

    void onCutUpChanged(double value);
    void onCutBottomChanged(double value);
    void onCutLeftChanged(double value);
    void onCutRightChanged(double value);

    void onVideoLoaded(const int w, const int h);

    void on_spinCropTop_valueChanged(int arg1);

    void on_spinCropLeft_valueChanged(int arg1);

    void on_spinCropBottom_valueChanged(int arg1);

    void on_spinCropRight_valueChanged(int arg1);

private:
    Ui::ConversionParameterDialog *ui;
    void read_fields(const ConversionParameters& param);
    void write_fields(ConversionParameters& param);
    bool m_enableAudioProcessing;
    bool m_singleFile;
    TimeRangeEdit *m_timeEdit;
    RangeSelector *m_rangeSel;
    ConversionParameters *m_param;
    AbstractPreviewer *m_previewer;
};

#endif // CONVERSIONPARAMETERDIALOG_H
