######################################################
# SpecFile: MistiQ.spec 
# Generato: http://www.mandrivausers.ro/
######
%define oname MystiQ

Summary:	Audio/Video converter
Name:		mystiq
Version:	20.03.23
Release:	1
License:	GPLv3
Group:		Video
Url:		https://mystiqapp.com/
Source0:	https://github.com/swl-x/MystiQ/archive/v%{version}.tar.gz
Patch0:		rosa-data.patch
BuildRequires:	qt5-devel
BuildRequires:	qt5-tools
BuildRequires:	qt5-linguist-tools
BuildRequires:	pkgconfig(libavresample)
BuildRequires:	pkgconfig(alsa)
BuildRequires:	pkgconfig(x11)
BuildRequires:	pkgconfig(sox)
BuildRequires:	desktop-file-utils
BuildRequires:	imagemagick
BuildRequires:	appstream-util
BuildRequires:	pkgconfig(Qt5Declarative)
BuildRequires:	pkgconfig(Qt5Multimedia)
BuildRequires:	pkgconfig(Qt5QuickWidgets)
Requires:	ffmpeg
Requires:	sox

%description
MystiQ is a GUI for FFmpeg, a powerful media converter. 
FFmpeg can read audio and video files in various 
formats and convert them into other formats. 
MystiQ features an intuitive graphical 
interface and a rich set of presets to help you 
convert media files within a few clicks.
Advanced users can also adjust conversion parameters in detail.

%files
%doc LICENSE README.md CONTRIBUTING.md
%{_bindir}/%{name}
%{_datadir}/%{name}/translations
%{_iconsdir}/hicolor/scalable/apps/%{name}.svg
%{_datadir}/applications/*.desktop
%{_mandir}/man1/*.1.xz

#-----------------------------------------------------------------------------

%prep
%setup -qn %{oname}-%{version}
%patch0 -p0
chmod -x mystiq.desktop icons/mystiq.svg
lrelease-qt5 *.pro

%build
%qmake_qt5 DEFINES+=NO_NEW_VERSION_CHECK 
%make 

%install
%makeinstall_qt 

# man
mkdir -p %{buildroot}%{_mandir}/man1
cp -R man/%{name}.1.gz %{buildroot}%{_mandir}/man1/%{name}.1.gz

#lang
mkdir -p %{buildroot}%{_datadir}/%{name}
cp -pR translations %{buildroot}%{_datadir}/%{name}/translations

