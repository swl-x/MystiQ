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

#include <QTreeWidget>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>
#include <QUrl>
#include <QDebug>
#include <QFileInfo>
#include <QProgressDialog>
#include <QSettings>
#include <QMenu>
#include <QFileDialog>
#include <QInputDialog>
#include <QDesktopServices>
#include <QTextDocument>
#include <QMimeData>
#include <QtMath>
#include <cassert>

#include "convertlist.h"
#include "progressbar.h"
#include "converter/mediaconverter.h"
#include "converter/mediaprobe.h"
#include "services/filepathoperations.h"
#include "services/constants.h"
#include "ui/conversionparameterdialog.h"
#include "ui/interactivecuttingdialog.h"
#include "addtaskwizard.h"
#include "services/extensions.h"

#define TIMEOUT 3000
#define MIN_DURATION 100 // Minimum duration(milliseconds) to show progress dialog.

namespace {
QString htmlEscape(QString s) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    return s.toHtmlEscaped();
#else // Qt4
    return Qt::escape(s);
#endif
}
}

class Task : public QObject
{
public:
    explicit Task(QObject *parent = nullptr) : QObject(parent) { }
    virtual ~Task();
    enum TaskStatus { QUEUED, RUNNING, FINISHED, FAILED };
    int id;
    TaskStatus status;
    ConversionParameters param;
    QTreeWidgetItem *listitem;
    QString errmsg;
};

Task::~Task() {}

/* This enum defines the columns of the list.
   The last item is always NUM_COLUMNS, which is used to identify
   how many columns there are. To add a new item, just follow the
   following steps:

     (1) Add a new enumeration constant to ConvertListColumns before
         NUM_COLUMNS.

     (2) Search for the function "init_treewidget_fill_column_titles"
         and fill in the title of the new field there. Read the
         instruction before the function body for details.

     (3) Search for the function "init_treewidget_columns_visibility"
         and set the default visibility of the field.

     (4) Search for the function "fill_list_fields". This function is
         called when a new task is being added to the list. Fill in
         the field according to the parameter and the probing result.

 */
enum ConvertListColumns
{
    COL_SOURCE,
    COL_DESTINATION,
    COL_DURATION,
    COL_FILE_SIZE,
    COL_AUDIO_SAMPLE_RATE,
    COL_AUDIO_BITRATE,
    COL_AUDIO_CHANNELS,
    COL_AUDIO_CODEC,
    COL_VIDEO_DIMENSIONS,
    COL_VIDEO_BITRATE,
    COL_VIDEO_FRAMERATE,
    COL_VIDEO_CODEC,
    COL_VIDEO_SUBTITLE,
    COL_PROGRESS,
    NUM_COLUMNS
};

class ConvertList::ListEventFilter : public QObject
{
public:
    ListEventFilter(ConvertList *parent) : QObject(parent), m_parent(parent) { }
    virtual ~ListEventFilter();

    // Propagate events from the list to its parent.
    bool eventFilter(QObject */*object*/, QEvent *event)
    {
        switch (event->type()) {
        case QEvent::KeyPress:
            return m_parent->list_keyPressEvent(static_cast<QKeyEvent*>(event));
        case QEvent::DragEnter:
            m_parent->list_dragEnterEvent(static_cast<QDragEnterEvent*>(event));
            return true;
        case QEvent::DragMove:
            m_parent->list_dragMoveEvent(static_cast<QDragMoveEvent*>(event));
            return true;
        case QEvent::DragLeave:
            m_parent->list_dragLeaveEvent(static_cast<QDragLeaveEvent*>(event));
            return true;
        case QEvent::Drop:
            m_parent->list_dropEvent(static_cast<QDropEvent*>(event));
            return true;
        case QEvent::ChildRemoved:
            m_parent->list_childRemovedEvent(static_cast<QChildEvent*>(event));
            return true;
        case QEvent::MouseButtonPress:
            m_parent->list_mousePressEvent(static_cast<QMouseEvent*>(event));
            return false; // don't eat mouse events
        default:
            break;
        }
        return false;
    }

private:
    ConvertList *m_parent;
};

ConvertList::ListEventFilter::~ListEventFilter() {}

