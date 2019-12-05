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

#include "mainwindow.h"
#include "version.h"
#include "ui_mainwindow.h"
#include "convertlist.h"
#include "addtaskwizard.h"
#include "aboutffmpegdialog.h"
#include "helpmystiqdialog.h"
#include "optionsdialog.h"
#include "aboutdialog.h"
#include "poweroffdialog.h"
#include "updatedialog.h"
#include "services/paths.h"
#include "services/notification.h"
#include "services/powermanagement.h"
#include "converter/mediaconverter.h"
#include "converter/presets.h"
#include "services/updatechecker.h"
#include "services/constants.h"
#include "services/settingtimer.h"
#include "interactivecuttingdialog.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QMessageBox>
#include <QLabel>
#include <QFileDialog>
#include <QDesktopServices>
#include <QApplication>
#include <QSettings>
#include <QCloseEvent>
#include <QTimer>
#include <QPushButton>
#include <QDebug>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent, const QStringList& fileList) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_presets(new Presets(this)),
    m_list(new ConvertList(m_presets, this)),
    m_argv_input_files(fileList),
    m_elapsedTimeLabel(new QLabel(this)),
    m_timer(new QTimer(this)),
    m_poweroff_button(nullptr),
    m_poweroff_actiongroup(nullptr),
    m_update_checker(new UpdateChecker(this))
{
    ui->setupUi(this);    

    connect(m_list, SIGNAL(task_finished(int)),
            this, SLOT(task_finished(int)));
    connect(m_list, SIGNAL(all_tasks_finished()),
            this, SLOT(all_tasks_finished()));
    connect(m_list, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotListContextMenu(QPoint)));
    connect(m_list, SIGNAL(itemSelectionChanged()),
            this, SLOT(refresh_action_states()));
    connect(m_timer, SIGNAL(timeout()),
            this, SLOT(timerEvent()));
    connect(m_list, SIGNAL(started()),
            this, SLOT(conversion_started()));
    connect(m_list, SIGNAL(stopped()),
            this, SLOT(conversion_stopped()));
    connect(m_update_checker, SIGNAL(receivedResult(int)),
            this, SLOT(received_update_result(int)));

    setup_widgets();
    setup_menus();
    setup_poweroff_button();
    setup_toolbar(Constants::getSpaceSeparatedList("ToolbarEntries"));
    setup_statusbar();
    setup_appicon();

    load_settings();

    refresh_action_states();

    if (!check_execute_conditions()) {
        // Close the window immediately after it has started.
        QTimer::singleShot(0, this, SLOT(close()));
    } else {
        QTimer::singleShot(0, this, SLOT(window_ready()));
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::window_ready()
{
    if (!m_argv_input_files.isEmpty()) {
        add_files(m_argv_input_files);
    }
    QSettings settings;
    if (settings.value("options/check_update_on_startup",
                       Constants::getBool("CheckUpdateOnStartup")).toBool()) {
        if (ask_for_update_permission())
            m_update_checker->checkUpdate();
    }
    refresh_status();
}

void MainWindow::task_finished(int exitcode)
{
    if (exitcode == 0) { // succeed
        Notification::send(this, "MystiQ"
                                 , tr("Conversion finished successfully.")
                                 , NotifyLevel::INFO);
    } else { // failed
        QMessageBox::critical(this, this->windowTitle()
                              , tr("Conversion failed.")
                              , QMessageBox::Ok);
    }
}

void MainWindow::all_tasks_finished()
{
    Notification::send(this, "MystiQ",
                       tr("All tasks have finished."), NotifyLevel::INFO);
    activateWindow(); // notify the user (make taskbar entry blink)
    refresh_action_states();

    if (PowerManagement::implemented() && m_poweroff_button->isChecked()) {
        // show poweroff dialog
        if (PoweroffDialog(this).exec(get_poweroff_behavior()) == QDialog::Accepted) {
            save_settings(); // save settings in case of power failure
        }
    }
}

// Menu Events

void MainWindow::slotAddFiles()
{
    add_files();
}

void MainWindow::slotOptions()
{
    OptionsDialog dialog(this);
    dialog.exec();
}

void MainWindow::slotSetTools()
{
    OptionsDialog dialog(this);
    dialog.exec_tools();
}

void MainWindow::slotExit()
{
    this->close();
}

void MainWindow::slotStartConversion()
{
    if (m_list->isEmpty()) {
        QMessageBox::information(this, this->windowTitle(),
                                 tr("Nothing to convert."), QMessageBox::Ok);
    } else {
        m_list->start();
    }
}

void MainWindow::slotStopConversion()
{
    m_list->stop();
}

void MainWindow::slotSetConversionParameters()
{
    if (m_list->selectedCount() > 0) {
        m_list->editSelectedParameters();
    }
}

// Open the output folder of the file.
void MainWindow::slotOpenOutputFolder()
{
    const ConversionParameters *param = m_list->getCurrentIndexParameter();
    if (param) {
        QString folder_path = QFileInfo(param->destination).path();
        if (QFileInfo(folder_path).exists()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(folder_path));
        }
    }
}

