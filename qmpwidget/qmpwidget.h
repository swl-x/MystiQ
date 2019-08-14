/*  MystiQ - a C++/Qt5 gui frontend for ffmpeg
 *  Copyright (C) 2011-2019 Maikel Llamaret Heredia <llamaret@webmisolutions.com>
 * Based in qmpwidget - A Qt widget for embedding MPlayer
 *  Copyright (C) 2010 by Jonas Gehring
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


#ifndef QMPWIDGET_H_
#define QMPWIDGET_H_


#include <QHash>
#include <QPointer>
#include <QTimer>
#include <QWidget>

class QAbstractSlider;
class QImage;
class QProcess;
class QStringList;

class QMPProcess;


class QMPwidget : public QWidget
{
	Q_OBJECT
    Q_PROPERTY(State state READ state)
    Q_PROPERTY(double streamPosition READ tell)
    Q_PROPERTY(QString videoOutput READ videoOutput WRITE setVideoOutput)
    Q_PROPERTY(QString mplayerPath READ mplayerPath WRITE setMPlayerPath)
    Q_PROPERTY(QString mplayerVersion READ mplayerVersion)
    Q_ENUMS(state)

	public:
		enum State {
			NotStartedState = -1,
			IdleState,
			LoadingState,
			StoppedState,
			PlayingState,
			BufferingState,
			PausedState,
			ErrorState
		};

		struct MediaInfo {
			QString videoFormat;
			int videoBitrate;
			QSize size;
			double framesPerSecond;

			QString audioFormat;
			double audioBitrate;
			int sampleRate;
			int numChannels;

			QHash<QString, QString> tags;

			bool ok;
			double length;
			bool seekable;

			MediaInfo();
		};

		enum Mode {
			EmbeddedMode = 0,
			PipeMode
		};

		enum SeekMode {
			RelativeSeek = 0,
			PercentageSeek,
			AbsoluteSeek
		};

	public:
        QMPwidget(QWidget *parent = nullptr);
		virtual ~QMPwidget();

		State state() const;
		MediaInfo mediaInfo() const;
		double tell() const;
		QProcess *process() const;

		void setMode(Mode mode);
		Mode mode() const;

		void setVideoOutput(const QString &output);
		QString videoOutput() const;

		void setMPlayerPath(const QString &path);
		QString mplayerPath() const;
		QString mplayerVersion();

		void setSeekSlider(QAbstractSlider *slider);
		void setVolumeSlider(QAbstractSlider *slider);

		void showImage(const QImage &image);

		virtual QSize sizeHint() const;

	public slots:
		void start(const QStringList &args = QStringList());
		void load(const QString &url);
		void play();
		void pause();
		void stop();
		bool seek(int offset, int whence = AbsoluteSeek);
		bool seek(double offset, int whence = AbsoluteSeek);

        void toggleFullScreen();

        void writeCommand(const QString &command);

	protected:
		virtual void mouseDoubleClickEvent(QMouseEvent *event);
		virtual void keyPressEvent(QKeyEvent *event);
		virtual void resizeEvent(QResizeEvent *event);

	private:
		void updateWidgetSize();

	private slots:
        void setVolume(int volume);

		void mpStateChanged(int state);
		void mpStreamPositionChanged(double position);
		void mpVolumeChanged(int volume);
		void delayedSeek();

	signals:
		void stateChanged(int state);
		void error(const QString &reason);

		void readStandardOutput(const QString &line);
		void readStandardError(const QString &line);

	private:
		QMPProcess *m_process;
		QWidget *m_widget;
		QPointer<QAbstractSlider> m_seekSlider;
		QPointer<QAbstractSlider> m_volumeSlider;
		Qt::WindowFlags m_windowFlags;
		QRect m_geometry;

		QTimer m_seekTimer;
		QString m_seekCommand;
};


#endif // QMPWIDGET_H_
