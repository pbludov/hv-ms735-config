Summary: hv-ms735-config
Name: hv-ms735-config
Provides: hv-ms735-config
Version: 1.2
Release: 1%{?dist}
License: LGPL-2.1+
Source: %{name}.tar.gz
URL: https://github.com/pbludov/hv-ms735-config
Vendor: Pavel Bludov <pbludov@gmail.com>
Packager: Pavel Bludov <pbludov@gmail.com>

BuildRequires: make, gcc-c++, libusb1-devel, libhidapi-devel

%{?rhl:Requires: qt5-qtbase}
%{?rhl:BuildRequires: qt5-qtbase-devel}

%{?fedora:Requires: qt5}
%{?fedora:BuildRequires: qt-devel}

%description
HAVIT Magic Eagle mouse unofficial configuration utility.
Allows you to configure the buttons and profiles of your device.

%global debug_package %{nil}

%define _rpmfilename %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm

%prep
%setup -c %{name}
 
%build
qmake-qt5 PREFIX=%{_prefix} QMAKE_CFLAGS+="%optflags" QMAKE_CXXFLAGS+="%optflags";
make -j 2 %{?_smp_mflags};

%install
make install INSTALL_ROOT="%buildroot";

%files
%defattr(-,root,root)
%{_mandir}/man1/%{name}.1.gz
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/48x48/apps/%{name}.png

%changelog
* Sun Apr 9 2017 Pavel Bludov <pbludov@gmail.com>
+ Version 1.0
- Initial comit