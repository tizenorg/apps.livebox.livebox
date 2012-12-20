Name: liblivebox
Summary: Library for the development of a livebox 
Version: 0.1.6
Release: 1
Group: main/app
License: Flora License
Source0: %{name}-%{version}.tar.gz
BuildRequires: cmake, gettext-tools
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(livebox-service)
BuildRequires: pkgconfig(provider)

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
mkdir -p %{buildroot}/usr/share/license

%post

%files -n liblivebox
%manifest liblivebox.manifest
%defattr(-,root,root,-)
/usr/lib/*.so*
/usr/share/license/*

%files devel
%defattr(-,root,root,-)
/usr/include/livebox/livebox.h
/usr/share/doc/livebox/livebox_PG.h
/usr/lib/pkgconfig/*.pc
