Name: buteo-sync-plugins-qt5
Version: 0.7.0
Release: 1
Summary: Synchronization plugins
Group: System/Libraries
URL: https://github.com/nemomobile/buteo-sync-plugins
License: LGPLv2.1
Source0: %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(Qt5Core)
# check if qtcontacts still uses qpixmap
#BuildRequires: pkgconfig(Qt5Gui)
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(Qt5Contacts)
BuildRequires: pkgconfig(Qt5Versit)
BuildRequires: pkgconfig(Qt5Sql)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(Qt5Test)
BuildRequires: pkgconfig(Qt5SystemInfo)
BuildRequires: pkgconfig(openobex)
BuildRequires: pkgconfig(accounts-qt5)
BuildRequires: pkgconfig(libsignon-qt5)
BuildRequires: pkgconfig(buteosyncml5)
BuildRequires: pkgconfig(buteosyncfw5) >= 0.6.24
BuildRequires: pkgconfig(qtcontacts-sqlite-qt5-extensions)
BuildRequires: pkgconfig(libmkcal-qt5)
BuildRequires: pkgconfig(libkcalcoren-qt5)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: doxygen

%description
%{summary}.

%files
%defattr(-,root,root,-)
%config %{_sysconfdir}/buteo/xml/*.xml
%config %{_sysconfdir}/buteo/profiles/server/*.xml
%config %{_sysconfdir}/buteo/profiles/client/*.xml
%config %{_sysconfdir}/buteo/profiles/storage/*.xml
%config %{_sysconfdir}/buteo/profiles/service/*.xml
%config %{_sysconfdir}/buteo/profiles/sync/bt_template.xml
%config %{_sysconfdir}/buteo/plugins/syncmlserver/*.xml
%{_libdir}/buteo-plugins-qt5/*.so
%{_libdir}/*.so.*

%package devel
Requires: %{name} = %{version}-%{release}
Summary: Development files for %{name}
Group: Development/Libraries

%description devel
%{summary}.

%files devel
%defattr(-,root,root,-)
%{_includedir}/*
%{_libdir}/*.so
%{_libdir}/*.prl
%{_libdir}/pkgconfig/*.pc

%package doc
Summary: Documentation for %{name}
Group: Documentation

%description doc
%{summary}.

%files doc
%defattr(-,root,root,-)
%{_docdir}/sync-app-doc


%package tests
Summary: Tests for %{name}
Group: System/Libraries
Requires: %{name} = %{version}-%{release}

%description tests
%{summary}.

%files tests
%defattr(-,root,root,-)
/opt/tests/buteo-sync-plugins


%package -n buteo-service-memotoo
Summary: Memotoo service description for Buteo SyncML
Group: System/Libraries
Requires: %{name} = %{version}

%description -n buteo-service-memotoo
%{summary}.

%files -n buteo-service-memotoo
%defattr(-,root,root,-)
%config %{_sysconfdir}/buteo/profiles/sync/memotoo.com.xml

%prep
%setup -q


%build
%qmake5
make %{?_smp_mflags}


%install
make INSTALL_ROOT=%{buildroot} install
rm -f %{buildroot}/%{_sysconfdir}/buteo/profiles/sync/switch.xml


%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
