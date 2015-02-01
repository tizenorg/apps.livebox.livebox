%bcond_with wayland

Name: libdynamicbox
Summary: Dynamicbox development library
Version: 1.0.0
Release: 1
Group: HomeTF/DynamicBox
License: Flora
Source0: %{name}-%{version}.tar.gz
Source1001: %{name}.manifest
BuildRequires: cmake, gettext-tools, coreutils
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(dynamicbox_service)
BuildRequires: pkgconfig(dynamicbox_provider)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(livebox-service)
BuildRequires: model-build-features

%if %{with wayland}
%else
BuildRequires: pkgconfig(x11)
%endif

%if "%{model_build_feature_livebox}" == "0"
ExclusiveArch:
%endif

%description
Library for the development of a dynamicbox 

%package devel
Summary: Dynamic Box development library (dev)
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
Header & Package configuration files for dynamicbox development

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

%if %{with wayland}
export WAYLAND_SUPPORT=On
export X11_SUPPORT=Off
%else
export WAYLAND_SUPPORT=Off
export X11_SUPPORT=On
%endif

%cmake . -DWAYLAND_SUPPORT=${WAYLAND_SUPPORT} -DX11_SUPPORT=${X11_SUPPORT}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/%{_datarootdir}/license

%post -n %{name} -p /sbin/ldconfig
%postun -n %{name} -p /sbin/ldconfig

%files -n %{name}
%defattr(-,root,root,-)
%manifest %{name}.manifest
%{_libdir}/libdynamicbox.so*
%{_datarootdir}/license/libdynamicbox

%files devel
%defattr(-,root,root,-)
%manifest %{name}.manifest
%{_includedir}/dynamicbox/dynamicbox.h
%{_includedir}/dynamicbox/internal/dynamicbox.h
%{_libdir}/pkgconfig/dynamicbox.pc

#################################################
# liblivebox (for old version)
%package -n liblivebox
Summary: Library for developing the dynamicbox (old version)
Group: HomeTF/Dynamicbox
License: Flora
Requires: libdynamicbox

%description -n liblivebox
Provider APIs to develop the dynamicbox. (old version)

%package -n liblivebox-devel
Summary: Header & package configuration files to support development of the dynamicbox (old version)
Group: Development/Libraries
Requires: liblivebox

%description -n liblivebox-devel
Dynamicbox development library (dev) (old version)

%files -n liblivebox
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/liblivebox.so*
%{_datarootdir}/license/liblivebox

%files -n liblivebox-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/livebox/livebox.h
%{_includedir}/livebox/livebox_product.h
%{_libdir}/pkgconfig/livebox.pc

# End of a file
