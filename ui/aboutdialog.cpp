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
#include <QDesktopServices>
#ifdef USE_LIBNOTIFY
 #include "services/notificationservice-libnotify.h"
#endif

#define PROJECT_HOMEPAGE "https://mystiqapp.com/"

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
    QTextBrowser *donations = ui->textDonations;
    QTextBrowser *license = ui->txtLicense;
    QTextBrowser *changelog = ui->txtChangelog;

    info->setOpenExternalLinks(true);

    info->setText(
         "<b>MystiQ Video Converter "+ QString("%1</b>").arg(VERSION_STRING)
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
         + tr("Developers:<br> %1").arg("<b>Maikel Llamaret Heredia</b>: llamaret@webmisolutions.com<br><b>Luis Felipe Domínguez Vega</b>: ldominguezvega@gmail.com<br><br>"
         + tr("Collaborators:<br> %1").arg("<b>Gabriel A. López López</b>: glpz@daxslab.com<br><b>Pavel Milanés Costa</b>: pavelmc@gmail.com<br><b>Carlos Cesar Caballero</b>: ccesar@daxslab.com<br><b>Juan José Morejón Angulo </b>: bxt.jjma@gmail.com<br><b>Jenny Cabrera Varona </b>: jenny.cabrera@yandex.com<br><b>Alexis López Zubieta</b>: contact@azubieta.net<br><b>Leodanis Pozo Ramos</b>: lpozor78@gmail.com") + "<br><br>")
         //: %1 is the name and email of the logo designer
         + tr("Application Name:<br> %1").arg("<b>Hugo Florentino</b>: cre8or@gmx.net") + "<br><br>"
         + tr("Icons Theme:<br> %1").arg("<b>Fabián Inostroza Oyarzún</b>: fabian_alexis@icloud.com") + "<br><br>"
         + tr("This program is free software; you can redistribute it and/or modify it "
              "under the terms of the GNU General Public License version 3.")
         + "<br><br>"
         + tr("Many people have contributed translations. You can also help translate the MystiQ Video Converter into your own language. Visit ")
         + url("https://www.transifex.com/swl-x-project/mystiq-video-converter/")
         + tr(" and join a translation team.")+"<br><br>"
         + tr("Some audio-processing functionalities are provided by SoX.")
         + " (" + url("http://sox.sourceforge.net/") + ")<br><br>"
         + tr("Some features of MystiQ Video Converter have been inspired by")+(" <b>VideoMorph</b>, <b>QWinff</b>, <b>Curlew</b>, <b>Ciano</b>, <b>HandBrake</b>, <b>FF Multi Converter</b>.")+"<br>"
         + url("https://videomorph.webmisolutions.com") + "<br>"
         + url("http://qwinff.github.io") + "<br>"
         + url("http://sourceforge.net/projects/curlew") + "<br>"
         + url("https://robertsanseries.github.io/ciano") + "<br>"
         + url("https://handbrake.fr") + "<br>"
         + url("https://sites.google.com/site/ffmulticonverter")
         + "<br>"
         );
    translators->setHtml(getTranslators());
    //translators->setText(getTranslators());

    donations->setText(
        "<p><b>MystiQ Video Converter</b> " + tr("is a free open source application. No one has the right to charge you any fees for the use of this application.</p><p>However, the community has contributed monetarily to the support of the project, to improve the conditions of the development team workflow. Some of our financial contributors are:</p>")
                                                +"<ul>"
                                                 "<li>Ernesto L. Acosta Valdés</li>"
                                                 "<li>Hugo Florentino</li>"
                                                 "<li>Rober García</li>"
                                                 "<li>Francisco Perdigón Romero</li>"
                                                 "<li>Mauricio López</li>"
                                                 "<li>Gabriel A. López</li>"
                                                 "<li>Juan José Morejón</li>"
                                                 "<li>Luis Felipe Domínguez</li>"
                                                 "<li>Yoel Torres</li>"
                                                 "<li>Óscar L. Garcell Martínez</li>"
                                                 "<li>Arián López Delgado</li>"
                                                 "<li>Danny Paula</li>"
                                                 "<li>Yanssel Peral Martínez</li>"
                                                 "<li>Daniel Villazón</li>"
                                                 "<li>Raulo (Arthanys)</li>"
                                                 "<li>Armando Felipe</li>"
                                                 "<li>Ernesto Santana</li>"
                                                 "<li>Dennis Quesada Cruz</li>"
                                                 "<li>Raúl Alderete</li>"
                                                 "<li>Adrián Rodríguez</li>"
                                                 "<li>Leslie León Sinclair</li>"
                                                 "<li>Rafael Salgueiro</li>"
                                                 "</ul>"
                + tr("There are other users who collaborate financially with the development of MystiQ Video Converter anonymously. We are infinitely grateful to all of them. Any amount, however small it may seem, contributes a lot.")
         );

    // Constraint the width of text area to the width of the banner.
    //info->setMaximumWidth(ui->lblBanner->pixmap()->width());

    // Set the background color of the textbox to the color of the window.
    QPalette p = info->palette();
    p.setColor(QPalette::Base, this->palette().color(QPalette::Window));

    info->setPalette(p);
    info->setFrameShape(QTextBrowser::NoFrame);  // Hide textbox border.

    translators->setPalette(p);
    translators->setFrameShape(QTextBrowser::NoFrame);

    donations->setPalette(p);
    donations->setFrameShape(QTextBrowser::NoFrame);

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
    ui->tabDonations->setAutoFillBackground(true);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