ConvertList::ConvertList(Presets *presets, QWidget *parent) :
    QWidget(parent),
    m_list(new QTreeWidget(this)),
    m_listEventFilter(new ListEventFilter(this)),
    prev_index(0),
    m_converter(new MediaConverter(this)),
    m_probe(new MediaProbe(this)),
    m_current_task(nullptr),
    is_busy(false),
    run_next(false),
    m_presets(presets)
{
    QLayout *layout = new QHBoxLayout(this);
    this->setLayout(layout);

    init_treewidget(m_list);
    layout->addWidget(m_list);

    connect(m_converter, SIGNAL(finished(int))
            , this, SLOT(task_finished_slot(int)));
    connect(m_converter, SIGNAL(progressRefreshed(int)),
            this, SLOT(progress_refreshed(int)));
    connect(m_list, SIGNAL(itemSelectionChanged()),
            this, SIGNAL(itemSelectionChanged()));
    connect(m_list, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(slotDoubleClick(QModelIndex)));

    // Propagate context menu event.
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_list, SIGNAL(customContextMenuRequested(QPoint))
            , this, SIGNAL(customContextMenuRequested(QPoint)));

    // enable drag/drop functions
    m_list->setAcceptDrops(true);

    // allow selecting multiple items
    m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Propagate events from the QTreeWidget to ConvertList.
    m_list->installEventFilter(m_listEventFilter);
    m_list->viewport()->installEventFilter(m_listEventFilter);

    // Enable internal drag-and-drop of list items
    m_list->setDragDropMode(QAbstractItemView::InternalMove);

    QSettings settings;
    QHeaderView *header = m_list->header();

    /* Only restore header states if the column count of the stored header state
       is the same as the current column count. Otherwise, the stored state is
       meaningless and should not be used. */
    int prev_column_count = settings.value("convertlist/column_count").toInt();
    if (prev_column_count == NUM_COLUMNS)
        header->restoreState(settings.value("convertlist/header_state").toByteArray());

    header->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(header, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotHeaderContextMenu(QPoint)));

    m_startTime.start();
    show_background_image();
}

ConvertList::~ConvertList()
{
    QSettings settings;
    /* must store column count along with header state, because saved header
       state is meaningless if the column count changes. */
    settings.setValue("convertlist/header_state", m_list->header()->saveState());
    settings.setValue("convertlist/column_count", NUM_COLUMNS);
}

bool ConvertList::addTask(ConversionParameters param)
{
    // get source file information
    qDebug() << "Probe media file: " << param.source;

    if (!m_probe->run(param.source, TIMEOUT)) {
        if (m_probe->error())
            qDebug() << "Failed to get media information";
        else
            qDebug() << "FFprobe timeout";
        // failed to get media information immediately
        return false;
    }

    /* Ensure unique output filename.
       If the destination filename already exists either on disk
       or in the ConvertList, rename it to prevent overwritting
       completed tasks.  */
    param.destination =
            FilePathOperations::GenerateUniqueFileName(param.destination, output_filenames());
    output_filenames_push(param.destination); // Record the filename for future reference.

    QStringList columns;
    for (int i=0; i<NUM_COLUMNS; i++)
        columns.append(QString());

    fill_list_fields(param, *m_probe, columns);

    QTreeWidgetItem *item = new QTreeWidgetItem(m_list, columns);

    /* Create a new Task object.
     * The ownership of the Task object belongs to m_list.
     * However, the Task object will also be deleted when the
     * corresponding QTreeWidgetItem is deleted. Search for
     * keyword "delete" in this file for the deletion code.
     */
    Task *task = new Task(m_list);
    task->param = param;
    task->status = Task::QUEUED;
    task->id = ++prev_index;
    task->listitem = item;

    QVariant task_var = qVariantFromValue(dynamic_cast<void*>(task));
    item->setData(0, Qt::UserRole, task_var);

    // Prevent dropping directly on an item
    item->setFlags(item->flags() & ~Qt::ItemIsDropEnabled);
    m_list->addTopLevelItem(item);

    progressBar(task)->adjustSize();

    update_tooltip(item);
    hide_background_image();

    qDebug() << QString("Added: \"%1\" -> \"%2\"").arg(param.source).arg(param.destination);

    return true;
}

