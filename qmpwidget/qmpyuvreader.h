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


#include <QImage>
#include <QDir>
#include <QMutex>
#include <QThread>

#ifdef Q_WS_WIN
 #include "windows.h"
#endif

#include <cstdio>
#include <sys/stat.h>


// Internal YUV pipe reader
class QMPYuvReader : public QThread
{
	Q_OBJECT

	public:
		// Constructor
        QMPYuvReader(QObject *parent = nullptr)
            : QThread(parent), m_stop(false), m_saveme(nullptr), m_savemeSize(-1)
		{
			QString tdir = QDir::tempPath();

			// Create pipe in a temporary directory
			char *temp = new char[tdir.length() + 12];
			strcpy(temp, tdir.toLocal8Bit().data());
			strcat(temp, "/XXXXXX");
            if (mkdtemp(temp) == nullptr) {
				qWarning("Can't create temporary directory");
				return;
			}
			strcat(temp, "/fifo");
			if (mkfifo(temp, 0600) != 0) {
				qWarning("Can't create pipe");
				return;
			}
			m_pipe = QString(temp);
			delete[] temp;

			initTables();
		}

		// Destructor
		~QMPYuvReader()
		{
			delete[] m_saveme;
			if (!m_pipe.isEmpty()) {
				QFile::remove(m_pipe);
				QDir().rmdir(QFileInfo(m_pipe).dir().path());
			}
		}

		// Tells the thread to stop and exit
		void stop()
		{
			m_mutex.lock();
			m_stop = true;
			m_mutex.unlock();
			wait();
		}

	protected:
		// Main thread loop
		void run()
		{
			FILE *f = fopen(m_pipe.toLocal8Bit().data(), "rb");
            if (f == nullptr) {
				qWarning("Can't open pipe");
				return;
			}

			// Parse stream header
			char c;
			int width, height, fps, t1, t2;
			int n = fscanf(f, "YUV4MPEG2 W%d H%d F%d:1 I%c A%d:%d", &width, &height, &fps, &c, &t1, &t2);
			if (n < 3) {
				fclose(f);
				qWarning("Unsupported pipe format");
				return;
			}

			unsigned char *yuv[3];
			yuv[0] = new unsigned char[width * height];
			yuv[1] = new unsigned char[width * height];
			yuv[2] = new unsigned char[width * height];

			QImage image(width, height, QImage::Format_ARGB32);

			// Read frames
			const unsigned int ysize = width * height;
			const unsigned int csize = width * height / 4;
			while (true) {
				m_mutex.lock();
				if (m_stop) {
					m_mutex.unlock();
					break;
				}
				m_mutex.unlock();

				if (fread(yuv[0], 1, 6, f) != 6) {
					goto ioerror;
				}
				if (fread(yuv[0], 1, ysize, f) != ysize) {
					goto ioerror;
				}
				if (fread(yuv[1], 1, csize, f) != csize) {
					goto ioerror;
				}
				if (fread(yuv[2], 1, csize, f) != csize) {
					goto ioerror;
				}
				supersample(yuv[1], width, height);
				supersample(yuv[2], width, height);
				yuvToQImage(yuv, &image, width, height);

				emit imageReady(image);
				continue;

ioerror:
				qWarning("I/O error reading from pipe");
				break;
			}

			delete[] yuv[0];
			delete[] yuv[1];
			delete[] yuv[2];
			fclose(f);
		}

		// 420 to 444 supersampling (from mjpegtools)
		void supersample(unsigned char *buffer, int width, int height)
		{
			unsigned char *inm, *in0, *inp, *out0, *out1;
			unsigned char cmm, cm0, cmp, c0m, c00, c0p, cpm, cp0, cpp;
			int x, y;

            if (m_saveme == nullptr || width > m_savemeSize) {
				delete[] m_saveme;
				m_savemeSize = width;
				m_saveme = new unsigned char[m_savemeSize];
			}
			memcpy(m_saveme, buffer, width);

			in0 = buffer + (width * height / 4) - 2;
			inm = in0 - width/2;
			inp = in0 + width/2;
			out1 = buffer + (width * height) - 1;
			out0 = out1 - width;

			for (y = height; y > 0; y -= 2) {
				if (y == 2) {
					in0 = m_saveme + width/2 - 2;
					inp = in0 + width/2;
				}
				for (x = width; x > 0; x -= 2) {
					cmm = ((x == 2) || (y == 2)) ? in0[1] : inm[0];
					cm0 = (y == 2) ? in0[1] : inm[1];
					cmp = ((x == width) || (y == 2)) ? in0[1] : inm[2];
					c0m = (x == 2) ? in0[1] : in0[0];
					c00 = in0[1];
					c0p = (x == width) ? in0[1] : in0[2];
					cpm = ((x == 2) || (y == height)) ? in0[1] : inp[0];
					cp0 = (y == height) ? in0[1] : inp[1];
					cpp = ((x == width) || (y == height)) ? in0[1] : inp[2];
					inm--;
					in0--;
					inp--;

					*(out1--) = (1*cpp + 3*(cp0+c0p) + 9*c00 + 8) >> 4;
					*(out1--) = (1*cpm + 3*(cp0+c0m) + 9*c00 + 8) >> 4;
					*(out0--) = (1*cmp + 3*(cm0+c0p) + 9*c00 + 8) >> 4;
					*(out0--) = (1*cmm + 3*(cm0+c0m) + 9*c00 + 8) >> 4;
				}
				out1 -= width;
				out0 -= width;
			}
		}