void MainWindow::slotAboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::slotAboutFFmpeg()
{
    AboutFFmpegDialog(this).exec();
}

void MainWindow::slotAbout()
{
    AboutDialog(this).exec();
}

void MainWindow::slotHelpMystiQDialog()
{
    HelpMystiQDialog(this).exec();
}

void MainWindow::slotDonate()
{
    QMessageBox d(this);
        d.setWindowTitle(tr("Support MystiQ"));
        QPushButton * ok_button = d.addButton(tr("Donate"), QMessageBox::YesRole);
        d.addButton(tr("Close"), QMessageBox::NoRole);
        d.setDefaultButton(ok_button);
        d.setText("<h1>" + tr("MystiQ needs you") + "</h1><p>" +
            tr("MystiQ is free software. However the development requires a lot of time and a lot of work. In order to keep developing MystiQ with new features we need your help. Please consider to support the MystiQ project by sending a donation. <b>Even the smallest amount will help a lot.</b>")
        );
        d.exec();
        if (d.clickedButton() == ok_button) {
            QDesktopServices::openUrl(QUrl("http://paypal.me/webmisolutions"));
        }
}

void MainWindow::slotReport()
{
#ifdef Q_OS_WIN
    QString eol = "\r\n";
#endif
#ifdef Q_OS_LINUX
    QString eol = "\n";
#endif
    QStringList stringList;
    stringList << "mailto:";
    stringList << "llamaret@webmisolutions.com";
    stringList << "?";
    stringList << "subject=";
    stringList << QString( tr("Reporting bugs from MystiQ ")+ tr("%1").arg(VERSION_STRING) );
    stringList << "&";
    stringList << "body=";
    stringList << tr("Your comment:");
    stringList << eol;
    stringList << eol;
    stringList << eol;
    stringList << eol;
    stringList << "--------------------------------";
    stringList << eol;
    stringList << tr("Report:");
    stringList << eol;
    stringList << eol;
    stringList << eol;
    stringList << eol;
    stringList << "--------------------------------";

    QString string = stringList.join( "" );
    bool b = QDesktopServices::openUrl( QUrl( string, QUrl::TolerantMode ) );
    Q_UNUSED(b)
}

void MainWindow::slotShowUpdateDialog()
{
    if (ask_for_update_permission()) {
        UpdateChecker update_checker;
        UpdateDialog(this).exec(update_checker);
    }
}

void MainWindow::slotCut()
{
    m_list->cutSelectedTask();
}

void MainWindow::slotListContextMenu(QPoint /*pos*/)
{
    refresh_action_states();

    QMenu menu;
    menu.addAction(ui->actionOpenOutputFolder);
    menu.addSeparator();
    menu.addAction(ui->actionRemoveSelectedItems);
    menu.addSeparator();
    menu.addAction(ui->actionRetry);
    menu.addAction(ui->actionRetryAll);
    menu.addSeparator();
    menu.addAction(ui->actionShowErrorMessage);
    menu.addAction(ui->actionChangeOutputFilename);
    menu.addAction(ui->actionChangeOutputDirectory);
    menu.addAction(ui->actionSetParameters);
    menu.addAction(ui->actionCut);

    menu.exec(QCursor::pos());
}

