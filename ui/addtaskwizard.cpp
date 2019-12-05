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

#include "addtaskwizard.h"
#include "ui_addtaskwizard.h"

#include "converter/presets.h"
#include "conversionparameterdialog.h"
#include "services/extensions.h"
#include "services/paths.h"
#include "services/constants.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QSettings>
#include <QProgressDialog>
#include <cassert>

#define DEFAULT_OUTPUT_TO_SOURCE_DIR Constants::getBool("OutputToSourceFolder")

#define PAGEID_SELECTFILES 0
#define PAGEID_PARAMS 1

#define BUSY_INDICATOR_MINIMUM_DURATION 100

enum OutputPathType
{
    SelectFolder,
    NewFolder,
    SourceFolder
};

AddTaskWizard::AddTaskWizard(Presets *presets, QWidget *parent) :
    QWizard(parent),
    ui(new Ui::AddTaskWizard),
    m_presets(presets),
    m_current_param(new ConversionParameters),
    m_exts(new Extensions())
{
    ui->setupUi(this);

    // setup signals/slots
    connect(ui->btnAdd, SIGNAL(clicked())
            , this, SLOT(slotAddFilesToList()));
    connect(ui->btnRemove, SIGNAL(clicked())
            , this, SLOT(slotRemoveFilesFromList()));
    connect(ui->cbExtension, SIGNAL(currentIndexChanged(int))
            , this, SLOT(slotExtensionSelected(int)));
    connect(ui->cbPreset, SIGNAL(currentIndexChanged(int))
            , this, SLOT(slotPresetSelected(int)));
    connect(ui->btnEditPreset, SIGNAL(clicked())
            , this, SLOT(slotEditPresetButton()));
    connect(ui->btnBrowseOutputPath, SIGNAL(clicked())
            , this, SLOT(slotBrowseOutputPathButton()));
    connect(this, SIGNAL(accepted())
            , this, SLOT(slotFinished()));

    connect(ui->rbSelectFolder, SIGNAL(toggled(bool))
            , ui->cbOutputPath, SLOT(setEnabled(bool)));
    connect(ui->rbSelectFolder, SIGNAL(toggled(bool))
            , ui->btnBrowseOutputPath, SLOT(setEnabled(bool)));
    connect(ui->rbNewFolder, SIGNAL(toggled(bool))
            , ui->txtNewFolderName, SLOT(setEnabled(bool)));
    ui->rbSelectFolder->setChecked(true); // trigger toggled() event
    ui->rbNewFolder->setChecked(true); // trigger toggled() event
    ui->rbSourceFolder->setChecked(true); // trigger toggled() event

    ui->lstFiles->setSelectionMode(QAbstractItemView::ExtendedSelection);

    load_extensions();

    ui->cbOutputPath->setEditable(true);

    load_settings();

    QSettings settings;
    // Load Geometry
    restoreGeometry(settings.value("addtaskwizard/geometry").toByteArray());

    // Hide "auto adjust bitrate" checkbox.
    ui->chkAutoAdjustBitrate->setVisible(false);
}

AddTaskWizard::~AddTaskWizard()
{
    QSettings settings;
    // Save Geometry
    settings.setValue("addtaskwizard/geometry", saveGeometry());
    delete ui;
    delete m_current_param;
    delete m_exts;
}

const QList<ConversionParameters>&
AddTaskWizard::getConversionParameters() const
{
    return m_params;
}

int AddTaskWizard::exec_openfile()
{
    ui->lstFiles->clear();

    if (startId() == PAGEID_SELECTFILES) { // popup select file dialog
        slotAddFilesToList();
        if (ui->lstFiles->count() == 0)
            return QWizard::Rejected;
    }
    return QWizard::exec();
}

int AddTaskWizard::exec(const QStringList &files)
{
    ui->lstFiles->clear();
    addFiles(files);
    return QWizard::exec();
}

bool AddTaskWizard::validateCurrentPage()
{
    switch (currentId()) {
    case 0: // Select input files
        // check if the list is empty
        if (ui->lstFiles->count() != 0) { // complete
            return true;
        } else {
            QMessageBox::information(this, this->windowTitle()
                                     , tr("Please select at least one file."));
            return false;
        }
        //break;
    case 1: // Select conversion parameters
        if (get_output_path_type() == SelectFolder) {
            if (!create_directory(ui->cbOutputPath->currentText()))
                return false;
        } else if (get_output_path_type() == NewFolder) {
            for (int i=0; i<ui->lstFiles->count(); i++) {
                QString input_file = ui->lstFiles->item(i)->text();
                QString output_path = get_output_path(input_file);
                if (!create_directory(output_path, /* do not confirm */ false))
                    return false;
            }
        }
        return true;
        //break;
    }
    return true;
}

