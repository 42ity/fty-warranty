#
#    fty-warranty - Agent sending metrics about warranty expiration
#
#    Copyright (C) 2014 - 2017 Eaton                                        
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
%define SYSTEMD_UNIT_DIR %(pkg-config --variable=systemdsystemunitdir systemd)
Name:           fty-warranty
Version:        1.0.0
Release:        1
Summary:        agent sending metrics about warranty expiration
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
BuildRequires:  systemd-devel
BuildRequires:  systemd
%{?systemd_requires}
BuildRequires:  xmlto
BuildRequires:  gcc-c++
BuildRequires:  libsodium-devel
BuildRequires:  zeromq-devel
BuildRequires:  czmq-devel
BuildRequires:  malamute-devel
BuildRequires:  fty-proto-devel
BuildRequires:  cxxtools-devel
BuildRequires:  tntdb-devel
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
fty-warranty agent sending metrics about warranty expiration.


%prep

%setup -q

%build
sh autogen.sh
%{configure} --enable-drafts=%{DRAFTS} --with-systemd-units
make %{_smp_mflags}

%install
make install DESTDIR=%{buildroot} %{?_smp_mflags}

# remove static libraries
find %{buildroot} -name '*.a' | xargs rm -f
find %{buildroot} -name '*.la' | xargs rm -f

%files
%defattr(-,root,root)
%{_bindir}/biostimer-warranty-metric
%{_mandir}/man1/biostimer-warranty-metric*
%config(noreplace) %{_sysconfdir}/fty-warranty/biostimer-warranty-metric.cfg
%{SYSTEMD_UNIT_DIR}/biostimer-warranty-metric.service
%{SYSTEMD_UNIT_DIR}/biostimer-warranty-metric.timer
%dir %{_sysconfdir}/fty-warranty
%if 0%{?suse_version} > 1315
%post
%systemd_post biostimer-warranty-metric.service biostimer-warranty-metric.timer
%preun
%systemd_preun biostimer-warranty-metric.service biostimer-warranty-metric.timer
%postun
%systemd_postun_with_restart biostimer-warranty-metric.service biostimer-warranty-metric.timer
%endif

%changelog
