/*
 *  qmpwidget - A Qt widget for embedding MPlayer
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


#include <QAbstractSlider>
#include <QKeyEvent>
#include <QPainter>
#include <QProcess>
#include <QStringList>
#include <QTemporaryFile>
#include <QThread>
#include <QtDebug>

#ifdef QT_OPENGL_LIB
 #include <QGLWidget>
#endif

#include "qmpwidget.h"

//#define QMP_DEBUG_OUTPUT

#ifdef QMP_USE_YUVPIPE
 #include "qmpyuvreader.h"
#endif // QMP_USE_YUVPIPE


// A plain video widget
class QMPPlainVideoWidget : public QWidget
{
	Q_OBJECT

	public:
        QMPPlainVideoWidget(QWidget *parent = nullptr)
			: QWidget(parent)
		{
			setAttribute(Qt::WA_NoSystemBackground);
			setMouseTracking(true);
		}

		void showUserImage(const QImage &image)
		{
			m_userImage = image;
			update();
		}

	public slots:
		void displayImage(const QImage &image)
		{
			m_pixmap = QPixmap::fromImage(image);
			update();
		}

	protected:
		void paintEvent(QPaintEvent *event)
		{
            Q_UNUSED(event)
			QPainter p(this);
			p.setCompositionMode(QPainter::CompositionMode_Source);

			if (!m_userImage.isNull()) {
				p.fillRect(rect(), Qt::black);
				p.drawImage(rect().center() - m_userImage.rect().center(), m_userImage);
			} else if (!m_pixmap.isNull()) {
				p.drawPixmap(rect(), m_pixmap);
			} else {
				p.fillRect(rect(), Qt::black);
			}
			p.end();
		}

	private:
		QPixmap m_pixmap;
		QImage m_userImage;
};


#ifdef QT_OPENGL_LIB

// A OpenGL video widget
class QMPOpenGLVideoWidget : public QGLWidget
{
	Q_OBJECT

	public:
        QMPOpenGLVideoWidget(QWidget *parent = nullptr)
			: QGLWidget(parent), m_tex(-1)
		{
			setMouseTracking(true);
		}

		void showUserImage(const QImage &image)
		{
			m_userImage = image;

			makeCurrent();
			if (m_tex >= 0) {
				deleteTexture(m_tex);
			}
			if (!m_userImage.isNull()) {
				m_tex = bindTexture(image);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			} else {
				glViewport(0, 0, width(), qMax(height(), 1));
			}
			updateGL();
		}

	public slots:
		void displayImage(const QImage &image)
		{
			if (!m_userImage.isNull())  {
				return;
			}

			makeCurrent();
			if (m_tex >= 0) {
				deleteTexture(m_tex);
			}
			m_tex = bindTexture(image);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			updateGL();
		}

	protected:
		void initializeGL()
		{
			glEnable(GL_TEXTURE_2D);
			glClearColor(0, 0, 0, 0);
			glClearDepth(1);
		}

		void resizeGL(int w, int h)
		{
			glViewport(0, 0, w, qMax(h, 1));
		}

		void paintGL()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glLoadIdentity();
			if (m_tex >= 0) {
				glBindTexture(GL_TEXTURE_2D, m_tex);
				if (!m_userImage.isNull()) {
					QRect r = m_userImage.rect();
					r.moveTopLeft(rect().center() - m_userImage.rect().center());
					glViewport(r.x(), r.y(), r.width(), r.height());
				}
				glBegin(GL_QUADS);
				glTexCoord2f(0, 0); glVertex2f(-1, -1);
				glTexCoord2f(1, 0); glVertex2f( 1, -1);
				glTexCoord2f(1, 1); glVertex2f( 1,  1);
				glTexCoord2f(0, 1); glVertex2f(-1,  1);
				glEnd();
			}
		}

	private:
		QImage m_userImage;
		int m_tex;
};

#endif // QT_OPENGL_LIB


// A custom QProcess designed for the MPlayer slave interface
class QMPProcess : public QProcess
{
	Q_OBJECT

	public:
        QMPProcess(QObject *parent = nullptr)
			: QProcess(parent), m_state(QMPwidget::NotStartedState), m_mplayerPath("mplayer"),
              m_fakeInputconf(nullptr)
#ifdef QMP_USE_YUVPIPE
			  , m_yuvReader(NULL)
#endif
		{
			resetValues();

#ifdef Q_WS_WIN
			m_mode = QMPwidget::EmbeddedMode;
			m_videoOutput = "directx,directx:noaccel";
#elif defined(Q_WS_X11)
			m_mode = QMPwidget::EmbeddedMode;
 #ifdef QT_OPENGL_LIB
			m_videoOutput = "gl2,gl,xv";
 #else
			m_videoOutput = "xv";
 #endif
#elif defined(Q_WS_MAC)
			m_mode = QMPwidget::PipeMode;
 #ifdef QT_OPENGL_LIB
			m_videoOutput = "gl,quartz";
 #else
			m_videoOutput = "quartz";
 #endif
#endif

			m_movieFinishedTimer.setSingleShot(true);
			m_movieFinishedTimer.setInterval(100);

			connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(readStdout()));
			connect(this, SIGNAL(readyReadStandardError()), this, SLOT(readStderr()));
			connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished()));
			connect(&m_movieFinishedTimer, SIGNAL(timeout()), this, SLOT(movieFinished()));
		}

		~QMPProcess()
		{
#ifdef QMP_USE_YUVPIPE
			if (m_yuvReader != NULL) {
				m_yuvReader->stop();
			}
#endif
            if (m_fakeInputconf != nullptr) {
				delete m_fakeInputconf;
			}
		}

		// Starts the MPlayer process in idle mode
		void start(QWidget *widget, const QStringList &args)
		{
			if (m_mode == QMPwidget::PipeMode) {
#ifdef QMP_USE_YUVPIPE
				m_yuvReader = new QMPYuvReader(this);
#else
				m_mode = QMPwidget::EmbeddedMode;
#endif
			}

			// Figure out the mplayer version in order to check if 
			// "-input nodefault-bindings" is available
			bool useFakeInputconf = true;
			QString version = mplayerVersion();
			if (version.contains("SVN")) { // Check revision
				QRegExp re("SVN-r([0-9]*)");
				if (re.indexIn(version) > -1) {
					int revision = re.cap(1).toInt();
					if (revision >= 28878) {
						useFakeInputconf = false;
					}
				}
			}

			QStringList myargs;
			myargs += "-slave";
			myargs += "-idle";
			myargs += "-noquiet";
			myargs += "-identify";
			myargs += "-nomouseinput";
			myargs += "-nokeepaspect";
			myargs += "-monitorpixelaspect";
			myargs += "1";
			if (!useFakeInputconf) {
				myargs += "-input";
				myargs += "nodefault-bindings:conf=/dev/null";
			} else {
#ifndef Q_WS_WIN
				// Ugly hack for older versions of mplayer (used in kmplayer and other)
                if (m_fakeInputconf == nullptr) {
					m_fakeInputconf = new QTemporaryFile();
					if (m_fakeInputconf->open()) {
						writeFakeInputconf(m_fakeInputconf);
					} else {
						delete m_fakeInputconf;
                        m_fakeInputconf = nullptr;
					}
				}
                if (m_fakeInputconf != nullptr) {
					myargs += "-input";
					myargs += QString("conf=%1").arg(m_fakeInputconf->fileName());
				}
#endif
			}

			if (m_mode == QMPwidget::EmbeddedMode) {
				myargs += "-wid";
				myargs += QString::number((int)widget->winId());
				if (!m_videoOutput.isEmpty()) {
					myargs += "-vo";
					myargs += m_videoOutput;
				}
			} else {
#ifdef QMP_USE_YUVPIPE
				myargs += "-vo";
				myargs += QString("yuv4mpeg:file=%1").arg(m_yuvReader->m_pipe);
#endif
			}

			myargs += args;
#ifdef QMP_DEBUG_OUTPUT
			qDebug() << myargs;
#endif
			QProcess::start(m_mplayerPath, myargs);
			changeState(QMPwidget::IdleState);

			if (m_mode == QMPwidget::PipeMode) {
#ifdef QMP_USE_YUVPIPE
				connect(m_yuvReader, SIGNAL(imageReady(const QImage &)), widget, SLOT(displayImage(const QImage &)));
				m_yuvReader->start();
#endif
			}
		}

		QString mplayerVersion()
		{
			QProcess p;
			p.start(m_mplayerPath, QStringList("-version"));
			if (!p.waitForStarted()) {
				return QString();
			}
			if (!p.waitForFinished()) {
				return QString();
			}

			QString output = QString(p.readAll());
			QRegExp re("MPlayer ([^ ]*)");
			if (re.indexIn(output) > -1) {
				return re.cap(1);
			}
			return output;
		}

		QProcess::ProcessState processState() const
		{
			return QProcess::state();
		}

		void writeCommand(const QString &command)
		{
#ifdef QMP_DEBUG_OUTPUT
			qDebug("in: \"%s\"", qPrintable(command));
#endif
			QProcess::write(command.toLocal8Bit()+"\n");
		}

		void quit()
		{
			writeCommand("quit");
			QProcess::waitForFinished(100);
			if (QProcess::state() == QProcess::Running) {
				QProcess::kill();
			}
			QProcess::waitForFinished(-1);
		}

		void pause()
		{
			writeCommand("pause");
		}

		void stop()
		{
			writeCommand("stop");
		}

	signals:
		void stateChanged(int state);
		void streamPositionChanged(double position);
		void error(const QString &reason);

		void readStandardOutput(const QString &line);
		void readStandardError(const QString &line);

	private slots:
		void readStdout()
		{
			QStringList lines = QString::fromLocal8Bit(readAllStandardOutput()).split("\n", QString::SkipEmptyParts);
			for (int i = 0; i < lines.count(); i++) {
				lines[i].remove("\r");
#ifdef QMP_DEBUG_OUTPUT
				qDebug("out: \"%s\"", qPrintable(lines[i]));
#endif
				parseLine(lines[i]);
				emit readStandardOutput(lines[i]);
			}
		}

		void readStderr()
		{
			QStringList lines = QString::fromLocal8Bit(readAllStandardError()).split("\n", QString::SkipEmptyParts);
			for (int i = 0; i < lines.count(); i++) {
				lines[i].remove("\r");
#ifdef QMP_DEBUG_OUTPUT
				qDebug("err: \"%s\"", qPrintable(lines[i]));
#endif
				parseLine(lines[i]);
				emit readStandardError(lines[i]);
			}
		}

		void finished()
		{
			// Called if the *process* has finished
			changeState(QMPwidget::NotStartedState);
		}

		void movieFinished()
		{
			if (m_state == QMPwidget::PlayingState) {
				changeState(QMPwidget::IdleState);
			}
		}

	private:
		// Parses a line of MPlayer output
		void parseLine(const QString &line)
		{
			if (line.startsWith("Playing ")) {
				changeState(QMPwidget::LoadingState);
			} else if (line.startsWith("Cache fill:")) {
				changeState(QMPwidget::BufferingState);
			} else if (line.startsWith("Starting playback...")) {
				m_mediaInfo.ok = true; // No more info here
				changeState(QMPwidget::PlayingState);
			} else if (line.startsWith("File not found: ")) {
				changeState(QMPwidget::ErrorState);
			} else if (line.endsWith("ID_PAUSED")) {
				changeState(QMPwidget::PausedState);
			} else if (line.startsWith("ID_")) {
				parseMediaInfo(line);
			} else if (line.startsWith("No stream found")) {
				changeState(QMPwidget::ErrorState, line);
			} else if (line.startsWith("A:") || line.startsWith("V:")) {
				if (m_state != QMPwidget::PlayingState) {
					changeState(QMPwidget::PlayingState);
				}
				parsePosition(line);
			} else if (line.startsWith("Exiting...")) {
				changeState(QMPwidget::NotStartedState);
			}
		}

		// Parses MPlayer's media identification output
		void parseMediaInfo(const QString &line)
		{
			QStringList info = line.split("=");
			if (info.count() < 2) {
				return;
			}

			if (info[0] == "ID_VIDEO_FORMAT") {
				m_mediaInfo.videoFormat = info[1];
			} else if (info[0] == "ID_VIDEO_BITRATE") {
				m_mediaInfo.videoBitrate = info[1].toInt();
			} else if (info[0] == "ID_VIDEO_WIDTH") {
				m_mediaInfo.size.setWidth(info[1].toInt());
			} else if (info[0] == "ID_VIDEO_HEIGHT") {
				m_mediaInfo.size.setHeight(info[1].toInt());
			} else if (info[0] == "ID_VIDEO_FPS") {
				m_mediaInfo.framesPerSecond = info[1].toDouble();

			} else if (info[0] == "ID_AUDIO_FORMAT") {
				m_mediaInfo.audioFormat = info[1];
			} else if (info[0] == "ID_AUDIO_BITRATE") {
				m_mediaInfo.audioBitrate = info[1].toInt();
			} else if (info[0] == "ID_AUDIO_RATE") {
				m_mediaInfo.sampleRate = info[1].toInt();
			} else if (info[0] == "ID_AUDIO_NCH") {
				m_mediaInfo.numChannels = info[1].toInt();

			} else if (info[0] == "ID_LENGTH") {
				m_mediaInfo.length = info[1].toDouble();
			} else if (info[0] == "ID_SEEKABLE") {
                m_mediaInfo.seekable = static_cast<bool>(info[1].toInt());

			} else if (info[0].startsWith("ID_CLIP_INFO_NAME")) {
				m_currentTag = info[1];
			} else if (info[0].startsWith("ID_CLIP_INFO_VALUE") && !m_currentTag.isEmpty()) {
				m_mediaInfo.tags.insert(m_currentTag, info[1]);
			}
		}

		// Parsas MPlayer's position output
		void parsePosition(const QString &line)
		{
			static QRegExp rx("[ :]");
			QStringList info = line.split(rx, QString::SkipEmptyParts);

			double oldpos = m_streamPosition;
			for (int i = 0; i < info.count(); i++) {
				if ( (info[i] == "V" || info[i] == "A") && info.count() > i) {
					m_streamPosition = info[i+1].toDouble();

					// If the movie is near its end, start a timer that will check whether
					// the movie has really finished.
					if (qAbs(m_streamPosition - m_mediaInfo.length) < 1) {
						m_movieFinishedTimer.start();
					}
				}
			}

			if (oldpos != m_streamPosition) {
				emit streamPositionChanged(m_streamPosition);
			}
		}

		// Changes the current state, possibly emitting multiple signals
		void changeState(QMPwidget::State state, const QString &comment = QString())
		{
#ifdef QMP_USE_YUVPIPE
			if (m_yuvReader != NULL && (state == QMPwidget::ErrorState || state == QMPwidget::NotStartedState)) {
				m_yuvReader->stop();
				m_yuvReader->deleteLater();
			}
#endif

			if (m_state == state) {
				return;
			}

			if (m_state == QMPwidget::PlayingState) {
				m_movieFinishedTimer.stop();
			}

			m_state = state;
			emit stateChanged(m_state);

			switch (m_state) {
				case QMPwidget::NotStartedState:
					resetValues();
					break;

				case QMPwidget::ErrorState:
					emit error(comment);
					resetValues();
					break;

				default: break;
			}
		}

		// Resets the media info and position values
		void resetValues()
		{
			m_mediaInfo = QMPwidget::MediaInfo();
			m_streamPosition = -1;
		}

		// Writes a dummy input configuration to the given device
		void writeFakeInputconf(QIODevice *device)
		{
			// Query list of supported keys
			QProcess p;
			p.start(m_mplayerPath, QStringList("-input") += "keylist");
			if (!p.waitForStarted()) {
				return;
			}
			if (!p.waitForFinished()) {
				return;
			}
			QStringList keys = QString(p.readAll()).split("\n", QString::SkipEmptyParts);

			// Write dummy command for each key
			QTextStream out(device);
			for (int i = 0; i < keys.count(); i++) {
				keys[i].remove("\r");
				out << keys[i] << " " << "ignored" << endl;
			}
		}

	public:
		QMPwidget::State m_state;

		QString m_mplayerPath;
		QString m_videoOutput;
		QString m_pipe;
		QMPwidget::Mode m_mode;

		QMPwidget::MediaInfo m_mediaInfo;
		double m_streamPosition; // This is the video position
		QTimer m_movieFinishedTimer;

		QString m_currentTag;

		QTemporaryFile *m_fakeInputconf;

#ifdef QMP_USE_YUVPIPE
		QPointer<QMPYuvReader> m_yuvReader;
#endif
};


// Initialize the media info structure
QMPwidget::MediaInfo::MediaInfo()
	: videoBitrate(0), framesPerSecond(0), sampleRate(0), numChannels(0),
	  ok(false), length(0), seekable(false)
{

}


/*!
 * \brief Constructor
 *
 * \param parent Parent widget
 */