int ConvertList::addTasks(const QList<ConversionParameters> &paramList)
{
    const int file_count = paramList.size();
    int success_count = 0;

    // Record the files that are not recognized by the converter.
    QStringList failed_files;

    // Create progress dialog.
    /* Translators: *//*: Cancel the operation of adding new tasks */
    QProgressDialog dlgProgress(QString(""),
                                tr("Cancel"),
                                0, file_count,  /* min/max */
                                this);
    dlgProgress.setWindowModality(Qt::WindowModal);
    dlgProgress.setMinimumDuration(MIN_DURATION);

    int progress_count = 0;
    QList<ConversionParameters>::const_iterator it = paramList.begin();
    for (; it!=paramList.end(); ++it) {

        // Indicate the current progress.
        /*: This text is the progress indicator of adding multiple tasks.
            %1 is the number of files that are already added.
            %2 is the total number of files. */
        dlgProgress.setLabelText(tr("Adding files (%1/%2)")
                                 .arg(progress_count).arg(file_count));

        // Update progress dialog.
        dlgProgress.setValue(progress_count++);

        // Check if the user has canceled the operation.
        if (dlgProgress.wasCanceled())
            break;

        if (addTask(*it)) { // This step takes the most of the time.
            success_count++;
        }
        else {
            failed_files.push_back(it->source); // Record failed files.
            qDebug() << QString("Failed to add file: %1").arg(it->source);
        }

    }

    dlgProgress.setValue(file_count); // Terminate the progress indicator.

    if (!failed_files.isEmpty()) { // Some files are incorrect.
        QMessageBox msgBox;
        msgBox.setText(tr("Some files are not recognized by the converter."));
        msgBox.setDetailedText(failed_files.join("\n"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }

    // start conversion if autostart is true
    const bool autostart_default = Constants::getBool("AutoStartConversion");
    bool autostart = QSettings().value("options/auto_start_conversion",
                                       autostart_default).toBool();
    if (autostart && count() > 0)
        start();

    return success_count;
}

bool ConvertList::isBusy() const
{
    return is_busy;
}

bool ConvertList::isEmpty() const
{
    return m_list->topLevelItemCount() == 0;
}

int ConvertList::count() const
{
    return m_list->topLevelItemCount();
}

int ConvertList::selectedCount() const
{
    return m_list->selectedItems().size();
}

int ConvertList::finishedCount() const
{
    return finished_items().size();
}

int ConvertList::elapsedTime() const
{
    return is_busy ? m_startTime.elapsed() : 0;
}

const ConversionParameters* ConvertList::getCurrentIndexParameter() const
{
    Task *task = first_selected_task();
    return task ? &task->param : nullptr;
}

bool ConvertList::selectedTaskFailed() const
{
    Task *task = first_selected_task();
    if (!task || selectedCount() != 1)
        return false;
    return task->status == Task::FAILED;
}

// Public Slots

void ConvertList::start()
{
    if (is_busy && !run_next)
        return;

    run_next = false;

    if (!is_busy) { // new session: start timing
        m_startTime.restart();
        is_busy = true;
        emit started();
    }

    if (!run_first_queued_task()) {
        // no task is executed
        this->stop();
        emit all_tasks_finished();
        emit stopped();
    }
}

void ConvertList::stop()
{
    is_busy = false;
    if (m_current_task) {
        progress_refreshed(0);
        m_current_task->status = Task::QUEUED;
        progressBar(m_current_task)->setActive(false);
        m_current_task = nullptr;
        emit stopped();
    }
    m_converter->stop();
}

void ConvertList::removeSelectedItems()
{
    remove_items(m_list->selectedItems());
}

void ConvertList::removeCompletedItems()
{
    remove_items(finished_items());
}

void ConvertList::editSelectedParameters()
{
    QList<QTreeWidgetItem*> itemList = m_list->selectedItems();

    if (itemList.isEmpty())
        return;

    Task *first_sel_task = get_task(itemList[0]);
    Q_ASSERT(first_sel_task != nullptr);
    ConversionParameters param = first_sel_task->param;
    bool singleItem = (itemList.size() == 1);

    ConversionParameterDialog dialog(this->parentWidget());

    if (dialog.exec(param, singleItem)) {
        foreach (QTreeWidgetItem* item, itemList) {
            Task *task = get_task(item);
            // copy conversion parameters
            // Be sure not to use assignment because it will overwrite the filename.
            task->param.copyConfigurationFrom(param);
            reset_task(task);
        }
    }
}

void ConvertList::changeSelectedOutputFile()
{
    Task *task = first_selected_task();

    if (!task)
        return;

    ConversionParameters &param = task->param;

    QString orig_name = QFileInfo(param.destination).completeBaseName();
    QString dir = QFileInfo(param.destination).path();
    QString ext = QFileInfo(param.destination).suffix();
    QString new_name = orig_name;

    bool try_again = false;
    do {
        new_name = QInputDialog::getText(this, tr("New File Name")
                    , tr("Please input the new name for the output file.")
                    , QLineEdit::Normal, new_name);

        try_again = false;
        if (!new_name.isEmpty() && new_name != orig_name) {
            QString orig_file = param.destination;
            QString file = QDir(dir).absoluteFilePath(new_name + "." + ext);
            QMessageBox::StandardButtons overwrite = QMessageBox::No;
            if (!change_output_file(task, file, overwrite, false))
                try_again = true;
        }
    } while (try_again);
}

void ConvertList::changeSelectedOutputDirectory()
{
    Task *task = first_selected_task();

    if (!task)
        return;

    ConversionParameters &param = task->param;

    QString orig_path = QFileInfo(param.destination).path();

    QString path = QFileDialog::getExistingDirectory(this, tr("Output Directory")
                                     , orig_path);

    if (!path.isEmpty()) {
        // Apply the output path to all selected items
        QMessageBox::StandardButtons overwrite = QMessageBox::No;
        QList<QTreeWidgetItem*> itemList = m_list->selectedItems();
        foreach (QTreeWidgetItem *item, itemList) {
            QString orig_file = task->param.destination;
            QString name = QFileInfo(orig_file).fileName();
            QString file = QDir(path).absoluteFilePath(name);
            change_output_file(get_task(item), file, overwrite, true);
        }
    }
}

void ConvertList::cutSelectedTask()
{
    /* This operation only changes begin time and duration of the parameter,
       so there is no need to refresh the list */
    if (selectedCount() != 1 || !InteractiveCuttingDialog::available())
        return;
    ConversionParameters *param = &first_selected_task()->param;
    InteractiveCuttingDialog(this).exec(param);
}

void ConvertList::retrySelectedItems()
{
    QList<QTreeWidgetItem*> itemList = m_list->selectedItems();

    if (itemList.isEmpty())
        return;

    foreach (QTreeWidgetItem* item, itemList)
        reset_task(get_task(item));

    start();
}

void ConvertList::retryAll()
{
    const int list_size = m_list->topLevelItemCount();
    for (int i=0; i<list_size; i++) {
        QTreeWidgetItem *item = m_list->topLevelItem(i);
        reset_task(get_task(item));
    }

    start();
}

void ConvertList::showErrorMessage()
{
    Task *task = first_selected_task();
    if (task) {
        QMessageBox msgBox;
        msgBox.setText(tr("Error Message from FFmpeg:\n\n") + task->errmsg);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
    }
}

void ConvertList::clear()
{
    const int item_count = count();
    QList<QTreeWidgetItem*> itemList;
    for (int i=0; i<item_count; i++)
        itemList.push_back(m_list->topLevelItem(i));
    remove_items(itemList);
}

// Private Slots
void ConvertList::task_finished_slot(int exitcode)
{
    if (m_current_task) {

        m_current_task->status = (exitcode == 0)
                ? Task::FINISHED
                : Task::FAILED;

        if (exitcode != 0)
            m_current_task->errmsg = m_converter->errorMessage();
        else
            m_current_task->errmsg = "";

        refresh_progressbar(m_current_task);

        m_current_task = nullptr;
        emit task_finished(exitcode);

        run_next = true;
        this->start(); // start next task
    }
}

void ConvertList::progress_refreshed(int percentage)
{
    if (m_current_task) {
        qDebug() << "Progress Refreshed: " << percentage << "%";
        progressBar(m_current_task)->setValue(static_cast<unsigned int> (percentage));
    }
}

void ConvertList::show_background_image()
{
    m_list->viewport()->setStyleSheet(
                "background-image: url(:/other/icons/list_background.png);"
                "background-position: center;"
                "background-repeat: no-repeat;");
    QString tip = tr("Drag and drop files here to add tasks.");
    m_list->viewport()->setStatusTip(tip);
    m_list->viewport()->setToolTip(tip);
}

void ConvertList::hide_background_image()
{
    m_list->viewport()->setStyleSheet("");
    m_list->viewport()->setStatusTip("");
    m_list->viewport()->setToolTip("");
}

void ConvertList::slotHeaderContextMenu(QPoint point)
{
    const int header_count = m_list->header()->count();
    const int current_column = m_list->header()->logicalIndexAt(point);

    // Count visible columns.
    int visible_column_count = 0, visible_column_index = 0;
    for (int i=0; i<header_count; i++) {
        if (!m_list->isColumnHidden(i)) {
            ++visible_column_count;
            visible_column_index = i;
        }
    }

    QMenu menu;

    // Add the item under the mouse to the list
    if (current_column >= 0 && visible_column_count > 1) {
        QAction *action = new QAction(&menu);
        QString column_text = m_list->headerItem()->text(current_column);
        /*: Hide a column in the list. For example, the text maybe 'Hide "Duration"'.
            The two \" are quotation marks in English,
            you may replace it with local quotation marks. */
        QString action_text = tr("Hide \"%1\"").arg(column_text);
        action->setText(action_text);
        action->setData(current_column);
        action->setCheckable(false);
        action->setChecked(false);
        menu.addAction(action);
    }

    QAction *actionRestore = new QAction(&menu);
    actionRestore->setText(tr("Restore All Columns"));
    actionRestore->setData(-1);
    connect(actionRestore, SIGNAL(triggered()),
            this, SLOT(slotRestoreListHeaders()));
    menu.addAction(actionRestore);

    menu.addSeparator();

    // Construct the rest of the menu and uncheck hidden items.
    for (int i=0; i<header_count; i++) {
        QString title = m_list->headerItem()->text(i);
        QAction *action = new QAction(title, &menu);
        action->setCheckable(true);
        action->setChecked(!m_list->isColumnHidden(i));
        action->setData(i); // save the column index

        // not allow user to hide the last column
        if (visible_column_count > 1 || visible_column_index != i)
            menu.addAction(action);
    }

    connect(&menu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotHeaderContextMenuTriggered(QAction*)));

    menu.exec(QCursor::pos());
}

void ConvertList::slotHeaderContextMenuTriggered(QAction *action)
{
    const int column_index = action->data().toInt();
    if (column_index >= 0)
        m_list->setColumnHidden(column_index, !action->isChecked());
}

void ConvertList::slotRestoreListHeaders()
{
    const int column_count = m_list->columnCount();
    QHeaderView *header = m_list->header();
    for (int i=0; i<column_count; i++) { // Restore all sections.
        m_list->showColumn(i);
        header->resizeSection(i, header->defaultSectionSize());
    }

    // Restore default value.
    init_treewidget_columns_visibility(m_list);
}

void ConvertList::slotDoubleClick(QModelIndex index)
{
    int row = index.row();
    if (row >= 0 && row < count()) {
        QTreeWidgetItem *item = m_list->topLevelItem(row);
        Task *task = get_task(item);
        if (task) {
            switch (task->status) {
            case Task::QUEUED:
            case Task::FAILED:
                // Show ConversionParameterDialog
                editSelectedParameters();
                break;
            case Task::FINISHED:
                // Open output folder
                {
                    QString folder_path = QFileInfo(task->param.destination).path();
                    if (QFileInfo(folder_path).exists()) {
                        QDesktopServices::openUrl(QUrl::fromLocalFile(folder_path));
                    }
                }
                break;
            default:
                break;
            }
        }
    }
}

void ConvertList::slotAllItemsRemoved()
{
    show_background_image();
}

// Events

bool ConvertList::list_keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) { // Remove all selected items.
        removeSelectedItems();
        return true; // processed
    } else {
        return false; // not processed
    }
}