// Events

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_list->isBusy()) {
        int reply = QMessageBox::warning(this, this->windowTitle(),
                             tr("Conversion is still in progress. Abort?"),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (reply == QMessageBox::No) {
            event->ignore();
            return;
        }
    }

    m_list->stop();

    save_settings();
}

void MainWindow::timerEvent()
{
    refresh_status();
}

void MainWindow::conversion_started()
{
    m_elapsedTimeLabel->clear();
    m_timer->start(1000);
    refresh_status();
    refresh_action_states();
}

void MainWindow::conversion_stopped()
{
    m_timer->stop();
    refresh_status();
    refresh_action_states();
}

void MainWindow::update_poweroff_button(int id)
{    
    const char *icon_id = "";
    QString title = "Shutdown Options";
    QString status_tip = "Shutdown Options";
    switch (id) {
    case PowerManagement::SHUTDOWN:
        icon_id = ":/actions/icons/system_shutdown.svg";
        title = tr("Shutdown");
        status_tip = tr("Shutdown when all tasks are done.");
        break;
    case PowerManagement::SUSPEND:
        icon_id = ":/actions/icons/system_suspend.svg";
        title = tr("Suspend");
        status_tip = tr("Suspend when all tasks are done.");
        break;
    case PowerManagement::HIBERNATE:
        icon_id = ":/actions/icons/system_hibernate.svg";
        title = tr("Hibernate");
        status_tip = tr("Hibernate when all tasks are done.");
        break;
    default:
        Q_ASSERT_X(false, __FUNCTION__, "Incorrect id! Be sure to handle every power action in switch().");
    }
    m_poweroff_button->setIcon(QIcon(icon_id));
    m_poweroff_button->setToolTip(status_tip);
    m_poweroff_button->setStatusTip(status_tip);
    ui->actionPoweroff->setIcon(QIcon(icon_id));
    ui->actionPoweroff->setText(title);
    ui->actionPoweroff->setStatusTip(status_tip);
    m_poweroff_button->setIcon(QIcon(icon_id));
    ui->actionPoweroff->setIcon(QIcon(icon_id));


}

void MainWindow::received_update_result(int status)
{
    if (status == UpdateChecker::UpdateFound) {
        QSettings settings;
        SettingTimer timer("mainwindow/last_remind_update_time");
        const int seconds_per_day = 86400;
        const unsigned int prev_update_version =
                settings.value("mainwindow/last_remind_update_version").toUInt();
        const unsigned int new_update_version = m_update_checker->versionId();
        const bool timeout = !timer.isValid()
                || timer.elapsedSeconds() > seconds_per_day;
        const bool is_different_version =
                new_update_version != prev_update_version;

        // Show update dialog only if the update dialog has not been shown
        // for a certain period or the version is different.
        if (timeout || is_different_version) {
            UpdateDialog(this).exec(*m_update_checker);
            timer.restart();
            settings.setValue("mainwindow/last_remind_update_version",
                              new_update_version);
        }
    }
}

// Private Methods

/* Check if all execute conditions are met.
   This function should return true if all the conditions are met
   and return false if any of the conditions fails.
*/
bool MainWindow::check_execute_conditions()
{
    QString errmsg;

    // check external programs
    if (!MediaConverter::checkExternalPrograms(errmsg)) {
        QMessageBox::critical(this, this->windowTitle(), errmsg);
#ifdef TOOLS_IN_DATA_PATH
        return false; // fatal: ffmpeg should be in the data path but doesn't exist
#else
        QTimer::singleShot(0, this, SLOT(slotSetTools()));
#endif
    }
    // load presets
    if (!load_presets())
        return false;

    return true;
}

// We should respect the user and ask before connecting to the Internet to
// check for updates.
// If the user says yes, remember the decision and don't ask next time.
// If the user says no, disable checking for updates on startup.
bool MainWindow::ask_for_update_permission()
{
    const char *setting_key = "update_permission";
    QSettings settings;
    bool permitted = settings.value(setting_key, false).toBool();
    if (permitted) return true;

    QString msg = tr("This program is going to check for updates online. "
                     "Do you allow this program to use the Internet "
                     "to check for updates?");

    int reply = QMessageBox::information(this,
                                          windowTitle(),
                                          msg,
                                          QMessageBox::Yes,
                                          QMessageBox::No);

    if (reply == QMessageBox::Yes) { // permitted
        settings.setValue(setting_key, true); // don't ask next time
        return true;
    } else { // rejected
        // disable auto update because the user probably doesn't like it
        settings.setValue("options/check_update_on_startup", false);
        return false;
    }
}