QMPwidget::QMPwidget(QWidget *parent)
	: QWidget(parent)
{
	setFocusPolicy(Qt::StrongFocus);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

#ifdef QT_OPENGL_LIB
	m_widget = new QMPOpenGLVideoWidget(this);
#else
	m_widget = new QMPPlainVideoWidget(this);
#endif

	QPalette p = palette();
	p.setColor(QPalette::Window, Qt::black);
	setPalette(p);

	m_seekTimer.setInterval(50);
	m_seekTimer.setSingleShot(true);
	connect(&m_seekTimer, SIGNAL(timeout()), this, SLOT(delayedSeek()));

	m_process = new QMPProcess(this);
	connect(m_process, SIGNAL(stateChanged(int)), this, SLOT(mpStateChanged(int)));
	connect(m_process, SIGNAL(streamPositionChanged(double)), this, SLOT(mpStreamPositionChanged(double)));
	connect(m_process, SIGNAL(error(const QString &)), this, SIGNAL(error(const QString &)));
	connect(m_process, SIGNAL(readStandardOutput(const QString &)), this, SIGNAL(readStandardOutput(const QString &)));
	connect(m_process, SIGNAL(readStandardError(const QString &)), this, SIGNAL(readStandardError(const QString &)));
}

/*!
 * \brief Destructor
 * \details
 * This function will ask the MPlayer process to quit and block until it has really
 * finished.
 */