void ConvertList::list_dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void ConvertList::list_dragMoveEvent(QDragMoveEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData && mimeData->hasUrls())
        event->acceptProposedAction();
}

void ConvertList::list_dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

// The user drops files into the area.
void ConvertList::list_dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData && mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        AddTaskWizard wizard(m_presets, parentWidget());
        QStringList files;
        Extensions exts;

        // convert urls into local paths
        foreach (QUrl url, urlList) {
            QString file = url.toLocalFile(); // local file name
            files.append(file);
        }

        // show add task wizard
        if (!files.isEmpty() && wizard.exec(files) == QWizard::Accepted) {
            addTasks(wizard.getConversionParameters());
        }
    }
}

void ConvertList::list_childRemovedEvent(QChildEvent */*event*/)
{
    const int task_count = count();
    // refresh all ProgressBar objects
    for (int i=0; i<task_count; i++) {
        refresh_progressbar(get_task(m_list->topLevelItem(i)));
    }
}

void ConvertList::list_mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isEmpty()) {
        // open add task wizard if the list is empty
        AddTaskWizard wizard(m_presets, this);
        if (wizard.exec_openfile() == QDialog::Accepted) {
            addTasks(wizard.getConversionParameters());
        }
    }
}

// Functions to access m_outputFileNames