// Popup wizard to add tasks.
void MainWindow::add_files()
{
    AddTaskWizard wizard(m_presets, this);

    if (wizard.exec_openfile() == QDialog::Accepted) {
        // Add all input files to the list.
        const QList<ConversionParameters> &paramList = wizard.getConversionParameters();
        m_list->addTasks(paramList);
    }
}

void MainWindow::add_files(const QStringList &fileList)
{
    AddTaskWizard wizard(m_presets, this);

    if (wizard.exec(fileList) == QDialog::Accepted) {
        // Add all input files to the list.
        const QList<ConversionParameters> &paramList = wizard.getConversionParameters();
        m_list->addTasks(paramList);
    }
}

void MainWindow::setup_widgets()
{
    // list
    ui->layoutListPlaceholder->addWidget(m_list);
    m_list->adjustSize();
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);

    this->m_elapsedTimeLabel->clear();
}

void MainWindow::setup_menus()
{
    /* === Menu Events === */

    // File
    connect(ui->actionAddFiles, SIGNAL(triggered()),
            this, SLOT(slotAddFiles()));
    connect(ui->actionOptions, SIGNAL(triggered()),
            this, SLOT(slotOptions()));
    connect(ui->actionExit, SIGNAL(triggered()),
            this, SLOT(slotExit()));

    // Edit
    connect(ui->menuEdit, SIGNAL(aboutToShow()),
            this, SLOT(refresh_action_states()));
    connect(ui->actionRemoveSelectedItems, SIGNAL(triggered()),
            m_list, SLOT(removeSelectedItems()));
    connect(ui->actionRemoveCompletedItems, SIGNAL(triggered()),
            m_list, SLOT(removeCompletedItems()));
    connect(ui->actionClearList, SIGNAL(triggered()),
            m_list, SLOT(clear()));
    connect(ui->actionSetParameters, SIGNAL(triggered()),
            this, SLOT(slotSetConversionParameters()));
    connect(ui->actionOpenOutputFolder, SIGNAL(triggered()),
            this, SLOT(slotOpenOutputFolder()));
    connect(ui->actionChangeOutputFilename, SIGNAL(triggered()),
            m_list, SLOT(changeSelectedOutputFile()));
    connect(ui->actionChangeOutputDirectory, SIGNAL(triggered()),
            m_list, SLOT(changeSelectedOutputDirectory()));
    connect(ui->actionShowErrorMessage, SIGNAL(triggered()),
            m_list, SLOT(showErrorMessage()));
    connect(ui->actionCut, SIGNAL(triggered()), SLOT(slotCut()));
    ui->actionCut->setVisible(InteractiveCuttingDialog::available());

    // Convert
    connect(ui->menuConvert, SIGNAL(aboutToShow()),
            this, SLOT(refresh_action_states()));
    connect(ui->actionStartConversion, SIGNAL(triggered()),
            this, SLOT(slotStartConversion()));
    connect(ui->actionStopConversion, SIGNAL(triggered()),
            this, SLOT(slotStopConversion()));
    connect(ui->actionRetry, SIGNAL(triggered()),
            m_list, SLOT(retrySelectedItems()));
    connect(ui->actionRetry, SIGNAL(triggered()),
            this, SLOT(refresh_action_states()));
    connect(ui->actionRetryAll, SIGNAL(triggered()),
            m_list, SLOT(retryAll()));
    connect(ui->actionRetryAll, SIGNAL(triggered()),
            this, SLOT(refresh_action_states()));

    // About
    connect(ui->actionHelpMystiQDialog, SIGNAL(triggered()),
            this, SLOT(slotHelpMystiQDialog()));
    connect(ui->actionAboutQt, SIGNAL(triggered()),
            this, SLOT(slotAboutQt()));
    connect(ui->actionAboutFFmpeg, SIGNAL(triggered()),
            this, SLOT(slotAboutFFmpeg()));
    connect(ui->actionAbout, SIGNAL(triggered()),
            this, SLOT(slotAbout()));
    connect(ui->actionReport, SIGNAL(triggered()),
            this, SLOT(slotReport()));
    connect(ui->actionDonate, SIGNAL(triggered()),
            this, SLOT(slotDonate()));
    connect(ui->actionCheckUpdate, SIGNAL(triggered()),
            this, SLOT(slotShowUpdateDialog()));
}

