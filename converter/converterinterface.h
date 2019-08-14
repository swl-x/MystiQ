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

#ifndef CONVERTERINTERFACE_H
#define CONVERTERINTERFACE_H

#include <QObject>
#include <QStringList>
#include <QProcess>
#include "conversionparameters.h"

/**
  Abstract Converter Interface
  Derived class must implement the virtual functions to provide converter details.
  */

class ConverterInterface : public QObject
{
    Q_OBJECT
public:
    explicit ConverterInterface(QObject *parent = nullptr);
    virtual QString executableName() const = 0;
    virtual void reset() = 0;
    virtual QProcess::ProcessChannel processReadChannel() const = 0;

    /*! Fill parameter list and determine whether the configuration needs
     *  additional audio filtering. This function must be called before
     *  starting the process. Implementations of this function should
     *  do necessary setup, such as obtaining stream duration for calculating
     *  progress.
     *  @param param [in] the conversion parameter
     *  @param list [out] the command-line parameter list
     *  @param needs_audio_filter [out] whether AudioFilter should be used. The return value is true
     *          implies that the parameter list makes the conversion process
     *          wait for data input from stdin.
     */
    virtual void fillParameterList(const ConversionParameters& param, QStringList& list
                                   , bool *needs_audio_filter) = 0;
    virtual void parseProcessOutput(const QString& line) = 0;
    virtual double progress() const = 0;
    virtual QString errorMessage() const;

signals:
    void progressRefreshed(double percentage);

public slots:

};

#endif // CONVERTERINTERFACE_H
