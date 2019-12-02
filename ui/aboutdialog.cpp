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

#include <QtGlobal>
#include <QLocale>
#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "version.h"
#include "services/constants.h"
#ifdef USE_LIBNOTIFY
 #include "services/notificationservice-libnotify.h"
#endif

#define PROJECT_HOMEPAGE "https://mystiq.swlx.info/"

namespace {
QString url(QString lnk)
{
    return QString("<a href=\"%1\">%1</a>").arg(lnk);
}
}

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    QTextBrowser *info = ui->txtInfo;
    QTextBrowser *translators = ui->txtTranslators;
    QTextBrowser *license = ui->txtLicense;
    QTextBrowser *changelog = ui->txtChangelog;

    info->setOpenExternalLinks(true);

    info->setText(
         "<b>MystiQ </b>"+ tr("%1").arg(VERSION_STRING)
#ifdef VERSION_ID_STRING
         + QString(" %1").arg(QString(VERSION_ID_STRING))
#endif
         + " "
         + ((Constants::getBool("Portable"))
                /*: Portable version (no installation, no writing registry) */
                ? tr("Portable") : "")
         + "<br>"
         /*: Qt version */
         + tr("Compiled with Qt %1").arg(QT_VERSION_STR)
#ifdef USE_LIBNOTIFY /*: libnotify version */
                + "<br>" + tr("Compiled with libnotify %1")
                .arg(NotificationService_libnotify::getVersion())
#endif
         + "<br>"
         + tr("MystiQ Homepage: %1").arg(url(PROJECT_HOMEPAGE))
         + "<br>"
         + tr("MystiQ is a GUI frontend for FFmpeg.")
         + "<br><br>"
         //: %1 is the name and email of the programmer
         + tr("Developers:<br> %1").arg("<b>Maikel Llamaret Heredia</b>: llamaret@webmisolutions.com<br><b>Gabriel A. López López</b>: glpz@daxslab.com<br><b>Pavel Milanés Costa</b>: pavelmc@gmail.com<br><b>Carlos Cesar Caballero</b>: ccesar@daxslab.com") + "<br><br>"
         //: %1 is the name and email of the logo designer
         + tr("Aplication Name:<br> %1").arg("<b>Hugo Florentino</b>: cre8or@gmx.net") + "<br><br>"
         + tr("This program is free software; you can redistribute it and/or modify it "
              "under the terms of the GNU General Public License version 2 or 3.")
         + "<br><br>"
         + tr("Some audio-processing functionalities are provided by SoX.")
         + " (" + url("http://sox.sourceforge.net/") + ")<br><br>"
         + tr("FFmpeg presets were taken from <b>VideoMorph</b>, <b>QWinff</b>, <b>Curlew</b>, <b>Ciano</b> and <b>FF Multi Converter</b>.")+"<br>"
         + " (" + url("https://videomorph.webmisolutions.com/") + ")<br>"
         + " (" + url("http://qwinff.github.io") + ")<br>"
         + " (" + url("http://sourceforge.net/projects/curlew") + ")<br>"
         + " (" + url("https://robertsanseries.github.io/ciano/") + ")<br>"
         + " (" + url("https://sites.google.com/site/ffmulticonverter/") + ")"
         + "<br>"
         );
    translators->setHtml(getTranslators());
    //translators->setText(getTranslators());

    // Constraint the width of text area to the width of the banner.
    //info->setMaximumWidth(ui->lblBanner->pixmap()->width());

    // Set the background color of the textbox to the color of the window.
    QPalette p = info->palette();
    p.setColor(QPalette::Base, this->palette().color(QPalette::Window));

    info->setPalette(p);
    info->setFrameShape(QTextBrowser::NoFrame);  // Hide textbox border.

    translators->setPalette(p);
    translators->setFrameShape(QTextBrowser::NoFrame);

    license->setPalette(p);
    license->setFrameShape(QTextBrowser::NoFrame);

    changelog->setPalette(p);
    changelog->setFrameShape(QTextBrowser::NoFrame);

    // Make the window size fixed.
    this->adjustSize();
    this->setMinimumWidth(this->width());
    this->setMinimumHeight(this->height());
    this->setMaximumWidth(this->width());
    this->setMaximumHeight(this->height());

    ui->tabInfo->setAutoFillBackground(true);
    ui->tabTranslators->setAutoFillBackground(true);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

QString AboutDialog::getTranslators()
{
    struct {QString locale; QString translator;} table[] = {
        //: English Language (USA)
        {"en_US", trad(tr("English (USA)")
                    , "Maikel Llamaret <llamaret@webmisolutions.com>")},
        //: Spanish Language (Spain)
        {"es_ES", trad(tr("Spanish (Spain)")
                    , "Maikel Llamaret <llamaret@webmisolutions.com>")},
    };
    const int size = sizeof(table) / sizeof(table[0]);

    QStringList translators;
    QString current_locale = QLocale::system().name();
    for (int i=0; i<size; i++) {
        // Put the translator of the current language at the top.
        if (current_locale.startsWith(table[i].locale))
            translators.push_front(table[i].translator);
        else
            translators.push_back(table[i].translator);
    }

    return translators.join("");
}

QString AboutDialog::trad(const QString& lang, const QString& author)
{
    return trad(lang, QStringList() << author);
}

QString AboutDialog::trad(const QString& lang, const QStringList& authors)
{
    QString s = "<ul>";
    for (int n = 0; n < authors.count(); n++) {
        QString author = authors[n];
        s += "<li>"+ author.replace("<", "&lt;").replace(">", "&gt;")
                + "</li>";
    }
    s+= "</ul>";

    return QString("<b>%1</b>: %2").arg(lang).arg(s);
}