void MainWindow::setup_toolbar(const QStringList &entries)
{
    Q_ASSERT(m_poweroff_button && "setup_poweroff_button() must be called first");

    // construct a table of available actions
    // map action name to action pointer
    QMap<QString, QAction*> toolbar_table;
#define ADD_ACTION(name) toolbar_table[QString(#name).toUpper()] = ui->action ## name
    ADD_ACTION(AddFiles);
    ADD_ACTION(Options);
    ADD_ACTION(Exit);
    ADD_ACTION(RemoveSelectedItems);
    ADD_ACTION(RemoveCompletedItems);
    ADD_ACTION(ClearList);
    ADD_ACTION(OpenOutputFolder);
    ADD_ACTION(SetParameters);
    ADD_ACTION(ChangeOutputFilename);
    ADD_ACTION(ChangeOutputDirectory); // TODO: rename to "folder"
    ADD_ACTION(ShowErrorMessage);
    ADD_ACTION(StartConversion);
    ADD_ACTION(StopConversion);
    ADD_ACTION(Retry);
    ADD_ACTION(RetryAll);
    // "Shutdown" button is special, so we don't add it here
#define POWEROFF_BUTTON_NAME "POWEROFF"
    ADD_ACTION(AboutQt);
    ADD_ACTION(AboutFFmpeg);
    ADD_ACTION(About);
    ADD_ACTION(CheckUpdate);
    ADD_ACTION(Report);
    ADD_ACTION(Donate);
    ADD_ACTION(HelpMystiQDialog);

    for (int i=0; i<entries.size(); i++) {
        QString entry = entries[i].toUpper(); // case-insensitive compare
        if (entry == POWEROFF_BUTTON_NAME && PowerManagement::implemented())
            ui->toolBar->addWidget(m_poweroff_button);
        else if (entry == "|") // separator
            ui->toolBar->addSeparator();
        else if (toolbar_table.contains(entry))
            ui->toolBar->addAction(toolbar_table[entry]);
    }
}

void MainWindow::setup_statusbar()
{
    ui->statusBar->addPermanentWidget(m_elapsedTimeLabel);
}

/*
 * Setup the poweroff button and menu.
 * The poweroff button is handled differently from other menu and buttons.
 * Its icon and title changes as the action changes.
 * When this function finishes, m_poweroff_button will point to the constructed
 * button widget.
 */
void MainWindow::setup_poweroff_button()
{
    QToolButton *button = new QToolButton(this);
    QMenu *menu = new QMenu(this);
    QList<QAction*> actionList;
    QActionGroup *checkGroup = new QActionGroup(this);

    m_poweroff_button = button;
    m_poweroff_actiongroup = checkGroup;

    // Insert all actions into the list.
    for (int i=0; i<PowerManagement::ACTION_COUNT; i++) {
        const char *icon_id = "";
        QString text = "Shutdown Options";
        switch (i) {
        case PowerManagement::SHUTDOWN:
            //: Shutdown the computer (completely poweroff)
            text = tr("Shutdown");
            icon_id = ":/actions/icons/system_shutdown.svg";
            break;
        case PowerManagement::SUSPEND:
            //: Suspend the computer (sleep to ram, standby)
            text = tr("Suspend");
            icon_id = ":/actions/icons/system_suspend.svg";
            break;
        case PowerManagement::HIBERNATE:
            //: Hibernate the computer (sleep to disk, completely poweroff)
            text = tr("Hibernate");
            icon_id = ":/actions/icons/system_hibernate.svg";
            break;
        default:
            Q_ASSERT_X(false, __FUNCTION__, "Incorrect id! Be sure to implement every power action in switch().");
        }

        QIcon icon(icon_id);

        actionList.append(new QAction(QIcon(icon)
                                      , text, this));
    }

    // Add all actions into the menu (from list)
    foreach (QAction *action, actionList) {
        menu->addAction(action);
        action->setCheckable(true);
        action->setActionGroup(checkGroup);
    }

    button->setMenu(menu);
    button->setPopupMode(QToolButton::MenuButtonPopup);

    // ensure that the toolbutton and actionPoweroff are both checkable
    ui->actionPoweroff->setCheckable(true);
    button->setCheckable(true);
    button->setChecked(false);

    /* Synchronize the checked state of the toolbutton and actionPoweroff.
       This cyclic connection doesn't cause an infinite loop because
       toggled(bool) is only triggered when the checked() state changes.
     */
    connect(button, SIGNAL(toggled(bool))
            , ui->actionPoweroff, SLOT(setChecked(bool)));
    connect(ui->actionPoweroff, SIGNAL(toggled(bool))
            , button, SLOT(setChecked(bool)));

    // update the poweroff button when the action changes
    for (int i=0; i<actionList.size(); i++) {
        QAction *action = actionList.at(i);

        connect (action, &QAction::triggered, [this, i] {
            update_poweroff_button(i);
        });
    }

    actionList.at(0)->trigger();

    /* Check if the power management functions are available.
       If not, hide poweroff button and menus.
     */
    if (!PowerManagement::implemented()) {
        m_poweroff_button->setVisible(false);
        ui->actionPoweroff->setVisible(false);
    }
}

