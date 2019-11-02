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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>

class ConvertList;

namespace Ui {
    class MainWindow;
}

QT_BEGIN_NAMESPACE
class QLabel;
class QToolButton;
class QActionGroup;
QT_END_NAMESPACE

class Presets;
class UpdateChecker;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /*! Construct the main window
     *  @param parent the parent of the QObject
     *  @param fileList the input files from argv
     */
    explicit MainWindow(QWidget *parent = nullptr, const QStringList& fileList = QStringList());
    ~MainWindow();

private slots:
    void window_ready(); //!< The main window is completely loaded.
    void task_finished(int);
    void all_tasks_finished();

    // Menu Events
    void slotAddFiles();
    void slotOptions();
    void slotSetTools();
    void slotExit();
    void slotStartConversion();
    void slotStopConversion();
    void slotSetConversionParameters();
    void slotOpenOutputFolder();
    void slotAboutQt();
    void slotAboutFFmpeg();
    void slotAbout();
    void slotShowUpdateDialog();
    void slotCut();

    void slotListContextMenu(QPoint);

    void refresh_action_states();
    void timerEvent(); ///< 1-second timer event
    void conversion_started();
    void conversion_stopped();

    void update_poweroff_button(int);
    void received_update_result(int);

protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow *ui;
    Presets *m_presets; //!< the preset loader that lives throughout the program
    ConvertList *m_list;
    const QStringList m_argv_input_files;
    QLabel *m_elapsedTimeLabel;
    QTimer *m_timer;
    QToolButton *m_poweroff_button;
    QActionGroup *m_poweroff_actiongroup;
    UpdateChecker *m_update_checker;
    bool check_execute_conditions();
    bool ask_for_update_permission();
    void add_files();
    void add_files(const QStringList& files);
    void setup_widgets();
    void setup_menus();
    void setup_toolbar(const QStringList& entries);
    void setup_statusbar();
    void setup_poweroff_button();
    void setup_appicon();
    void set_poweroff_behavior(int);
    int get_poweroff_behavior();
    bool load_presets();
    void load_settings();
    void save_settings();
    void refresh_status();
    void refresh_statusbar();
    void refresh_titlebar();
};

#endif // MAINWINDOW_H
