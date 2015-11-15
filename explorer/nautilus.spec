%define __spec_install_post /usr/lib/rpm/brp-compress
Name:             nautilus
Summary:          Nautilus is a network user environment
Version:          2.22.5.1
Release:          1
License:          GPL
Group:            User Interface/Desktop
Source:           ftp://ftp.gnome.org/pub/GNOME/sources/%{name}-%{version}.tar.gz
URL:              http://nautilus.eazel.com/
BuildRoot:        %{_tmppath}/%{name}-%{version}-root
BuildRequires:    pkgconfig >= 0.8
Requires:         esound >= @ESOUND_REQUIRED@
Requires:         bonobo-activation >= 2.1.0
Requires:         eel2 >= 2.21.90
Requires:         glib2 >= 2.15.6
Requires:         gnome-vfs2 >= @GNOME_VFS_REQUIRED@
Requires:         gtk2 >= 2.11.6
Requires:         libart_lgpl >= @ART_REQUIRED@
Requires:         libbonobo >= 2.1.0
Requires:         libbonoboui >= @BONOBO_UI_REQUIRED@
Requires:         libgnome >= 2.14.0
Requires:         gnome-desktop >= 2.9.91
Requires:         libgnomeui >= 2.6.0
Requires:         librsvg2 >= 2.0.1
Requires:         libxml2 >= 2.4.7
BuildRequires:    esound-devel >= @ESOUND_REQUIRED@
BuildRequires:    bonobo-activation-devel >= 2.1.0
BuildRequires:    eel2-devel >= 2.21.90
BuildRequires:    glib2-devel >= 2.15.6
BuildRequires:    gnome-vfs2-devel >= @GNOME_VFS_REQUIRED@
BuildRequires:    gtk2-devel >= 2.11.6
BuildRequires:    libart_lgpl-devel >= @ART_REQUIRED@
BuildRequires:    libbonobo-devel >= 2.1.0
BuildRequires:    libbonoboui-devel >= @BONOBO_UI_REQUIRED@
BuildRequires:    libgnome-devel >= 2.14.0
BuildRequires:    gnome-desktop-devel >= 2.9.91
BuildRequires:    libgnomeui-devel >= 2.6.0
BuildRequires:    librsvg2-devel >= 2.0.1
BuildRequires:    libxml2-devel >= 2.4.7


%description
Nautilus integrates access to files, applications, media, Internet-based
resources and the Web.  Nautilus delivers a dynamic and rich user
experience.  Nautilus is an free software project developed under the
GNU General Public License and is a core component of the GNOME desktop
project.

%package devel
Summary:          Libraries and include files for developing Nautilus components
Group:            Development/Libraries
Requires:         %name = %{version}
Requires:         pkgconfig >= 0.8
Requires:         esound >= @ESOUND_REQUIRED@
Requires:         esound-devel >= @ESOUND_REQUIRED@
Requires:         bonobo-activation >= 2.1.0
Requires:         bonobo-activation-devel >= 2.1.0
Requires:         eel2 >= 2.21.90
Requires:         eel2-devel >= 2.21.90
Requires:         glib2 >= 2.15.6
Requires:         glib2-devel >= 2.15.6
Requires:         gnome-vfs2 >= @GNOME_VFS_REQUIRED@
Requires:         gnome-vfs2-devel >= @GNOME_VFS_REQUIRED@
Requires:         gtk2 >= 2.11.6
Requires:         gtk2-devel >= 2.11.6
Requires:         libart_lgpl >= @ART_REQUIRED@
Requires:         libart_lgpl-devel >= @ART_REQUIRED@
Requires:         libbonobo >= 2.1.0
Requires:         libbonobo-devel >= 2.1.0
Requires:         libbonoboui >= @BONOBO_UI_REQUIRED@
Requires:         libbonoboui-devel >= @BONOBO_UI_REQUIRED@
Requires:         libgnome >= 2.14.0
Requires:         libgnome-devel >= 2.14.0
Requires:         gnome-desktop >= 2.9.91
Requires:         gnome-desktop-devel >= 2.9.91
Requires:         libgnomeui >= 2.6.0
Requires:         libgnomeui-devel >= 2.6.0
Requires:         librsvg2 >= 2.0.1
Requires:         librsvg2-devel >= 2.0.1
Requires:         libxml2 >= 2.4.7
Requires:         libxml2-devel >= 2.4.7


