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

#include "conversionparameterdialog.h"
#include "converter/audiofilter.h"
#include "converter/mediaprobe.h"
#include "services/ffplaypreviewer.h"
#include "services/mplayerpreviewer.h"
#include "rangewidgetbinder.h"
#include "rangeselector.h"
#include "timerangeedit.h"
#include "interactivecuttingdialog.h"
#include "previewdialog.h"
#include "ui_conversionparameterdialog.h"
#include <QLayout>
#include <cmath>
#include <QQuickItem>

#define TO_BYTEPERCENT(percent) ((percent) * 256 / 100)
#define TO_PERCENT(bytepercent) ((bytepercent) * 100 / 256)

#define DEFAULT_AUDIO_BITRATE 64
#define DEFAULT_VIDEO_BITRATE 200
#define DEFAULT_AUDIO_CHANNELS 2

ConversionParameterDialog::ConversionParameterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConversionParameterDialog),
    m_timeEdit(new TimeRangeEdit(this)),
    m_rangeSel(new RangeSelector(this)),
    m_previewer(nullptr)
{
    ui->setupUi(this);

    // bind visual range selection and range edit (auto sync between them)
    new RangeWidgetBinder(m_rangeSel, m_timeEdit, this);

    ui->layoutTimeSel->addWidget(m_rangeSel);
    ui->layoutTimeSel->addWidget(m_timeEdit);

    // Setup audio sample rate selection
    ui->cbAudioSampleRate->addItem("44100");
    ui->cbAudioSampleRate->addItem("22050");
    ui->cbAudioSampleRate->addItem("11025");

    connect(ui->btnPreview, SIGNAL(clicked()),
            this, SLOT(preview_time_selection()));
    connect(ui->btnInteractiveCutting, SIGNAL(clicked()), SLOT(interactive_cutting()));
    connect(ui->chkDisableAudio, SIGNAL(toggled(bool)), SLOT(audio_tab_update_enabled_widgets()));
    connect(ui->chkCopyAudio, SIGNAL(toggled(bool)), SLOT(audio_tab_update_enabled_widgets()));
    connect(ui->chkDisableVideo, SIGNAL(toggled(bool)), SLOT(video_tab_update_enabled_widgets()));
    connect(ui->chkInsertSubtitle, SIGNAL(toggled(bool)), SLOT(video_tab_update_enabled_widgets()));
    connect(ui->chkCopyVideo, SIGNAL(toggled(bool)), SLOT(video_tab_update_enabled_widgets()));

    // Hide speed-changing options if sox is not available.
    m_enableAudioProcessing = AudioFilter::available();
    if (!m_enableAudioProcessing)
        ui->groupScaling->setVisible(false);

    m_previewer = create_previewer();

    connect(ui->cropWidget->rootObject(), SIGNAL(cut_up_changed(double)), this, SLOT(onCutUpChanged(double)));
    connect(ui->cropWidget->rootObject(), SIGNAL(cut_bottom_changed(double)), this, SLOT(onCutBottomChanged(double)));
    connect(ui->cropWidget->rootObject(), SIGNAL(cut_left_changed(double)), this, SLOT(onCutLeftChanged(double)));
    connect(ui->cropWidget->rootObject(), SIGNAL(cut_right_changed(double)), this, SLOT(onCutRightChanged(double)));

    connect(ui->cropWidget->rootObject(), SIGNAL(video_loaded(int,int)), this, SLOT(onVideoLoaded(int,int)));
}

ConversionParameterDialog::~ConversionParameterDialog()
{
    delete ui;
}

bool ConversionParameterDialog::exec(ConversionParameters& param, bool single_file)
{
    m_singleFile = single_file;
    m_param = &param;
    read_fields(param);

    if (!param.source.isEmpty()) {
        qDebug() << "Passing file source to QML component";
        ui->cropWidget->rootObject()->setProperty("file_source", QString("file://%1").arg(param.source));
    }

    bool accepted = (QDialog::exec() == QDialog::Accepted);
    if (accepted) {
        write_fields(param);
    }
    return accepted;
}

void ConversionParameterDialog::preview_time_selection()
{
    if (PreviewDialog::available()) {
        PreviewDialog(this).exec(m_param->source,
                                 m_timeEdit->fromBegin(),
                                 m_timeEdit->beginTime(),
                                 m_timeEdit->toEnd(),
                                 m_timeEdit->endTime());
    } else {
        int timeBegin = -1, timeEnd = -1;
        if (!m_timeEdit->fromBegin())
            timeBegin = m_timeEdit->beginTime();
        if (!m_timeEdit->toEnd())
            timeEnd = m_timeEdit->endTime();
        m_previewer->play(m_param->source, timeBegin, timeEnd);
    }
}