void ConvertList::output_filenames_push(const QString& filename)
{
    if (m_outputFileNames.contains(filename)) {
        ++m_outputFileNames[filename];
    } else {
        m_outputFileNames.insert(filename, 1);
    }
}

void ConvertList::output_filenames_pop(const QString &filename)
{
    int count = m_outputFileNames.value(filename, 0);
    if (count > 1) {
        --m_outputFileNames[filename];
    } else if (count == 1) {
        m_outputFileNames.remove(filename);
    }
}

QHash<QString, int>& ConvertList::output_filenames()
{
    return m_outputFileNames;
}


// Initialize the QTreeWidget listing files.
void ConvertList::init_treewidget(QTreeWidget *w)
{
    Q_ASSERT_X(w, "ConvertList::init_treewidget", "w: null pointer");

    w->setColumnCount(NUM_COLUMNS);

    QStringList columnTitle;
    for (int i=0; i<NUM_COLUMNS; i++) {
        columnTitle.append(QString());
    }

    // Set column titles.
    init_treewidget_fill_column_titles(columnTitle);

    w->setHeaderLabels(columnTitle);
    //w->header()->setMovable(false); // disable title drag-drop reordering

    w->setRootIsDecorated(false);
    w->setUniformRowHeights(true);

    init_treewidget_columns_visibility(w);
}