QMPwidget::~QMPwidget()
{
	if (m_process->processState() == QProcess::Running) {
		m_process->quit();
	}
	delete m_process;
}

/*!
 * \brief Returns the current MPlayer process state
 *
 * \returns The process state
 */
QMPwidget::State QMPwidget::state() const
{
	return m_process->m_state;
}

/*!
 * \brief Returns the current media info object 
 * \details
 * Please check QMPwidget::MediaInfo::ok to make sure the media
 * information has been fully parsed.
 *
 * \returns The media info object
 */
QMPwidget::MediaInfo QMPwidget::mediaInfo() const
{
	return m_process->m_mediaInfo;
}

/*!
 * \brief Returns the current playback position
 *
 * \returns The current playback position in seconds
 * \sa seek()
 */
double QMPwidget::tell() const
{
	return m_process->m_streamPosition;
}

/*!
 * \brief Returns the MPlayer process
 *
 * \returns The MPlayer process
 */
QProcess *QMPwidget::process() const
{
	return m_process;
}

/*!
 * \brief Sets the video playback mode
 * \details
 * Please see \ref playbackmodes for a discussion of the available modes.
 *
 * \param mode The video playback mode
 * \sa mode()
 */
void QMPwidget::setMode(Mode mode)
{
#ifdef QMP_USE_YUVPIPE
	m_process->m_mode = mode;
#else
	Q_UNUSED(mode)
#endif
}

