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

#ifndef CONVERTLIST_H
#define CONVERTLIST_H

#include <QWidget>
#include <QVector>
#include <QElapsedTimer>
#include <QHash>
#include <QMessageBox>
#include <QModelIndex>
#include "converter/conversionparameters.h"

class MediaConverter;
class MediaProbe;
class QTreeWidget;
class QTreeWidgetItem;
class ProgressBar;
class Presets;

class Task;

class ConvertList : public QWidget
{
    Q_OBJECT
public:

    explicit ConvertList(Presets *presets, QWidget *parent = nullptr);
    ~ConvertList();

    /*! Append a task to the list
     * @param param the conversion parameter including the source and destination filename.
     * @return If the function succeeds, it returns true.
     *  Otherwise, it returns false.
     */
    bool addTask(ConversionParameters param);

    /*! Append a list of tasks to the list
     * @param paramList the list of conversion parameters
     * @return the number of successfully added tasks.
     */
    int addTasks(const QList<ConversionParameters>& paramList);

    bool isBusy() const;
    bool isEmpty() const;
    int count() const;
    int selectedCount() const;
    int finishedCount() const;

    /*! Get the elapsed time of the session (in milliseconds).
     *  @retval 0 the converter is idle.
     */
    int elapsedTime() const;

    /*! Returns the pointer to the ConversionParameters object of the
     *  currently selected item.
     *  @return If the function fails, it returns NULL.
     *  @retval NULL the parameter doesn't exist
     */
    const ConversionParameters* getCurrentIndexParameter() const;

    /*! Determine whether the selected task has failed.
     *  @note If multiple tasks are selected, this function always returns false.
     */
    bool selectedTaskFailed() const;

signals:
    void start_conversion(int index, ConversionParameters param);
    void task_finished(int);
    void all_tasks_finished();
    void customContextMenuRequested(const QPoint &pos);
    void itemSelectionChanged();
    void started();
    void stopped();

public slots:

    /*! Start the conversion progress.
     *  If another task is being processed, the function does nothing.
     */
    void start();

    /*! Stop the conversion progress
     */
    void stop();

    /*! Remove all selected tasks but quietly ignore tasks in progress.
     */
    void removeSelectedItems();

    /*! Remove all tasks marked as completed.
     */
    void removeCompletedItems();

    /*! Popup edit-parameter dialog.
     *  The parameter of the first selected task will be used as the default
     *  configuration. If the user presses OK in the dialog, all selected
     *  tasks will be set to the same parameter.
     */
    void editSelectedParameters();

    /*! Popup an input box to change the output filename
     *  of the **first** selected file.
     *  @warning If multiple files are selected, only the first
     *   file will be changed.
     */
    void changeSelectedOutputFile();

    /*! Popup a directory selection dialog to change the output directory.
     */
    void changeSelectedOutputDirectory();

    /*! Select a time range to convert.
     */
    void cutSelectedTask();

    /*! Mark selected items as queued so that they will be converted again.
     *  If the converter is idle, start converting those items.
     *  Otherwise, the items are simply marked as queued.
     */
    void retrySelectedItems();

    void retryAll();

    /*! Popup a message box to show the error message from ffmpeg.
     *  This function only shows the error message for the first selected task.
     */
    void showErrorMessage();

    /*! Remove all tasks but quietly ignore tasks that are in progress.
     */
    void clear();

private slots:
    void task_finished_slot(int);
    void progress_refreshed(int);
    void show_background_image();
    void hide_background_image();
    void slotHeaderContextMenu(QPoint);
    void slotHeaderContextMenuTriggered(QAction*);
    void slotRestoreListHeaders();
    void slotDoubleClick(QModelIndex);
    void slotAllItemsRemoved();

protected:
    bool list_keyPressEvent(QKeyEvent *event);
    void list_dragEnterEvent(QDragEnterEvent *event);
    void list_dragMoveEvent(QDragMoveEvent *event);
    void list_dragLeaveEvent(QDragLeaveEvent *event);
    void list_dropEvent(QDropEvent *event);
    void list_childRemovedEvent(QChildEvent *event);
    void list_mousePressEvent(QMouseEvent *event);

private:
    Q_DISABLE_COPY(ConvertList)

    class ListEventFilter;
    friend class ListEventFilter;

    QTreeWidget *m_list;
    ListEventFilter *m_listEventFilter;
    int prev_index;
    MediaConverter *m_converter;
    MediaProbe *m_probe;
    Task *m_current_task;
    bool is_busy;
    bool run_next; ///< run next task regardless of the value of is_busy
    Presets *m_presets;

    /** this variable should only be accessed by the output_filename_set* functions */
    QHash<QString, int> m_outputFileNames;
    void output_filenames_push(const QString& filename);
    void output_filenames_pop(const QString& filename);
    QHash<QString, int>& output_filenames();

    QElapsedTimer m_startTime;
    void init_treewidget(QTreeWidget*);
    void init_treewidget_fill_column_titles(QStringList&);
    void init_treewidget_columns_visibility(QTreeWidget*);
    bool run_first_queued_task();
    void fill_list_fields(ConversionParameters&, MediaProbe&, QStringList&);
    void reset_task(Task *task);
    void remove_items(const QList<QTreeWidgetItem*>&);
    ProgressBar* progressBar(Task*);
    QString to_human_readable_size_1024(qint64 nBytes);
    bool change_output_file(Task *task, const QString& new_file
            , QMessageBox::StandardButtons &overwrite, bool show_all_buttons);
    void remove_item(QTreeWidgetItem *item);
    Task* first_selected_task() const;
    Task* get_task(QTreeWidgetItem*) const;
    void refresh_progressbar(Task*);
    void update_tooltip(QTreeWidgetItem *item);
    QList<QTreeWidgetItem*> finished_items() const;
};

#endif // CONVERTLIST_H