/* Fill in the column titles of each field.
   Use the COL_* macros as the index.
   Example: columnTitle[COL_SOURCE] = tr("Source");
*/
void ConvertList::init_treewidget_fill_column_titles(QStringList &columnTitle)
{
    columnTitle[COL_SOURCE] = tr("Source");
    columnTitle[COL_DESTINATION] = tr("Destination");
    columnTitle[COL_DURATION] = tr("Duration");
    columnTitle[COL_FILE_SIZE] = tr("File Size");

    // Audio Information
    columnTitle[COL_AUDIO_SAMPLE_RATE] = /*: Audio */ tr("Sample Rate");
    columnTitle[COL_AUDIO_BITRATE] = tr("Audio Bitrate");
    columnTitle[COL_AUDIO_CHANNELS] = /*: Audio */ tr("Channels");
    columnTitle[COL_AUDIO_CODEC] = tr("Audio Codec");

    // Video Information
    columnTitle[COL_VIDEO_DIMENSIONS] = tr("Dimensions");
    columnTitle[COL_VIDEO_BITRATE] = tr("Video Bitrate");
    columnTitle[COL_VIDEO_FRAMERATE] = /*: Video */ tr("Framerate");
    columnTitle[COL_VIDEO_CODEC] = tr("Video Codec");

    columnTitle[COL_PROGRESS] = tr("Progress");

    // Check if all columns have titles
    // RIP: The asserts need to be for something that break the system
//    for (int i=0; i<NUM_COLUMNS; i++) {
//        qDebug() << "Column" << i << "Title" << columnTitle[i];
//        Q_ASSERT_X(!columnTitle[i].isEmpty(), __FUNCTION__, "every column must have a title");
//    }
}

/* Set the default visibility of each field.
   This configuration will be overriden by user settings.
   For example, to make the duration field invisible by default, write
   w->hideColumn(COL_DURATION, true);
*/
void ConvertList::init_treewidget_columns_visibility(QTreeWidget *w)
{
   // w->hideColumn(COL_FILE_SIZE);
    // Audio Information
    w->hideColumn(COL_AUDIO_SAMPLE_RATE);
    w->hideColumn(COL_AUDIO_BITRATE);
    w->hideColumn(COL_AUDIO_CHANNELS);
    w->hideColumn(COL_AUDIO_CODEC);
    // Video Information
    w->hideColumn(COL_VIDEO_DIMENSIONS);
    w->hideColumn(COL_VIDEO_BITRATE);
    w->hideColumn(COL_VIDEO_FRAMERATE);
    w->hideColumn(COL_VIDEO_CODEC);
    w->hideColumn(COL_VIDEO_SUBTITLE);
}

bool ConvertList::run_first_queued_task()
{
    // execute the first queued task in the list and return
    // returns true if a task is run, false if none
    const int task_count = count();
    for (int i=0; i<task_count; i++) {
        QTreeWidgetItem *item = m_list->topLevelItem(i);
        Task *task = get_task(item);
        if (task->status == Task::QUEUED) {
            QSettings settings;

            // start the task
            is_busy = true;
            task->status = Task::RUNNING;
            m_current_task = task;

            progressBar(task)->setActive(true);

            task->param.threads = settings.value("options/threads", DEFAULT_THREAD_COUNT).toInt();
            qDebug() << "Threads: " + QString::number(task->param.threads);

            m_converter->start(task->param);
            emit start_conversion(i, task->param);

            return true;
        }
    }
    return false;
}

/* Fill in the columns of the list according to the conversion parameter
   and the probing results. QStringList columns will be filled with empty
   items in advanced and the MediaProbe probe will be ready for reading.
   Just write available information to the corresponding fields.
   Use the COL_* macros as the array index.
   Example: columns[COL_SOURCE] = param.source;
*/
void ConvertList::fill_list_fields(ConversionParameters &param, MediaProbe &probe,
                                    QStringList &columns)