		// Converts YCbCr data to a QImage
		void yuvToQImage(unsigned char *planes[], QImage *dest, int width, int height)
		{
			unsigned char *yptr = planes[0];
			unsigned char *cbptr = planes[1];
			unsigned char *crptr = planes[2];

			// This is partly from mjpegtools
			for (int y = 0; y < height; y++) {
				QRgb *dptr = (QRgb *)dest->scanLine(y);
				for (int x = 0; x < width; x++) {
					*dptr = qRgb(qBound(0, (RGB_Y[*yptr] + R_Cr[*crptr]) >> 18, 255),
						qBound(0, (RGB_Y[*yptr] + G_Cb[*cbptr]+ G_Cr[*crptr]) >> 18, 255),
						qBound(0, (RGB_Y[*yptr] + B_Cb[*cbptr]) >> 18, 255));
					++yptr;
					++cbptr;
					++crptr;
					++dptr;
				}
			}
		}

		// Rounding towards zero
		inline int zround(double n)
		{
			if (n >= 0) {
                return static_cast<int>(n + 0.5);
			} else {
                return static_cast<int>(n - 0.5);
			}
		}

		// Initializes the YCbCr -> RGB conversion tables (again, from mjpegtools)
		void initTables(void)
		{
			/* clip Y values under 16 */
			for (int i = 0; i < 16; i++) {
                RGB_Y[i] = zround((1.0 * static_cast<double>(16-16) * 255.0 / 219.0 * static_cast<double>(1<<18)) + static_cast<double>(1<<(18-1)));
			}
			for (int i = 16; i < 236; i++) {
                RGB_Y[i] = zround((1.0 * static_cast<double>(i - 16) * 255.0 / 219.0 * static_cast<double>(1<<18)) + static_cast<double>(1<<(18-1)));
			}
			/* clip Y values above 235 */
			for (int i = 236; i < 256; i++) {
                RGB_Y[i] = zround((1.0 * static_cast<double>(235 - 16)  * 255.0 / 219.0 * static_cast<double>(1<<18)) + static_cast<double>(1<<(18-1)));
			}

			/* clip Cb/Cr values below 16 */   
			for (int i = 0; i < 16; i++) {
                R_Cr[i] = zround(1.402 * static_cast<double>(-112) * 255.0 / 224.0 * static_cast<double>(1<<18));
                G_Cr[i] = zround(-0.714136 * static_cast<double>(-112) * 255.0 / 224.0 * static_cast<double>(1<<18));
                G_Cb[i] = zround(-0.344136 * static_cast<double>(-112) * 255.0 / 224.0 * static_cast<double>(1<<18));
                B_Cb[i] = zround(1.772 * static_cast<double>(-112) * 255.0 / 224.0 * static_cast<double>(1<<18));
			}
			for (int i = 16; i < 241; i++) {
                R_Cr[i] = zround(1.402 * static_cast<double>(i - 128) * 255.0 / 224.0 * static_cast<double>(1<<18));
                G_Cr[i] = zround(-0.714136 * static_cast<double>(i - 128) * 255.0 / 224.0 * static_cast<double>(1<<18));
                G_Cb[i] = zround(-0.344136 * static_cast<double>(i - 128) * 255.0 / 224.0 * static_cast<double>(1<<18));
                B_Cb[i] = zround(1.772 * static_cast<double>(i - 128) * 255.0 / 224.0 * static_cast<double>(1<<18));
			}
			/* clip Cb/Cr values above 240 */  
			for (int i = 241; i < 256; i++) {
                R_Cr[i] = zround(1.402 * static_cast<double>(112) * 255.0 / 224.0 * static_cast<double>(1<<18));
                G_Cr[i] = zround(-0.714136 * static_cast<double>(112) * 255.0 / 224.0 * static_cast<double>(1<<18));
                G_Cb[i] = zround(-0.344136 * static_cast<double>(i-128) * 255.0 / 224.0 * static_cast<double>(1<<18));
                B_Cb[i] = zround(1.772 * static_cast<double>(112) * 255.0 / 224.0 * static_cast<double>(1<<18));
			}
		}

	signals:
		void imageReady(const QImage &image);

	public:
		QString m_pipe;

	private:
		QMutex m_mutex;
		bool m_stop;

		// Conversion tables
		int RGB_Y[256];
		int R_Cr[256];
		int G_Cb[256];
		int G_Cr[256];
		int B_Cb[256];

		// Temporary buffers
		unsigned char *m_saveme;
		int m_savemeSize;
};