/*!
 * \brief Returns the current video playback mode
 *
 * \returns The current video playback mode
 * \sa setMode()
 */
QMPwidget::Mode QMPwidget::mode() const
{
	return m_process->m_mode;
}

/*!
 * \brief Sets the video output mode
 * \details
 * The video output mode string will be passed to MPlayer using its \p -vo option.
 * Please see http://www.mplayerhq.hu/DOCS/HTML/en/video.html for an overview of
 * available video output modes.
 *
 * Per default, this string will have the following values:
 * <table>
 *  <tr><th>System</th><th>Configuration</th><th>Value</th></tr>
 *  <tr>
 *   <td>Windows</td>
 *   <td></td>
 *   <td>\p "directx,directx:noaccel"</td>
 *  </tr>
 *  <tr>
 *   <td>X11</td>
 *   <td>Compiled without OpenGL support</td>
 *   <td>\p "xv"</td>
 *  </tr>
 *  <tr>
 *   <td>X11</td>
 *   <td>Compiled with OpenGL support</td>
 *   <td>\p "gl2,gl,xv"</td>
 *  </tr>
 *  <tr>
 *   <td>Mac OS X</td>
 *   <td>Compiled without OpenGL support</td>
 *   <td>\p "quartz"</td>
 *  </tr>
 *  <tr>
 *   <td>Mac OS X</td>
 *   <td>Compiled with OpenGL support</td>
 *   <td>\p "gl,quartz"</td>
 *  </tr>
 * </table>
 *
 *
 * \param output The video output mode string
 * \sa videoOutput()
 */