{
    columns[COL_SOURCE] = QFileInfo(param.source).fileName(); // source file
    columns[COL_DESTINATION] = QFileInfo(param.destination).fileName(); // destination file
    columns[COL_DURATION] = QString().sprintf("%02d:%02d:%02.0f"   // duration
                  , probe.hours()              //    hours
                  , probe.minutes()            //    minutes
                  , probe.seconds());          //    seconds
    // File Size
    columns[COL_FILE_SIZE] = to_human_readable_size_1024(QFileInfo(param.source).size());

    // Audio Information
    if (probe.hasAudio()) {
        columns[COL_AUDIO_SAMPLE_RATE] = tr("%1 Hz").arg(probe.audioSampleRate());
        columns[COL_AUDIO_BITRATE] = tr("%1 kb/s").arg(probe.audioBitRate());
        columns[COL_AUDIO_CHANNELS] = QString::number(probe.audioChannels());
        columns[COL_AUDIO_CODEC] = probe.audioCodec();
    }

    // Video Information
    if (probe.hasVideo()) {
        columns[COL_VIDEO_DIMENSIONS] = QString("%1x%2")
                .arg(probe.videoWidth()).arg(probe.videoHeight());
        columns[COL_VIDEO_BITRATE] = tr("%1 kb/s").arg(probe.videoBitRate());
        columns[COL_VIDEO_FRAMERATE] = tr("%1 fps").arg(probe.videoFrameRate());
        columns[COL_VIDEO_CODEC] = probe.videoCodec();
    }
}

// Reset the item to the queued state.
void ConvertList::reset_task(Task *task)
{
    if (task && task->status != Task::RUNNING) {
        task->status = Task::QUEUED;
        refresh_progressbar(task);
    }
}

// Remove items in the list.
// A progress dialog is shown if the operation takes time longer than MIN_DURATION.
void ConvertList::remove_items(const QList<QTreeWidgetItem *>& itemList)
{
    /*: Remove files from the tasklist */
    QProgressDialog dlgProgress(tr("Removing tasks..."),
                                tr("Cancel"),
                                0, itemList.count(),
                                this);
    dlgProgress.setWindowModality(Qt::WindowModal);
    dlgProgress.setMinimumDuration(MIN_DURATION);

    int progress_count = 0;
    foreach (QTreeWidgetItem *item, itemList) {
        // Update the progress value.
        dlgProgress.setValue(++progress_count);

        // Check if the user has canceled the operation.
        if (dlgProgress.wasCanceled())
            break;

        remove_item(item);
    }

    dlgProgress.setValue(itemList.size());
}

/**
 * Retrieve the ProgressBar widget associated with the task.
 * This function creates the widget if it doesn't exist.
 * @return Returns the pointer to the ProgressBar widget.
 */
ProgressBar* ConvertList::progressBar(Task *task)
{
    ProgressBar *prog = qobject_cast<ProgressBar *>(m_list->itemWidget(task->listitem, COL_PROGRESS));
    if (!prog) {
        prog = new ProgressBar();
        m_list->setItemWidget(task->listitem, COL_PROGRESS, prog);
    }
    return prog;
}

// Convert bytes to human readable form such as
// "10 KiB" instead of "102400 Bytes".
QString ConvertList::to_human_readable_size_1024(qint64 nBytes)
{
    double num = nBytes;
    QStringList list;
    list << tr("KiB") << tr("MiB") << tr("GiB") << tr("TiB");

    QStringListIterator i(list);
    QString /*: Bytes */ unit(tr("B"));

    while(num >= 1024.0 && i.hasNext()) {
        unit = i.next();
        num /= 1024.0;
    }

    return QString().setNum(num,'f',2)+" "+unit;
}

/** Change the @c destination of the @a task to @a new_file
 *  and update relevant fields in the list.
 *  @warning @a task must not be NULL
 *  @return true if success, false if failed
 */
bool ConvertList::change_output_file(Task *task, const QString &new_file
        , QMessageBox::StandardButtons &overwrite, bool show_all_buttons)
{
    if (overwrite == QMessageBox::NoToAll) return false;

    ConversionParameters &param = task->param;
    QTreeWidgetItem *item = task->listitem;

    QString orig_file = param.destination;

    if (new_file == orig_file) return true; // success: no need to rename

    if ((QFileInfo(new_file).exists() || output_filenames().contains(new_file))
            && overwrite != QMessageBox::YesToAll) {
        QMessageBox::StandardButtons flags = QMessageBox::Yes | QMessageBox::No;
        if (show_all_buttons) {
            flags |= QMessageBox::YesToAll | QMessageBox::NoToAll;
        }
        // The file name already exists.
        // Ask the user whether to force using the file name.
        overwrite = QMessageBox::warning(this, tr("File Exists"),
                          tr("%1 already exists on disk or in the task list. "
                             "Still use this name as the output filename?").arg(new_file)
                          , flags);
        if (overwrite != QMessageBox::Yes && overwrite != QMessageBox::YesToAll)
            return false;
    }

    param.destination = new_file;

    // Rebuild the set of all output filenames in the list.
    output_filenames_pop(orig_file);
    output_filenames_push(new_file);

    // Update item text
    item->setText(COL_DESTINATION, QFileInfo(new_file).fileName());
    update_tooltip(item);

    qDebug() << "Output filename changed: " + orig_file + " => " + new_file;
    return true;
}