%description devel
This package provides the necessary development libraries and include
files to allow you to develop Nautilus components.

%prep
%setup -q
mv ltmain.sh ltmain.sh.orig
sed s/relink=yes/relink=no/ ltmain.sh.orig > ltmain.sh

%build
%configure
make %{?_smp_mflags}

%install
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT
%makeinstall

%find_lang %name

%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%post

%postun
/sbin/ldconfig
/usr/bin/scrollkeeper-update

%files -f %name.lang
%defattr(-, root, root)
%doc AUTHORS COPYING COPYING-DOCS COPYING.LIB ChangeLog INSTALL
%doc MAINTAINERS NEWS README THANKS TODO
#%doc %{_datadir}/gnome/help/*
%{_sysconfdir}/gconf/*
%{_sysconfdir}/X11/serverconfig/.directory
%{_sysconfdir}/X11/starthere/*
%{_sysconfdir}/X11/starthere/.directory
%{_sysconfdir}/X11/sysconfig/.directory
%{_bindir}/*
%{_libdir}/*.so*
%{_libdir}/bonobo/libnautilus-*-view.so
%{_libdir}/bonobo/servers/*
%{_libexecdir}/*
%{_datadir}/applications/*
%{_datadir}/gnome-2.0/ui/*
%{_datadir}/idl/*
%{_datadir}/nautilus
%{_datadir}/locale/*/*
%{_datadir}/pixmaps/*
%{_datadir}/gnome/network/*
#%doc %{_datadir}/omf/nautilus/*


%files devel
%defattr(-, root, root)
%{_includedir}/*
%{_libdir}/*.so
%{_libdir}/*a
%{_libdir}/pkgconfig/*


%changelog
* Tue Mar 12 2003 Neil Weisenfeld <weisen@ai.mit.edu>
- Added various file targets: gnome/network, the libexecdir stuff,
  locale, gconf (do we need to register this somehow?)
- Fixed nautilus-tree-view to come from lib/bonobo instead of lib/
- Actually, now pick up nautilus-*-view.so from lib/bonobo
- Commented out omf and gnome/help documentation.  (should this be
  coming from somewhere else?)
- killed dependence on ltmain-fixed.sh as we're "fixing" it in the %prep

* Tue Mar 05 2002 Chris Chabot <chabotc@reviewboard.com>
- Fixed file list
- ltmain.sh fix
- fixed last format issues
- converted to .spec.in

* Mon Feb 04 2002 Roy-Magne Mo  <rmo@sunnmore.net>
- Fixed lang

* Sun Feb 03 2002 Roy-Magne Mo  <rmo@sunnmore.net>
- Added buildrequires

* Sat Jan 19 2002 Chris Chabot <chabotc@reviewboard.com>
- Minor changes & restored macro's
- Major cleanups
- Moved versions to defines
- Since entry below hints at removing sugested, extras, remove their headers

* Fri Jan 18 2002  <builder@localhost.localdomain>
- removed some unnecessary %defines
- got rid of suggested and extras packages, to make life easier
- re-write %files section from scratch
- made setup quiet
- fixed %post and %postun scripts
- removed -k from make lines (it's silly for RPMs)

* Tue Apr 17 2001 Gregory Leblanc <gleblanc@grego1.cu-portland.edu>
- Added BuildRequires lines
- Changed Source to point to ftp.gnome.org instead of just the tarball name
- Moved %description sections closer to their %package sections
- Moved %changelog to the end, where so that it's not in the way
- Changed configure and make install options to allow moving of
libraries, includes, binaries more easily
- Removed hard-coded paths (don't define %prefix or %docdir)
- replace %{prefix}/bin with %{_bindir}
- replace %{prefix}/share with %{_datadir}
- replace %{prefix}/lib with %{_libdir}
- replace %{prefix}/include with %{_includedir}

* Tue Oct 10 2000 Robin Slomkowski <rslomkow@eazel.com>
- removed obsoletes from sub packages and added mozilla and trilobite
subpackages

* Wed Apr 26 2000 Ramiro Estrugo <ramiro@eazel.com>
- created this thing