void AddTaskWizard::slotAddFilesToList()
{
    /*: This text is the title of an openfile dialog. */
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select Files"),
              m_prev_path,  // default path
              tr("Multimedia") + m_exts->multimedia().forFilter() + ";;" +
              tr("Video") + m_exts->video().forFilter() + ";;" +
              tr("Audio") + m_exts->audio().forFilter() + ";;" +
              tr("All files") + "(*)"
              );

    if (!files.isEmpty()) {
        addFiles(files);
        // Save open file path.
        m_prev_path = QFileInfo(files[0]).path(); // save previous open path
        QSettings settings;
        settings.setValue("addtaskwizard/openfilepath", m_prev_path);

    } else {
        // no file selected
    }
}

void AddTaskWizard::slotRemoveFilesFromList()
{
    QList<QListWidgetItem*> itemList = ui->lstFiles->selectedItems();
    foreach (QListWidgetItem *item, itemList) {
        ui->lstFiles->takeItem(ui->lstFiles->row(item));
    }
}

void AddTaskWizard::slotEditPresetButton()
{
    ConversionParameterDialog dialog(this);
    bool singleFile = (ui->lstFiles->count() == 1);
    ConversionParameters param = *m_current_param;
    dialog.setGeometry(this->x(), this->y(), dialog.width(), dialog.height());

    if (singleFile) {
        param.source = ui->lstFiles->item(0)->text();
    }

    if (dialog.exec(param, singleFile)) {
        m_current_param->copyConfigurationFrom(param);
        m_cbpreset_index = ui->cbPreset->currentIndex();
        ui->cbPreset->setCurrentIndex(-1); // select no item
    }
}

void AddTaskWizard::slotBrowseOutputPathButton()
{
    /*: This text is the title of an open directory dialog. */
    QString selected_path = QFileDialog::getExistingDirectory(this, tr("Select Directory")
                                        , ui->cbOutputPath->currentText());
    if (!selected_path.isEmpty())
        ui->cbOutputPath->setEditText(selected_path);
}

// When the user selects an extension, insert all possible presets
// into the preset combobox.
void AddTaskWizard::slotExtensionSelected(int ext_index)
{
    if (ext_index == -1) return;

    QString extension = ui->cbExtension->itemData(ext_index).toString();
    QList<Preset> presetList;

    ui->cbPreset->clear();
    if (m_presets->getPresets(extension, presetList)) {
        std::sort(presetList.begin(), presetList.end()); // Sort the presets by the order in the xml file.
        foreach (Preset preset, presetList) {
            ui->cbPreset->addItem(preset.label, preset.id);
        }
    }

    // Restore the last used preset of the extension.
    if (ext_index >= 0 && ext_index < m_ext_preset.size()) {
        // index in the preset combobox
        int index = m_ext_preset[ext_index].toInt();
        if (index >= 0 && index < ui->cbPreset->count())
            ui->cbPreset->setCurrentIndex(index);
        else
            ui->cbPreset->setCurrentIndex(0);
    }
}

void AddTaskWizard::slotPresetSelected(int index)
{
    if (index == -1) return;
    unsigned int id = ui->cbPreset->itemData(index).toUInt();
    Preset preset;
    if (!m_presets->findPresetById(id, preset)) {
        return; // assert false
    }
    *m_current_param = ConversionParameters::fromFFmpegParameters(preset.parameters);
}

// This function is executed when the users presses "Finish"
void AddTaskWizard::slotFinished()
{
    const int size = ui->lstFiles->count();
    m_params.clear();

    // The variable "param" is reused in the loop.
    ConversionParameters param(*m_current_param);
    const int ext_index = ui->cbExtension->currentIndex();
    const QString ext = ui->cbExtension->itemData(ext_index).toString();

    // Write conversion parameters to m_params.
    for (int i=0; i<size; i++) {
        QString input_filename = ui->lstFiles->item(i)->text();
        QString input_file_basename = QFileInfo(input_filename).completeBaseName();
        QDir output_dir(get_output_path(input_filename));

        // Fill in input and output filenames
        // IMPORTANT: Only "source" and "destination" should be modified in the loop.
        param.source = input_filename;
        param.destination = output_dir.absoluteFilePath(input_file_basename + "." + ext);

        // Save the configuration for the file.
        m_params.append(param);
    }

    save_settings();
}

