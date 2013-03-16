Name: liblivebox
Summary: Library for the development of a livebox 
Version: 0.1.16
Release: 1
Group: main/app
License: Flora License
Source0: %{name}-%{version}.tar.gz
BuildRequires: cmake, gettext-tools, coreutils
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(livebox-service)
BuildRequires: pkgconfig(provider)
BuildRequires: pkgconfig(elementary)

%description
Livebox development library

%package devel
Summary: Files for livebox development.
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
Livebox development library (dev)

%prep
%setup -q

%build
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/%{_datarootdir}/license

%post

%files -n liblivebox
%manifest liblivebox.manifest
%defattr(-,root,root,-)
%{_libdir}/*.so*
%{_datarootdir}/license/*

%files devel
%defattr(-,root,root,-)
%{_includedir}/livebox/livebox.h
%{_datarootdir}/doc/livebox/livebox_PG.h
%{_libdir}/pkgconfig/*.pc

# End of a file