void QMPwidget::setVideoOutput(const QString &output)
{
	m_process->m_videoOutput = output;
}

/*!
 * \brief Returns the current video output mode
 *
 * \returns The current video output mode
 * \sa setVideoOutput()
 */
QString QMPwidget::videoOutput() const
{
	return m_process->m_videoOutput;
}

/*!
 * \brief Sets the path to the MPlayer executable
 * \details
 * Per default, it is assumed the MPlayer executable is
 * available in the current OS path. Therefore, this value is
 * set to "mplayer".
 *
 * \param path Path to the MPlayer executable
 * \sa mplayerPath()
 */
void QMPwidget::setMPlayerPath(const QString &path)
{
	m_process->m_mplayerPath = path;
}

/*!
 * \brief Returns the current path to the MPlayer executable
 *
 * \returns The path to the MPlayer executable
 * \sa setMPlayerPath()
 */
QString QMPwidget::mplayerPath() const
{
	return m_process->m_mplayerPath;
}

/*!
 * \brief Returns the version string of the MPlayer executable
 * \details
 * If the mplayer  
 *
 *
 * \returns The version string of the MPlayer executable
 */
QString QMPwidget::mplayerVersion()
{
	return m_process->mplayerVersion();
}

/*!
 * \brief Sets a seeking slider for this widget
 */
void QMPwidget::setSeekSlider(QAbstractSlider *slider)
{
	if (m_seekSlider) {
		m_seekSlider->disconnect(this);
		disconnect(m_seekSlider);
	}

	if (m_process->m_mediaInfo.ok) {
		slider->setRange(0, m_process->m_mediaInfo.length);
	}
	if (m_process->m_mediaInfo.ok) {
		slider->setEnabled(m_process->m_mediaInfo.seekable);
	}

	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(seek(int)));
	m_seekSlider = slider;
}

/*!
 * \brief Sets a volume slider for this widget
 */
void QMPwidget::setVolumeSlider(QAbstractSlider *slider)
{
	if (m_volumeSlider) {
		m_volumeSlider->disconnect(this);
		disconnect(m_volumeSlider);
	}


	slider->setRange(0, 100);
	slider->setValue(100); // TODO

	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setVolume(int)));
	m_volumeSlider = slider;
}

/*!
 * \brief Shows a custom image
 * \details
 * This function sets a custom image that will be shown instead of the MPlayer
 * video output. In order to show MPlayer's output again, call this function
 * with a null image.
 *
 * \note If the current playback mode is not set to \p PipeMode, this function
 * will have no effect if MPlayer draws to the widget.
 *
 * \param image Custom image
 */
void QMPwidget::showImage(const QImage &image)
{
#ifdef QT_OPENGL_LIB
	qobject_cast<QMPOpenGLVideoWidget *>(m_widget)->showUserImage(image);
#else
	qobject_cast<QMPPlainVideoWidget*>(m_widget)->showUserImage(image);
#endif
}

