%bcond_with wayland

Name: liblivebox
Summary: Livebox development library
Version: 0.7.4
Release: 1
Group: HomeTF/Livebox
License: Flora
Source0: %{name}-%{version}.tar.gz
Source1001: %{name}.manifest
BuildRequires: cmake, gettext-tools, coreutils
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(livebox-service)
BuildRequires: pkgconfig(provider)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(elementary)

%if %{with wayland}
%else
BuildRequires: pkgconfig(x11)
%endif

%if "%{sec_product_feature_livebox}" == "0"
ExclusiveArch:
%endif

%description
Library for the development of a livebox

%package devel
Summary: Livebox development library (dev)
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
Header & Package configuration files for livebox development

%prep
%setup -q
cp %{SOURCE1001} .

%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif

%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS="${CFLAGS} -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="${CXXFLAGS} -DTIZEN_ENGINEER_MODE"
export FFLAGS="${FFLAGS} -DTIZEN_ENGINEER_MODE"
%endif

%if "%{_repository}" == "wearable"
export WEARABLE=On
export MOBILE=Off
%elseif "%{_repository}" == "mobile"
export WEARABLE=Off
export MOBILE=On
%endif

%if %{with wayland}
export WAYLAND_SUPPORT=On
export X11_SUPPORT=Off
%else
export WAYLAND_SUPPORT=Off
export X11_SUPPORT=On
%endif

%cmake . -DWAYLAND_SUPPORT=${WAYLAND_SUPPORT} -DX11_SUPPORT=${X11_SUPPORT} -DMOBILE=${MOBILE} -DWEARABLE=${WEARABLE}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/%{_datarootdir}/license

%post -n liblivebox -p /sbin/ldconfig
%postun -n liblivebox -p /sbin/ldconfig

%files -n liblivebox
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/*.so*
%{_datarootdir}/license/*

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/livebox/livebox.h
%{_libdir}/pkgconfig/*.pc

# End of a file