/**
 * @brief Remove the @a item along with the associated Task object.
 */
void ConvertList::remove_item(QTreeWidgetItem *item)
{
    Task *task = get_task(item);
    Q_ASSERT(task != nullptr);
    if (task->status != Task::RUNNING) { // not a running task
        output_filenames_pop(task->param.destination);
        const int item_index = m_list->indexOfTopLevelItem(item);
        QTreeWidgetItem *item = m_list->takeTopLevelItem(item_index);
        /* Delete the Task object
         * The ownership of the Task object belongs to m_list.
         * However, it is no longer used when the corresponding
         * list item is deleted, so it's OK to delete it here.
         */
        delete get_task(item);
        delete item;
        qDebug() << "Removed list item " << item_index;
        if (isEmpty()) // removed the last item
            slotAllItemsRemoved();
    } else { // The task is being executed.

        if (/* DISABLES CODE */ (false))  // Silently ignore the event.
            QMessageBox::warning(this, tr("Remove Task")
                              , tr("Cannot remove a task while it is in progress.")
                              , QMessageBox::Ok);
    }
}

/**
 * @brief This function returns the pointer to the first selected task.
 * @retval 0 No item is selected.
 */
Task* ConvertList::first_selected_task() const
{
    QList<QTreeWidgetItem*> itemList = m_list->selectedItems();
    if (itemList.isEmpty())
        return nullptr;
    else
        return get_task(itemList[0]);
}

/**
 * @brief Retrieve the task associated with the tree item
 * @retval 0 The task doesn't exist.
 */
Task* ConvertList::get_task(QTreeWidgetItem *item) const
{
    return static_cast<Task*>(item->data(0, Qt::UserRole).value<void*>());
}

void ConvertList::refresh_progressbar(Task *task)
{
    ProgressBar *prog = progressBar(task);
    switch (task->status) {
    case Task::QUEUED:
        prog->setValue(0);
        prog->setToolTip("");
        prog->setStatusTip("");
        prog->setActive(false);
        break;
    case Task::RUNNING:
        prog->setValue(static_cast<unsigned>(floor(m_converter->progress())));
        prog->setToolTip("");
        prog->setStatusTip("");
        prog->setActive(true);
        break;
    case Task::FINISHED:
        prog->setValue(100);
        /*: The text to be displayed on the progress bar when a conversion finishes */
        prog->showText(tr("Finished"));
        prog->setToolTip(tr("Finished"));
        prog->setStatusTip("");
        prog->setActive(false);
        break;
    case Task::FAILED:
        prog->setValue(0);
        /*: The text to be displayed on the progress bar when a conversion fails */
        prog->showText(tr("Failed"));
        //: %1 is the error message
        prog->setToolTip(tr("Error: %1").arg(task->errmsg));
        prog->setStatusTip(prog->toolTip()); // show error message in statusbar
        prog->setActive(false);
        break;
    }
}


void ConvertList::update_tooltip(QTreeWidgetItem *item)
{
    QStringList tip;

    // List all columns except "progress" in tooltip.

    tip << "<p style='white-space:pre'>"; // prevent automatic linebreak
    int count = 0;
    for (int i=0; i<NUM_COLUMNS; i++) {
        if (i != COL_PROGRESS
                && !m_list->isColumnHidden(i)) {

            if (count++ != 0)
                tip << "<br/>"; // prepend linebreak if necessary

            // show full filename for source and destination
            // otherwise, show the content of the column
            QString content;
            if (i == COL_SOURCE)
                content = get_task(item)->param.source;
            else if (i == COL_DESTINATION)
                content= get_task(item)->param.destination;
            else
               content = item->text(i);

            // show only visible columns
            tip << "<b>"
                   + m_list->headerItem()->text(i) // column title
                   + ":</b> "
                   + htmlEscape(content) // column content
                   ;
        }
    }
    tip << "</p>";

    QString tip_str = tip.join("");

    // set tooltip for every column in the row
    for (int i=0; i<NUM_COLUMNS; i++) {
        if (i != COL_PROGRESS)
            item->setToolTip(i, tip_str);
    }
}

QList<QTreeWidgetItem*> ConvertList::finished_items() const
{
    QList<QTreeWidgetItem*> itemList;
    const int item_count = count();
    for (int i=0; i<item_count; i++) {
        Task *task = get_task(m_list->topLevelItem(i));
        if (task->status == Task::FINISHED) {
            itemList.push_back(task->listitem);
        }
    }
    return itemList;
}