// Fill window icon with multiple sizes of images.
void MainWindow::setup_appicon()
{
    QIcon icon;
    QDir iconDir = QDir(":/app/icons/");
    QStringList fileList = iconDir.entryList();
    QRegExp pattern("^mystiq_[0-9]+x[0-9]+\\.png$");
    foreach (QString file, fileList) {
        if (pattern.indexIn(file) >= 0) {
            icon.addPixmap(QPixmap(iconDir.absoluteFilePath(file)));
        }
    }
    setWindowIcon(icon);
    // Using logo app as About icon
    //ui->actionAbout->setIcon(icon);
}

void MainWindow::set_poweroff_behavior(int action)
{
    if (action >= PowerManagement::ACTION_COUNT)
        action = PowerManagement::SHUTDOWN;
    m_poweroff_actiongroup->actions().at(action)->trigger();
}

int MainWindow::get_poweroff_behavior()
{
    for (int i=0; i<m_poweroff_actiongroup->actions().size(); i++) {
        if (m_poweroff_actiongroup->actions().at(i)->isChecked())
            return i;
    }
    return PowerManagement::SHUTDOWN;
}

bool MainWindow::load_presets()
{
    // The default preset file is located in <datapath>/presets.xml
    QString default_preset_file = ":/other/presets.xml";

    QString local_preset_file;
    if (!Constants::getBool("Portable")) { // non-portable app
        // rename local preset file created by older versions of mystiq
        // operation: mv ~/.mystiq/presets.xml ~/.mystiq/presets.xml.old
        QString local_preset_file_old = QDir(QDir::homePath()).absoluteFilePath(".mystiq/presets.xml");
        if (QFile(local_preset_file_old).exists()) {
            QFile::remove(local_preset_file_old + ".old");
            if (QFile::rename(local_preset_file_old, local_preset_file_old + ".old")) {
                qDebug() << local_preset_file_old + " is no longer used, "
                            "rename to " + local_preset_file_old + ".old";
            }
        }

        // use global preset temporarily
        local_preset_file = default_preset_file;
    } else {
        // portable app
        local_preset_file = default_preset_file;
    }

    QSettings settings;
    bool removeUnavailableCodecs = settings.value("options/hideformats", true).toBool();
    // Load the preset file from the user's home directory
    // The presets are loaded once and shared between objects
    // that need the information.
    if (!m_presets->readFromFile(local_preset_file, removeUnavailableCodecs)) {
        QMessageBox::critical(this, this->windowTitle(),
                              tr("Failed to load preset file. "
                                 "The application will quit now."));
        return false;
    }
    return true;
}