void AddTaskWizard::addFiles(const QStringList &files)
{
    // create a busy-indicator dialog
    QProgressDialog dlgProgress(this);
    dlgProgress.setRange(0, 0); // no min/max values, work as busy-indicator
    dlgProgress.setWindowModality(Qt::WindowModal);
    dlgProgress.setMinimumDuration(BUSY_INDICATOR_MINIMUM_DURATION);
    dlgProgress.setAutoClose(false); // don't close when value reaches 0
    dlgProgress.setLabelText(tr("Searching for files..."));
    dlgProgress.show();

    // add files to the list
    QStringList incorrect_files; // Record files that are not valid for conversion.
    foreach (QString file, files) {
        recursively_add_file(file, incorrect_files, dlgProgress);
    }
    dlgProgress.hide();

    // show error message if a file could not be found
    if (!incorrect_files.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText(tr("Some files could not be found."));
        msgBox.setDetailedText(incorrect_files.join("\n"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
}

bool AddTaskWizard::load_extensions()
{
    // update extension combo bar
    QList<QString> extensions;
    m_presets->getExtensions(extensions);
    ui->cbExtension->clear();
    foreach (QString ext, extensions) {
        ui->cbExtension->addItem(ext.toUpper(), QVariant(ext));
    }

    return true;
}

void AddTaskWizard::load_settings()
{
    QSettings settings;

    // extension combobox
    int ext_index = settings.value("addtaskwizard/extension").toInt();
    if (ext_index < 0 || ext_index >= ui->cbExtension->count())
        ext_index = 0;
    ui->cbExtension->setCurrentIndex(ext_index);

    m_ext_preset = settings.value("addtaskwizard/selected_presets").toList().toVector();
    m_ext_preset.resize(ui->cbExtension->count());

    //ui->chkAutoAdjustBitrate->setChecked(
    //      settings.value("addtaskwizard/auto_audio_bitrate", false).toBool());
    ui->chkAutoAdjustBitrate->setChecked(false);

    if (ext_index >= 0 && ext_index < m_ext_preset.size()) {
        // preset combobox
        QApplication::processEvents();

        int preset_index = m_ext_preset[ext_index].toInt();
        if (preset_index < 0 || preset_index >= ui->cbPreset->count())
            preset_index = 0;

        ui->cbPreset->setCurrentIndex(preset_index);
    }

    // open file dialog default path
    m_prev_path = settings.value("addtaskwizard/openfilepath"
                                 , QDir::homePath()).toString();

    // Load recent output paths.
    QStringList recent_paths = settings.value("addtaskwizard/recentpaths").toStringList();
    ui->cbOutputPath->clear();
    ui->cbOutputPath->addItems(recent_paths);
    if (ui->cbOutputPath->count() == 0) {
        ui->cbOutputPath->addItem(QDir::homePath());
    }
    ui->cbOutputPath->setCurrentIndex(0); // Select the most recent path.

    const int output_path_type = settings.value("addtaskwizard/output_path_type").toInt();
    set_output_path_type(output_path_type);

    const QString new_folder_name = settings.value(
                "addtaskwizard/new_folder_name",
                Constants::getString("NewFolderName")).toString();
    ui->txtNewFolderName->setText(new_folder_name);
}

void AddTaskWizard::save_settings()
{
    QSettings settings;
    settings.setValue("addtaskwizard/extension", ui->cbExtension->currentIndex());

    const int ext_index = ui->cbExtension->currentIndex();
    int preset_index = ui->cbPreset->currentIndex();

    if (preset_index < 0) // The user has edited the preset, so no preset is selected.
        preset_index = m_cbpreset_index; // Save the last selected preset instead.

    if (ext_index >= 0 && ext_index < m_ext_preset.size())
        m_ext_preset[ext_index] = preset_index;

    //settings.setValue("addtaskwizard/auto_audio_bitrate", ui->chkAutoAdjustBitrate->isChecked());

    // the last used preset of each extension
    settings.setValue("addtaskwizard/selected_presets", m_ext_preset.toList());

    // Save recent output paths
    QStringList recent_paths;
    recent_paths.push_back(ui->cbOutputPath->currentText()); // Save current text.
    for (int i=0; i<ui->cbOutputPath->count(); i++) {
        QString path = ui->cbOutputPath->itemText(i);
        if (recent_paths.indexOf(path) == -1) // avoid duplicate items
            recent_paths.push_back(ui->cbOutputPath->itemText(i));
    }

    const int num_recent_paths = Constants::getInteger("NumRecentPaths");
    if (recent_paths.size() > num_recent_paths) {
        // Make the list contain at most num_recent_paths items.
        recent_paths = recent_paths.mid(0, num_recent_paths);
    }
    settings.setValue("addtaskwizard/recentpaths", recent_paths);
    settings.setValue("addtaskwizard/output_path_type", get_output_path_type());
    settings.setValue("addtaskwizard/new_folder_name", ui->txtNewFolderName->text());
}

void AddTaskWizard::set_output_path_type(int n)
{
    switch (n)
    {
    case SelectFolder:
        ui->rbSelectFolder->setChecked(true); break;
    case NewFolder:
        ui->rbNewFolder->setChecked(true); break;
    case SourceFolder:
        ui->rbSourceFolder->setChecked(true); break;
    default:
        Q_ASSERT(false);
    }
}

int AddTaskWizard::get_output_path_type()
{
    if (ui->rbNewFolder->isChecked())
        return NewFolder;
    if (ui->rbSourceFolder->isChecked())
        return SourceFolder;
    else
        return SelectFolder;
}

QString AddTaskWizard::get_output_path(const QString &input_filename)
{
    QString input_folder_name(QFileInfo(input_filename).absolutePath());
    QDir input_folder(input_folder_name);
    switch (get_output_path_type()) {
    case SelectFolder:
        return ui->cbOutputPath->currentText();
    case NewFolder:
        return input_folder.absoluteFilePath(ui->txtNewFolderName->text());
    case SourceFolder:
        return input_folder.absoluteFilePath(input_folder_name);
    default:
        Q_ASSERT(false);
    }
    return "";
}

bool AddTaskWizard::create_directory(const QString &dir, bool confirm)
{
    // check if output directory exists
    QDir output_dir(dir);
    if (output_dir.exists()) {
        return true;
    } else { // The folder doesn't exist.
        // Prompt the user to create new folder.
        QMessageBox::StandardButton reply;
        if (confirm)
            reply = QMessageBox::warning(this, this->windowTitle()
                             , tr("Folder does not exist. Create a new folder?")
                             , QMessageBox::Yes | QMessageBox::No);
        else
            reply = QMessageBox::Yes;

        if (reply == QMessageBox::Yes) {
            // The user chooses to create folder.
            qDebug() << "Try to create folder " << output_dir.path();
            bool succeed = QDir().mkpath(output_dir.path());
            if (!succeed) { // failed to create folder
                QMessageBox::critical(this, this->windowTitle()
                                      , tr("Failed to create folder. "
                                           "Please select another output folder.")
                                      , QMessageBox::Ok);
            }
            return succeed;
        } else {
            return false;
        }
    }
}

void AddTaskWizard::recursively_add_file(
        const QString &file, // input
        QStringList &incorrect_files, // output
        QProgressDialog &dlgProgress,
        int depth)
{
    // ignore extensions that are not known as media files when doing
    // recursive search
    bool ignore_unknown_extensions = (depth > 0);

    QFileInfo fileinfo(file);
    if (fileinfo.isFile()) { // file
        if (ignore_unknown_extensions
                && !m_exts->contains(fileinfo.suffix()))
            return; // ignore unknown extensions

        QListWidgetItem *item = new QListWidgetItem(file);
        item->setToolTip(file);
        ui->lstFiles->addItem(item);
    } else if (fileinfo.isDir()) { // directory
        QDir dir(file);
        QStringList children = list_directory(dir);
        foreach (QString child, children) { // traverse directory
            // check for cancel events prior to recursion
            QApplication::processEvents();
            if (dlgProgress.wasCanceled())
                return;
            recursively_add_file(dir.absoluteFilePath(child),
                                 incorrect_files,
                                 dlgProgress,
                                 depth+1);
        }
    } else {
        incorrect_files.append(file);
    }
}

QStringList AddTaskWizard::list_directory(const QDir &dir)
{
    QDir::Filters filters = QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot;
    return dir.entryList(filters);
}