/*!
 * \brief Returns a suitable size hint for this widget
 * \details
 * This function is used internally by Qt.
 */
QSize QMPwidget::sizeHint() const
{
	if (m_process->m_mediaInfo.ok && !m_process->m_mediaInfo.size.isNull()) {
		return m_process->m_mediaInfo.size;
	}
	return QWidget::sizeHint();
}

/*!
 * \brief Starts the MPlayer process with the given arguments
 * \details
 * If there's another process running, it will be terminated first. MPlayer
 * will be run in idle mode and is avaiting your commands, e.g. via load().
 *
 * \param args MPlayer command line arguments
 */
void QMPwidget::start(const QStringList &args)
{
	if (m_process->processState() == QProcess::Running) {
		m_process->quit();
	}
	m_process->start(m_widget, args);
}

/*!
 * \brief Loads a file or url and starts playback
 *
 * \param url File patho or url
 */
void QMPwidget::load(const QString &url)
{
	Q_ASSERT_X(m_process->state() != QProcess::NotRunning, "QMPwidget::load()", "MPlayer process not started yet");

	// From the MPlayer slave interface documentation:
	// "Try using something like [the following] to switch to the next file.
	// It avoids audio playback starting to play the old file for a short time
	// before switching to the new one.
	writeCommand("pausing_keep_force pt_step 1");
	writeCommand("get_property pause");

	QString url_escaped = QString(url).replace("'", "\\'");

	writeCommand(QString("loadfile '%1'").arg(url_escaped));
}

/*!
 * \brief Resumes playback
 */
void QMPwidget::play()
{
	if (m_process->m_state == PausedState) {
		m_process->pause();
	}
}

/*!
 * \brief Pauses playback
 */
void QMPwidget::pause()
{
	if (m_process->m_state == PlayingState) {
		m_process->pause();
	}
}

/*!
 * \brief Stops playback
 */
void QMPwidget::stop()
{
	m_process->stop();
}

/*!
 * \brief Media playback seeking
 *
 * \param offset Seeking offset in seconds
 * \param whence Seeking mode
 * \returns \p true If the seeking mode is valid
 * \sa tell()
 */
bool QMPwidget::seek(int offset, int whence)
{
	return seek(double(offset), whence);
}

/*!
 * \brief Media playback seeking
 *
 * \param offset Seeking offset in seconds
 * \param whence Seeking mode
 * \returns \p true If the seeking mode is valid
 * \sa tell()
 */
bool QMPwidget::seek(double offset, int whence)
{
	m_seekTimer.stop(); // Cancel all current seek requests

	switch (whence) {
		case RelativeSeek:
		case PercentageSeek:
		case AbsoluteSeek:
			break;
		default:
			return false;
	}

	// Schedule seek request
	m_seekCommand = QString("seek %1 %2").arg(offset).arg(whence);
	m_seekTimer.start();
	return true;
}

/*!
 * \brief Toggles full-screen mode
 */
void QMPwidget::toggleFullScreen()
{
	if (!isFullScreen()) {
		m_windowFlags = windowFlags() & (Qt::Window);
		m_geometry = geometry();
		setWindowFlags((windowFlags() | Qt::Window));
		// From Phonon::VideoWidget
#ifdef Q_WS_X11
		show();
		raise();
		setWindowState(windowState() | Qt::WindowFullScreen);
#else
		setWindowState(windowState() | Qt::WindowFullScreen);
		show();
#endif
	} else {
		setWindowFlags((windowFlags() ^ (Qt::Window)) | m_windowFlags);
		setWindowState(windowState() & ~Qt::WindowFullScreen);
		setGeometry(m_geometry);
		show();
	}
}

/*!
 * \brief Sends a command to the MPlayer process
 * \details
 * Since MPlayer is being run in slave mode, it reads commands from the standard
 * input. It is assumed that the interface provided by this class might not be
 * sufficient for some situations, so you can use this functions to directly
 * control the MPlayer process.
 *
 * For a complete list of commands for MPlayer's slave mode, see
 * http://www.mplayerhq.hu/DOCS/tech/slave.txt .
 *
 * \param command The command line. A newline character will be added internally.
 */
void QMPwidget::writeCommand(const QString &command)
{
	m_process->writeCommand(command);
}

/*!
 * \brief Mouse double click event handler
 * \details
 * This implementation will toggle full screen and accept the event
 *
 * \param event Mouse event
 */
void QMPwidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	toggleFullScreen();
	event->accept();
}

/*!
 * \brief Keyboard press event handler
 * \details
 * This implementation tries to resemble the classic MPlayer interface. For a
 * full list of supported key codes, see \ref shortcuts.
 *
 * \param event Key event
 */