void ConversionParameterDialog::interactive_cutting()
{
    if (m_singleFile) {
        InteractiveCuttingDialog(this).exec(m_param->source, m_timeEdit);
    }
}

AbstractPreviewer *ConversionParameterDialog::create_previewer()
{
    //AbstractPreviewer *previewer;
    // Use mplayer by default.
    //previewer = new MPlayerPreviewer(this);
    //if (previewer->available())
    //    return previewer;
    // mplayer not available, use ffplay as fallback
    //delete previewer;
    return new FFplayPreviewer(this);
}

// read the fields from the ConversionParameters
void ConversionParameterDialog::read_fields(const ConversionParameters& param)
{
    // Additional Options
    ui->txtFFmpegOptions->setPlainText(param.ffmpeg_options);

    // Audio Options
    ui->chkDisableAudio->setChecked(param.disable_audio);
    ui->chkCopyAudio->setChecked(param.copy_audio);

    ui->spinAudioBitrate->setValue(param.audio_bitrate);

    ui->cbAudioSampleRate->setEditText(QString::number(param.audio_sample_rate));

    ui->spinChannels->setValue(param.audio_channels);

    if (param.audio_volume)
        ui->spinVolume->setValue(TO_PERCENT(param.audio_volume));
    else
        ui->spinVolume->setValue(100);

    // Video Options
    ui->chkDisableVideo->setChecked(param.disable_video);
    ui->chkInsertSubtitle->setChecked(param.insert_subtitle);
    ui->chkCopyVideo->setChecked(param.copy_video);

    ui->spinVideoBitrate->setValue(param.video_bitrate);

    ui->spinWidth->setValue(param.video_width);
    ui->spinHeight->setValue(param.video_height);

    ui->chkVideoSameQuality->setChecked(param.video_same_quality);
    ui->chkDeinterlace->setChecked(param.video_deinterlace);

    ui->spinCropTop->setValue(param.video_crop_top);
    ui->spinCropBottom->setValue(param.video_crop_bottom);
    ui->spinCropLeft->setValue(param.video_crop_left);
    ui->spinCropRight->setValue(param.video_crop_right);

    // Time Options
    bool show_slider = false;
    m_rangeSel->setVisible(false); // hide slider if this dialog is reused
    if (m_singleFile) {
        // time slider: only show in single file mode
        MediaProbe probe;
        if (probe.run(param.source)) { // probe the source file, blocking call
            // success, set the duration and show the range slider
            int duration = static_cast<int>(probe.mediaDuration());
            m_timeEdit->setMaxTime(duration);
            m_rangeSel->setMaxValue(duration);
            m_rangeSel->setVisible(true);
            show_slider = true;
        }
    }
    bool show_preview_button = show_slider && m_previewer->available();
    bool show_cutting_button = show_slider && InteractiveCuttingDialog::available();
    ui->btnPreview->setVisible(show_preview_button);
    ui->btnInteractiveCutting->setVisible(show_cutting_button);

    if (param.time_begin > 0) {
        m_timeEdit->setBeginTime(static_cast<int>(param.time_begin));
        m_timeEdit->setFromBegin(false);
    } else {
        m_timeEdit->setBeginTime(0);
        m_timeEdit->setFromBegin(true);
    }
    if (param.time_end > 0) {
        m_timeEdit->setEndTime(static_cast<int>(param.time_end));
        m_timeEdit->setToEnd(false);
    } else {
        m_timeEdit->setEndTime(0);
        m_timeEdit->setToEnd(true);
    }
    if (param.speed_scaling)
        ui->spinSpeedFactor->setValue(param.speed_scaling_factor * 100.0);
    else
        ui->spinSpeedFactor->setValue(100.0);
}

//#define QTIME_TO_SECS(t) ((t.hour()) * 3600 + (t.minute()) * 60 + (t.second()))

