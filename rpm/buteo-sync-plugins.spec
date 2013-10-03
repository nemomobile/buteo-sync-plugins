Name: buteo-sync-plugins
Version: 0.7.0
Release: 1
Summary: Synchronization plugins
Group: System/Libraries
URL: https://github.com/nemomobile/buteo-sync-plugins
License: LGPLv2.1
Source0: %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(QtCore)
BuildRequires: pkgconfig(QtNetwork)
BuildRequires: pkgconfig(QtContacts)
BuildRequires: pkgconfig(QtSystemInfo)
BuildRequires: pkgconfig(openobex)
BuildRequires: pkgconfig(accounts-qt)
BuildRequires: pkgconfig(libsignon-qt)
BuildRequires: pkgconfig(buteosyncml)
BuildRequires: pkgconfig(buteosyncfw)
BuildRequires: pkgconfig(libmkcal)
BuildRequires: doxygen

%description
%{summary}.

%files
%defattr(-,root,root,-)
%config %{_sysconfdir}/buteo/xml/*.xml
%config %{_sysconfdir}/buteo/profiles/client/*.xml
%config %{_sysconfdir}/buteo/profiles/storage/*.xml
%config %{_sysconfdir}/buteo/profiles/sync/bt_template.xml
%{_libdir}/buteo-plugins/*.so
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
qmake
make %{?_smp_mflags}


%install
make INSTALL_ROOT=%{buildroot} install
rm -f %{buildroot}/%{_sysconfdir}/buteo/profiles/sync/switch.xml


%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
