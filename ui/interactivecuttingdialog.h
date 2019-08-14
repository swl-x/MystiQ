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

#ifndef INTERACTIVECUTTINGDIALOG_H
#define INTERACTIVECUTTINGDIALOG_H

#include <QDialog>

namespace Ui {
class InteractiveCuttingDialog;
}

class MediaPlayerWidget;
class TimeRangeEdit;
class RangeSelector;
class ConversionParameters;

class InteractiveCuttingDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit InteractiveCuttingDialog(QWidget *parent = nullptr);
    ~InteractiveCuttingDialog();

    static bool available();

    bool fromBegin() const;
    bool toEnd() const;

    int beginTime() const;
    int endTime() const;

public slots:
    void setFromBegin(bool);
    void setToEnd(bool);
    void setBeginTime(int);
    void setEndTime(int);

    /**
     * @brief Start the dialog to cut a file
     * @param filename the file to process
     * @return QDialog::Accepted if the user selects a range.
     */
    int exec(const QString& filename);

    /**
     * @brief Start the dialog to cut a file and modify @a range if the user presses OK.
     * @param filename the file to process
     * @param range [in,out] the range widget to be modified
     * @return
     */
    int exec(const QString& filename, TimeRangeEdit *range);

    /**
     * @brief Start the dialog to cut @c param->source and modify time_begin and time_end
     *  if the user presses OK.
     * @param param [in,out] pointer to the ConversionParameters object to be modified
     * @return
     */
    int exec(ConversionParameters *param);

private slots:
    int exec();
    void playerStateChanged();
    void set_as_begin();
    void set_as_end();
    void seek_to_selection_begin();
    void seek_to_selection_end();
    void play_selection();
    
private:
    Ui::InteractiveCuttingDialog *ui;
    MediaPlayerWidget *player;
    RangeSelector *m_rangeSel;
    TimeRangeEdit *m_rangeEdit;
    void load_settings();
    void save_settings();
};

#endif // INTERACTIVECUTTINGDIALOG_H
