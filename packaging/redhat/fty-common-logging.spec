#
#    fty-common-logging - Provides common logs
#
#    Copyright (C) 2014 - 2018 Eaton
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

# To build with draft APIs, use "--with drafts" in rpmbuild for local builds or add
#   Macros:
#   %_with_drafts 1
# at the BOTTOM of the OBS prjconf
%bcond_with drafts
%if %{with drafts}
%define DRAFTS yes
%else
%define DRAFTS no
%endif
Name:           fty-common-logging
Version:        1.0.0
Release:        1
Summary:        provides common logs
License:        GPL-2.0+
URL:            https://42ity.org
Source0:        %{name}-%{version}.tar.gz
Group:          System/Libraries
# Note: ghostscript is required by graphviz which is required by
#       asciidoc. On Fedora 24 the ghostscript dependencies cannot
#       be resolved automatically. Thus add working dependency here!
BuildRequires:  ghostscript
BuildRequires:  asciidoc
BuildRequires:  automake
BuildRequires:  autoconf
BuildRequires:  libtool
BuildRequires:  pkgconfig
BuildRequires:  xmlto
BuildRequires:  gcc-c++
BuildRequires:  log4cplus-devel
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
fty-common-logging provides common logs.

%package -n libfty_common_logging1
Group:          System/Libraries
Summary:        provides common logs shared library

%description -n libfty_common_logging1
This package contains shared library for fty-common-logging: provides common logs

%post -n libfty_common_logging1 -p /sbin/ldconfig
%postun -n libfty_common_logging1 -p /sbin/ldconfig

%files -n libfty_common_logging1
%defattr(-,root,root)
%{_libdir}/libfty_common_logging.so.*

%package devel
Summary:        provides common logs
Group:          System/Libraries
Requires:       libfty_common_logging1 = %{version}
Requires:       log4cplus-devel

%description devel
provides common logs development tools
This package contains development files for fty-common-logging: provides common logs

%files devel
%defattr(-,root,root)
%{_includedir}/*
%{_libdir}/libfty_common_logging.so
%{_libdir}/pkgconfig/libfty_common_logging.pc
%{_mandir}/man3/*
%{_mandir}/man7/*

%prep

%setup -q

%build
sh autogen.sh
%{configure} --enable-drafts=%{DRAFTS}
make %{_smp_mflags}

%install
make install DESTDIR=%{buildroot} %{?_smp_mflags}

# remove static libraries
find %{buildroot} -name '*.a' | xargs rm -f
find %{buildroot} -name '*.la' | xargs rm -f


%changelog
