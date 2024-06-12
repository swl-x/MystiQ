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

#include "conversionparameters.h"
#include "mediaprobe.h"
#include <QStringList>
#include <QDebug>
#include <QRegularExpression>
#include <cassert>

#define TIMEOUT 3000

namespace {
int parseFFmpegArguments(QStringList& args, int index, ConversionParameters& result) {
    int used_arg_count = 0;
    try {
        QString& arg = args[index];

        /* BEGIN MACRO DEFINITIONS */

#define CHECK_OPTION_BEGIN if (false) do { } while (0)

#define CHECK_OPTION(argument) else if (arg == argument)

// absorb options like "-option" with no parameter
// set result.property to value
#define CHECK_OPTION_1(argument, property, value) \
    else if (arg == argument) do { result.property = value; used_arg_count = 1; } while (0)

// absorb options like "-option str" where "str" is a string.
// set result.property to the next token
#define CHECK_OPTION_2(argument, property) \
    else if (arg == argument) do { \
        result.property = args[index+1]; \
        used_arg_count = 2; } while (0)

// absorb options like "-option value" where "value" has numeric meaning
// convertmethod is the corresponding conversion method of QString
// set result.property to the converted value of the next token
#define CHECK_OPTION_2_METHOD(argument, property, convertmethod) \
    else if (arg == argument) do { \
        QString nextarg = args[index+1]; \
        nextarg.replace(QRegularExpression("[a-z]"), ""); /* remove units */ \
        result.property = nextarg.convertmethod(); \
        used_arg_count = 2; } while (0)

#define CHECK_OPTION_2_FUNCTION(argument, property, convertfunction) \
    else if (arg == argument) do { \
        QString nextarg = args[index+1]; \
        nextarg.replace(QRegExp("[a-z]"), ""); /* remove units */ \
        result.property = convertfunction(nextarg); \
        used_arg_count = 2; } while (0)

#define CHECK_OPTION_END do { } while (0)

        /* END MACRO DEFINITIONS */

        if (arg[0] == '-') {

            CHECK_OPTION_BEGIN;

            // Threads
            CHECK_OPTION_2_METHOD("-threads", threads, toInt);

            // Audio Options
            CHECK_OPTION_1("-an", disable_audio, true);
            CHECK_OPTION_2_METHOD("-ab", audio_bitrate, toInt);
            CHECK_OPTION_2_METHOD("-ar", audio_sample_rate, toInt);
            CHECK_OPTION_2_METHOD("-ac", audio_channels, toInt);
            CHECK_OPTION_2_METHOD("-vol", audio_volume, toInt);

            // Video Options
            CHECK_OPTION_1("-vn", disable_video, true);
            CHECK_OPTION_1("-vf", insert_subtitle, true);
            CHECK_OPTION_1("-vf", disable_color, true);
            CHECK_OPTION_1("-vf", vertical_flip, true);
            CHECK_OPTION_1("-vf", horizontal_flip, true);
            CHECK_OPTION_1("-vf", rotate_90more, true);
            CHECK_OPTION_1("-vf", rotate_90less, true);
            CHECK_OPTION_1("-vf", rotate_180, true);
            CHECK_OPTION_1("-vf", rggm, true);
            CHECK_OPTION_1("-vf", rbgm, true);
            CHECK_OPTION_1("-vf", rcc, true);
            CHECK_OPTION_1("-vf", rchc, true);
            CHECK_OPTION_1("-vf", rcd, true);
            CHECK_OPTION_1("-vf", gmgm, true);
            CHECK_OPTION_1("-vf", gmc, true);
            CHECK_OPTION_1("-vf", ybc, true);
            CHECK_OPTION_1("-sameq", video_same_quality, true);
            CHECK_OPTION_1("-deinterlace", video_deinterlace, true);
            CHECK_OPTION_2_METHOD("-b", video_bitrate, toInt);
            CHECK_OPTION_2_METHOD("-croptop", video_crop_top, toInt);
            CHECK_OPTION_2_METHOD("-cropbottom", video_crop_bottom, toInt);
            CHECK_OPTION_2_METHOD("-cropleft", video_crop_left, toInt);
            CHECK_OPTION_2_METHOD("-cropright", video_crop_right, toInt);

            /* TODO: add begin time and duration */

            // width and height are in the same parameter (for example: "-s 800x600")
            CHECK_OPTION("-s") {
                QRegularExpression pattern(QString::fromLatin1("([0-9]+)x([0-9]+)"));
                QRegularExpressionMatch match = pattern.match(args[index+1]);
                if (match.hasMatch()) {
                    result.video_width = match.captured(1).toInt();
                    result.video_height = match.captured(2).toInt();
                    used_arg_count = 2;
                }
            }

            CHECK_OPTION("-filter:v")
            {
                QRegularExpression pattern(QString::fromLatin1("crop=(\\d+):(\\d+):(\\d+):(\\d+)"));
                QRegularExpressionMatch match = pattern.match(args[index + 1]);

                if (match.hasMatch())
                {
                    int out_w = match.captured(1).toInt();
                    int out_h = match.captured(2).toInt();
                    int x = match.captured(3).toInt();
                    int y = match.captured(4).toInt();

                    result.video_crop_top = y;
                    result.video_crop_left = x;
                    result.video_crop_right = out_w + x;
                    result.video_crop_bottom = out_h + y;

                    used_arg_count = 2;
                }
            }

            CHECK_OPTION_END;

        }

    } catch (...) {
        return 0;
    }
    return used_arg_count;

#undef CHECK_OPTION_BEGIN
#undef CHECK_OPTION_1
#undef CHECK_OPTION_2
#undef CHECK_OPTION_2_METHOD
#undef CHECK_OPTION_2_FUNCTION
#undef CHECK_OPTION_END
}
}

void ConversionParameters::copyConfigurationFrom(const ConversionParameters &src)
{
    QString orig_src = source;
    QString orig_dest = destination;
    *this = src;
    source = orig_src;
    destination = orig_dest;
}

ConversionParameters
ConversionParameters::fromFFmpegParameters(const QString &params_str)
{
    ConversionParameters result;
    QStringList args = params_str.split(QString::fromLatin1(" "), Qt::SkipEmptyParts);

    for (int i=0; i<args.size();) {
        int used_arg_count = parseFFmpegArguments(args, i, result);

        if (used_arg_count) {
            for (int k=0; k<used_arg_count; k++)
                args.removeAt(i); // remove recognized arguments
        } else {
            i++;
        }
    }

    result.ffmpeg_options = args.join(QString::fromLatin1(" ")); // unrecognized arguments

    return result;
}

ConversionParameters
ConversionParameters::fromFFmpegParameters(const char *params_str)
{
    return fromFFmpegParameters(QString::fromLatin1(params_str));
}
