Name: hv-ms735-config
Provides: hv-ms735-config
Version: 1.1.1
Release: 1%{?dist}
License: LGPL-2.1+
Source: %{name}.tar.gz
URL: https://github.com/pbludov/hv-ms735-config
Vendor: Pavel Bludov <pbludov@gmail.com>
Packager: Pavel Bludov <pbludov@gmail.com>
Summary: HAVIT Magic Eagle HV-MS735 mouse configuration utility

%description
HAVIT Magic Eagle mouse unofficial configuration utility.
Allows you to configure the buttons and profiles of your device.

%global debug_package %{nil}

BuildRequires: make, gcc-c++

%{?fedora:BuildRequires:          qt5-qtbase-devel, libusb1-devel,       hidapi-devel}
%{?rhel:BuildRequires:            qt5-qtbase-devel, libusb1-devel,       hidapi-devel}
%{?suse_version:BuildRequires: libqt5-qtbase-devel, libusb-1_0-devel, libhidapi-devel}

%if 0%{?mageia}
%define qmake qmake
BuildRequires: libusb1-devel, hidapi-devel
%ifarch x86_64 amd64
BuildRequires: lib64qt5base5-devel 
%else
BuildRequires: libqt5base5-devel 
%endif
%else
%define qmake qmake-qt5
%endif

%prep
%setup -c %{name}
 
%build
%{qmake} PREFIX=%{_prefix} QMAKE_CFLAGS+="%optflags" QMAKE_CXXFLAGS+="%optflags"
make %{?_smp_mflags}

%install
make install INSTALL_ROOT="%buildroot"

%files
%defattr(-,root,root)
%{_sysconfdir}/udev/rules.d/51-hv-ms735-mouse.rules
%{_mandir}/man1/%{name}.1.*
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/48x48/apps/%{name}.png

%posttrans
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
/usr/bin/update-desktop-database &> /dev/null || :

%changelog
* Sun Apr 23 2017 Pavel Bludov <pbludov@gmail.com>
+ Version 1.1.0
- Mouse events.
- Finaly fix MacOS mouse connectivity issue.

* Sat Apr 15 2017 Pavel Bludov <pbludov@gmail.com>
+ Version 1.0.1
- USB report rate.
- Configurable delay for the neon illumination.
- More stable MacOS version.

* Sun Apr 9 2017 Pavel Bludov <pbludov@gmail.com>
+ Version 1.0
- Initial commit