void QMPwidget::keyPressEvent(QKeyEvent *event)
{
	bool accept = true;
	switch (event->key()) {
		case Qt::Key_P:
		case Qt::Key_Space:
			if (state() == PlayingState) {
				pause();
			} else if (state() == PausedState) {
				play();
			}
			break;

		case Qt::Key_F:
			toggleFullScreen();
			break;

		case Qt::Key_Q:
		case Qt::Key_Escape:
			stop();
			break;

		case Qt::Key_Minus:
			writeCommand("audio_delay -0.1");
			break;
		case Qt::Key_Plus:
			writeCommand("audio_delay 0.1");
			break;

		case Qt::Key_Left:
			seek(-10, RelativeSeek);
			break;
		case Qt::Key_Right:
			seek(10, RelativeSeek);
			break;
		case Qt::Key_Down:
			seek(-60, RelativeSeek);
			break;
		case Qt::Key_Up:
			seek(60, RelativeSeek);
			break;
		case Qt::Key_PageDown:
			seek(-600, RelativeSeek);
			break;
		case Qt::Key_PageUp:
			seek(600, RelativeSeek);
			break;

		case Qt::Key_Asterisk:
			writeCommand("volume 10");
			break;
		case Qt::Key_Slash:
			writeCommand("volume -10");
			break;

		case Qt::Key_X:
			writeCommand("sub_delay 0.1");
			break;
		case Qt::Key_Z:
			writeCommand("sub_delay -0.1");
			break;

		default:
			accept = false;
			break;
	}

	event->setAccepted(accept);
}

/*!
 * \brief Resize event handler
 * \details
 * If you reimplement this function, you need to call this handler, too.
 *
 * \param event Resize event
 */
void QMPwidget::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);
	updateWidgetSize();
}

void QMPwidget::updateWidgetSize()
{
	if (!m_process->m_mediaInfo.size.isNull()) {
		QSize mediaSize = m_process->m_mediaInfo.size;
		QSize widgetSize = size();

		double factor = qMin(double(widgetSize.width()) / mediaSize.width(), double(widgetSize.height()) / mediaSize.height());
		QRect wrect(0, 0, int(factor * mediaSize.width() + 0.5), int(factor * mediaSize.height()));
		wrect.moveTopLeft(rect().center() - wrect.center());
		m_widget->setGeometry(wrect);
	} else {
		m_widget->setGeometry(QRect(QPoint(0, 0), size()));
	}
}

void QMPwidget::delayedSeek()
{
	if (!m_seekCommand.isEmpty()) {
		writeCommand(m_seekCommand);
		m_seekCommand = QString();
	}
}

void QMPwidget::setVolume(int volume)
{
	writeCommand(QString("volume %1 1").arg(volume));
}

void QMPwidget::mpStateChanged(int state)
{
	if (m_seekSlider != NULL && state == PlayingState && m_process->m_mediaInfo.ok) {
		m_seekSlider->setRange(0, m_process->m_mediaInfo.length);
		m_seekSlider->setEnabled(m_process->m_mediaInfo.seekable);
	}

	updateWidgetSize();
	emit stateChanged(state);
}

void QMPwidget::mpStreamPositionChanged(double position)
{
	if (m_seekSlider != NULL && m_seekCommand.isEmpty() && m_seekSlider->value() != qRound(position)) {
		m_seekSlider->disconnect(this);
		m_seekSlider->setValue(qRound(position));
		connect(m_seekSlider, SIGNAL(valueChanged(int)), this, SLOT(seek(int)));
	}
}

void QMPwidget::mpVolumeChanged(int volume)
{
	if (m_volumeSlider != NULL) {
		m_volumeSlider->disconnect(this);
		m_volumeSlider->setValue(volume);
		connect(m_seekSlider, SIGNAL(valueChanged(int)), this, SLOT(setVolume(int)));
	}
}


#include "qmpwidget.moc"


/* Documentation follows */

/*!
 * \class QMPwidget
 * \brief A Qt widget for embedding MPlayer
 * \details
 *
 * \section Overview
 *
 * \subsection comm MPlayer communication
 *
 * If you want to communicate with MPlayer through its 
 * <a href="http://www.mplayerhq.hu/DOCS/tech/slave.txt">slave mode protocol</a>,
 * you can use the writeCommand() slot. If MPlayer writes to its standard output
 * or standard error channel, the signals readStandardOutput() and
 * readStandardError() will be emitted.
 *
 * \subsection controls Graphical controls
 *
 * You can connect sliders for seeking and volume adjustment to an instance of
 * this class. Please use setSeekSlider() and setVolumeSlider(), respectively.
 *
 * \section example Usage example
 *
 * A minimal example using this widget to play a low-res version of
 * <a href="http://www.bigbuckbunny.org/">Big Buck Bunny</a> might look as follows.
 * Please note that the actual movie URL has been shortened for the sake of clarity.
\code
#include <QApplication>
#include "qmpwidget.h"

// Program entry point
int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	QMPwidget widget;
	widget.show();
	widget.start(QStringList("http://tinyurl.com/2vs2kg5"));

	return app.exec();
}
\endcode
 *
 *
 * For further information about this project, please refer to the
 * <a href="index.html">main page</a>.
 */