// Hide unused actions
void MainWindow::refresh_action_states()
{
    int selected_file_count = m_list->selectedCount();

    // Hide actionSetParameters if no item in m_list is selected.
    bool hide_SetParameters = (selected_file_count == 0);

    // Hide actionStartConversion if the conversion is in progress.
    bool hide_StartConversion = m_list->isBusy();

    // Hide actionStopConversion if nothing is being converted.
    bool hide_StopConversion = !m_list->isBusy();

    // Show actionOpenOutputFolder only if 1 file is selected.
    bool hide_OpenFolder = (selected_file_count <= 0);

    // Hide actionRemoveSelectedItems if no file is selected.
    bool hide_RemoveSelectedItems = (selected_file_count == 0);

    bool hide_Retry = (selected_file_count == 0);
    bool hide_RetryAll = (m_list->isEmpty());

    bool hide_ClearList = (m_list->isEmpty());

    bool hide_ChangeOutputFilename = m_list->selectedCount() != 1;
    bool hide_ChangeOutputDirectory = m_list->selectedCount() <= 0;


    /* Show actionShowErrorMessage if and only if one task is selected
       and the state of the selected task is FAILED
     */
    bool hide_ShowErrorMessage = (selected_file_count != 1
                                  || !m_list->selectedTaskFailed());

    ui->actionSetParameters->setDisabled(hide_SetParameters);
    ui->actionStartConversion->setDisabled(hide_StartConversion);
    ui->actionStopConversion->setDisabled(hide_StopConversion);
    ui->actionOpenOutputFolder->setDisabled(hide_OpenFolder);
    ui->actionRemoveSelectedItems->setDisabled(hide_RemoveSelectedItems);
    ui->actionRetry->setDisabled(hide_Retry);
    ui->actionRetryAll->setDisabled(hide_RetryAll);
    ui->actionClearList->setDisabled(hide_ClearList);
    ui->actionChangeOutputFilename->setDisabled(hide_ChangeOutputFilename);
    ui->actionChangeOutputDirectory->setDisabled(hide_ChangeOutputDirectory);
    ui->actionShowErrorMessage->setDisabled(hide_ShowErrorMessage);
    ui->actionCut->setEnabled(selected_file_count == 1); // cut only 1 file at a time
}

void MainWindow::load_settings()
{
    QSettings settings;
    restoreGeometry(settings.value("mainwindow/geometry").toByteArray());
    restoreState(settings.value("mainwindow/state").toByteArray());
    int poweroff_behavior = settings.value("options/poweroff_behavior"
                                         , PowerManagement::SHUTDOWN).toInt();
    set_poweroff_behavior(poweroff_behavior);

}

void MainWindow::save_settings()
{
    QSettings settings;
    settings.setValue("mainwindow/geometry", saveGeometry());
    settings.setValue("mainwindow/state", saveState());
    settings.setValue("options/poweroff_behavior", get_poweroff_behavior());
}

void MainWindow::refresh_status()
{
    refresh_statusbar();
    refresh_titlebar();
}

void MainWindow::refresh_statusbar()
{
    if (m_list->isBusy()) {
        int total_seconds = m_list->elapsedTime() / 1000;
        int hours = total_seconds / 3600;
        int minutes = (total_seconds / 60) % 60;
        int seconds = total_seconds % 60;

        QString timeinfo = tr("Elapsed Time: %1 h %2 m %3 s")
                .arg(hours).arg(minutes).arg(seconds);
        //m_elapsedTimeLabel->setText(timeinfo);
        this->m_elapsedTimeLabel->setText(timeinfo);
    } else {
        //m_elapsedTimeLabel->clear();
        //ui->lblTime->clear();
    }
}

void MainWindow::refresh_titlebar()
{
    const int task_count = m_list->count();
    const int finished_task_count = m_list->finishedCount();
    if (finished_task_count < task_count && m_list->isBusy()) {
        //: Converting the %1-th file in %2 files. %2 is the number of files.
        setWindowTitle(tr("MystiQ is Converting %1/%2")
                       .arg(finished_task_count+1).arg(task_count));
    } else {
        setWindowTitle(tr("MystiQ Video Converter"));
    }
}

//void MainWindow::on_pushButton_clicked()
//{
//    this->close();
//}

void MainWindow::on_actionHelpMystiQDialog_triggered()
{

}

void MainWindow::on_actionAbout_triggered()
{

}

void MainWindow::on_actionReport_triggered()
{

}

void MainWindow::on_actionDonate_triggered()
{

}