// write the fields to the ConversionParameters
void ConversionParameterDialog::write_fields(ConversionParameters& param)
{
    // Additional Options
    param = param.fromFFmpegParameters(ui->txtFFmpegOptions->toPlainText());

    // Audio Options
    param.disable_audio = ui->chkDisableAudio->isChecked();
    param.copy_audio = ui->chkCopyAudio->isChecked();
    param.audio_sample_rate = ui->cbAudioSampleRate->currentText().toInt();
    param.audio_bitrate = ui->spinAudioBitrate->value();
    param.audio_channels = ui->spinChannels->value();
    param.audio_volume = TO_BYTEPERCENT(ui->spinVolume->value());

    // Video Options
    param.disable_video = ui->chkDisableVideo->isChecked();
    param.insert_subtitle = ui->chkInsertSubtitle->isChecked();
    param.copy_video = ui->chkCopyVideo->isChecked();
    param.video_bitrate = ui->spinVideoBitrate->value();
    param.video_same_quality = ui->chkVideoSameQuality->isChecked();
    param.video_deinterlace = ui->chkDeinterlace->isChecked();

    param.video_width = ui->spinWidth->value();
    param.video_height = ui->spinHeight->value();

    param.video_crop_top = ui->spinCropTop->value();
    param.video_crop_bottom = ui->spinCropBottom->value();
    param.video_crop_left = ui->spinCropLeft->value();
    param.video_crop_right = ui->spinCropRight->value();

    // Time Options
    if (m_timeEdit->fromBegin())
        param.time_begin = 0;
    else
        param.time_begin = static_cast<unsigned int>(m_timeEdit->beginTime());
    if (m_timeEdit->toEnd())
        param.time_end = 0;
    else // ffmpeg accepts duration, not end time
        param.time_end = static_cast<unsigned int>(m_timeEdit->endTime());
    double speed_ratio = ui->spinSpeedFactor->value();
    if (!m_enableAudioProcessing || std::abs(speed_ratio - 100.0) <= 0.01) {
        param.speed_scaling = false;
        param.speed_scaling_factor = 1.0;
    } else {
        param.speed_scaling = true;
        param.speed_scaling_factor = speed_ratio / 100.0;
    }

}

void ConversionParameterDialog::audio_tab_update_enabled_widgets()
{
    bool disable_audio = ui->chkDisableAudio->isChecked();
    bool copy_audio = ui->chkCopyAudio->isChecked();

    ui->chkDisableAudio->setEnabled(true); // always enabled
    ui->chkCopyAudio->setEnabled(!disable_audio);
    ui->groupAudioOptions->setEnabled(!disable_audio && !copy_audio);
}

void ConversionParameterDialog::video_tab_update_enabled_widgets()
{
    bool disable_video= ui->chkDisableVideo->isChecked();
    bool insert_subtitle= ui->chkInsertSubtitle->isChecked();
    bool copy_video = ui->chkCopyVideo->isChecked();

    ui->chkDisableVideo->setEnabled(true); // always enabled
    //ui->chkInsertSubtitle->setEnabled(true); // always enabled
    ui->chkCopyVideo->setEnabled(!disable_video);
    ui->chkInsertSubtitle->setWindowModified(!insert_subtitle);
    ui->chkInsertSubtitle->setDisabled(disable_video || copy_video);
    ui->groupVideoOptions->setEnabled(!disable_video && !copy_video);
}

void ConversionParameterDialog::onCutUpChanged(double value)
{
    ui->spinCropTop->setValue(static_cast<int> (round(value)));
}

void ConversionParameterDialog::onCutBottomChanged(double value)
{
    qDebug() << "WHOOOT NOTIFY" << value;
    ui->spinCropBottom->setValue(static_cast<int> (round(value)));
}

void ConversionParameterDialog::onCutLeftChanged(double value)
{
    ui->spinCropLeft->setValue(static_cast<int> (round(value)));
}

void ConversionParameterDialog::onCutRightChanged(double value)
{
    ui->spinCropRight->setValue(static_cast<int> (round(value)));
}

void ConversionParameterDialog::onVideoLoaded(const int w, const int h)
{
    ui->spinCropTop->setValue(0);
    ui->spinCropLeft->setValue(0);

    ui->spinCropRight->setValue(w);
    ui->spinCropBottom->setValue(h);
}

void ConversionParameterDialog::on_spinCropTop_valueChanged(int arg1)
{
    QMetaObject::invokeMethod(ui->cropWidget->rootObject(), "top_cut_change",
                              Q_ARG(QVariant, static_cast<double>(arg1)));
}

void ConversionParameterDialog::on_spinCropLeft_valueChanged(int arg1)
{
    QMetaObject::invokeMethod(ui->cropWidget->rootObject(), "left_cut_change",
                              Q_ARG(QVariant, static_cast<double>(arg1)));
}

void ConversionParameterDialog::on_spinCropBottom_valueChanged(int arg1)
{
    QMetaObject::invokeMethod(ui->cropWidget->rootObject(), "bottom_cut_change",
                              Q_ARG(QVariant, static_cast<double>(arg1)));
}

void ConversionParameterDialog::on_spinCropRight_valueChanged(int arg1)
{
    QMetaObject::invokeMethod(ui->cropWidget->rootObject(), "right_cut_change",
                              Q_ARG(QVariant, static_cast<double>(arg1)));
}