/*!
 * \enum QMPwidget::State
 * \brief MPlayer state
 * \details
 * This enumeration is somewhat identical to <a href="http://doc.trolltech.com/phonon.html#State-enum">
 * Phonon's State enumeration</a>, except that it has an additional
 * member which is used when the MPlayer process has not been started yet (NotStartedState)
 *
 * <table>
 *  <tr><th>Constant</th><th>Value</th><th>Description</th></tr>
 *  <tr>
 *   <td>\p QMPwidget::NotStartedState</td>
 *   <td>\p -1</td>
 *   <td>The Mplayer process has not been started yet or has already terminated.</td>
 *  </tr>
 *  <tr>
 *   <td>\p QMPwidget::IdleState</td>
 *   <td>\p 0</td>
 *   <td>The MPlayer process has been started, but is idle and waiting for commands.</td>
 *  </tr>
 *  <tr>
 *   <td>\p QMPwidget::LoadingState</td>
 *   <td>\p 1</td>
 *   <td>The media file is being loaded, but playback has not been started yet.</td>
 *  </tr>
 *  <tr>
 *   <td>\p QMPwidget::StoppedState</td>
 *   <td>\p 2</td>
 *   <td>This constant is deprecated and is not being used</td>
 *  </tr>
 *  <tr>
 *   <td>\p QMPwidget::PlayingState</td>
 *   <td>\p 3</td>
 *   <td></td>
 *  </tr>
 *  <tr>
 *   <td>\p QMPwidget::BufferingState</td>
 *   <td>\p 4</td>
 *   <td></td>
 *  </tr>
 *  <tr>
 *   <td>\p QMPwidget::PausedState</td>
 *   <td>\p 5</td>
 *   <td></td>
 *  </tr>
 *  <tr>
 *   <td>\p QMPwidget::ErrorState</td>
 *   <td>\p 6</td>
 *   <td></td>
 *  </tr>
 * </table>
 */

/*!
 * \enum QMPwidget::Mode
 * \brief Video playback modes
 * \details
 * This enumeration describes valid modes for video playback. Please see \ref playbackmodes for a
 * detailed description of both modes.
 *
 * <table>
 *  <tr><th>Constant</th><th>Value</th><th>Description</th></tr>
 *  <tr>
 *   <td>\p QMPwidget::EmbeddedMode</td>
 *   <td>\p 0</td>
 *   <td>MPlayer will render directly into a Qt widget.</td>
 *  </tr>
 *  <tr>
 *   <td>\p QMPwidget::PipedMode</td>
 *   <td>\p 1</td>
 *   <td>MPlayer will write the video data into a FIFO which will be parsed in a seperate thread.\n
  The frames will be rendered by QMPwidget.</td>
 *  </tr>
 * </table>
 */

/*!
 * \enum QMPwidget::SeekMode
 * \brief Seeking modes
 * \details
 * This enumeration describes valid modes for seeking the media stream.
 *
 * <table>
 *  <tr><th>Constant</th><th>Value</th><th>Description</th></tr>
 *  <tr>
 *   <td>\p QMPwidget::RelativeSeek</td>
 *   <td>\p 0</td>
 *   <td>Relative seek in seconds</td>
 *  </tr>
 *  <tr>
 *   <td>\p QMPwidget::PercantageSeek</td>
 *   <td>\p 1</td>
 *   <td>Seek to a position given by a percentage of the whole movie duration</td>
 *  </tr>
 *  <tr>
 *   <td>\p QMPwidget::AbsoluteSeek</td>
 *   <td>\p 2</td>
 *   <td>Seek to a position given by an absolute time</td>
 *  </tr>
 * </table>
 */

/*!
 * \fn void QMPwidget::stateChanged(int state)
 * \brief Emitted if the state has changed
 * \details
 * This signal is emitted when the state of the MPlayer process changes.
 *
 * \param state The new state
 */

/*!
 * \fn void QMPwidget::error(const QString &reason)
 * \brief Emitted if the state has changed to QMPwidget::ErrorState
 * \details
 * This signal is emitted when the state of the MPlayer process changes to QMPwidget::ErrorState.
 *
 * \param reason Textual error description (may be empty)
 */

/*!
 * \fn void QMPwidget::readStandardOutput(const QString &line)
 * \brief Signal for reading MPlayer's standard output
 * \details
 * This signal is emitted when MPlayer wrote a line of text to its standard output channel.
 */

/*!
 * \fn void QMPwidget::readStandardError(const QString &line)
 * \brief Signal for reading MPlayer's standard error
 * \details
 * This signal is emitted when MPlayer wrote a line of text to its standard error channel.
 */