QString AboutDialog::getTranslators()
{
    struct {QString locale; QString translator;} table[] = {
        //: Turkish Language (Turkey)
        {"tr_TR", trad(tr("Turkish (Turkey)")
                    , "Serdar Sağlam")},
        //: Swedish Language (Sweden)
        {"sv_SV", trad(tr("Swedish (Sweden)")
                    , "Åke Engelbrektson")},
        //: Japanese Language (Japan)
        {"jp_JP", trad(tr("Japanese (Japan)")
                    , "Tilt")},
        //: German Language (Germany)
        {"de_DE", trad(tr("German (Germany)")
                    , "Leslie León Sinclair")},
        //: Italian Language (Italy)
        {"it_IT", trad(tr("Italian (Italy)")
                    , "SymbianFlo")},
        //: Spanish Language (Spain)
        {"es_ES", trad(tr("Spanish (Spain)")
                    , "Maikel Llamaret Heredia")},
        //: Hungarian Language (Hungary)
        {"hu_HU", trad(tr("Hungarian (Hungary)")
                    , "Varga Gábor")},
        //: Russian Language (Russia)
        {"hu_HU", trad(tr("Russian (Russia)")
                    , "Алексей Суднев, Виктор Ерухин, Александр, Андрей Селезнев, Жердагдемедийна Гуравча, Просто Человек")},
        //: Galician Language (Galicia)
        {"gl_GL", trad(tr("Galician (Galicia)")
                    , "Leandro Vergara")},
        //: Indonesian Language (Indonesia)
        {"id_ID", trad(tr("Indonesian (Indonesia)")
                    , "Kiki Syahadat")},
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

void AboutDialog::on_github_button_clicked()
{
    QString github_go="https://github.com/swl-x/MystiQ/";
    bool b = QDesktopServices::openUrl( QUrl( github_go, QUrl::TolerantMode ) );
    Q_UNUSED(b)
}

void AboutDialog::on_gitter_button_clicked()
{
    QString gitter_go="https://gitter.im/swl-x-MystiQ/community";
    bool b = QDesktopServices::openUrl( QUrl( gitter_go, QUrl::TolerantMode ) );
    Q_UNUSED(b)
}

void AboutDialog::on_facebook_button_clicked()
{
    QString facebook_go="https://facebook.com/blogswlx";
    bool b = QDesktopServices::openUrl( QUrl( facebook_go, QUrl::TolerantMode ) );
    Q_UNUSED(b)
}

void AboutDialog::on_twitter_button_clicked()
{
    QString twitter_go="https://twitter.com/swl_swlx";
    bool b = QDesktopServices::openUrl( QUrl( twitter_go, QUrl::TolerantMode ) );
    Q_UNUSED(b)
}

void AboutDialog::on_transifex_button_clicked()
{
    QString transifex_go="https://www.transifex.com/swl-x-project/mystiq-video-converter/";
    bool b = QDesktopServices::openUrl( QUrl( transifex_go, QUrl::TolerantMode ) );
    Q_UNUSED(b)
}

void AboutDialog::on_liberapay_button_clicked()
{
    QString liberapay_go="https://liberapay.com/MystiQ/donate";
    bool b = QDesktopServices::openUrl( QUrl( liberapay_go, QUrl::TolerantMode ) );
    Q_UNUSED(b)
}

void AboutDialog::on_opencollective_button_clicked()
{
    QString opencollective_go="https://opencollective.com/mystiq/donate";
    bool b = QDesktopServices::openUrl( QUrl( opencollective_go, QUrl::TolerantMode ) );
    Q_UNUSED(b)
}

void AboutDialog::on_patreon_button_clicked()
{
    QString patreon_go="https://patreon.com/mystiq";
    bool b = QDesktopServices::openUrl( QUrl( patreon_go, QUrl::TolerantMode ) );
    Q_UNUSED(b)
}
