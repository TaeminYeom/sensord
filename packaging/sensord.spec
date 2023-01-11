Name:       sensord
Summary:    Sensor daemon
Version:    4.0.54
Release:    1
Group:      System/Sensor Framework
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    sensord.service
Source2:    sensord.socket

BuildRequires:  cmake
BuildRequires:  libattr-devel
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(libsystemd)
BuildRequires:  pkgconfig(libsyscommon)
BuildRequires:  pkgconfig(libtzplatform-config)
BuildRequires:  pkgconfig(cynara-creds-socket)
BuildRequires:  pkgconfig(cynara-client)
BuildRequires:  pkgconfig(cynara-session)
BuildRequires:  pkgconfig(hal-api-sensor)
BuildRequires:  pkgconfig(hal-api-common)

Requires:   %{name}-dummy = %{version}-%{release}
Provides:   %{name}-profile_mobile = %{version}-%{release}
Provides:   %{name}-profile_wearable = %{version}-%{release}
Provides:   %{name}-profile_ivi = %{version}-%{release}
Provides:   %{name}-profile_common = %{version}-%{release}
%global __provides_exclude ^.*-genuine\\.so.*$

%description
This package provides the fully functional internal API library and the service daemon
of the Sensor Framework. The library replaces the dummy library installed by %{name}-dummy.


%package    dummy
Summary:    Sensor Framework 'dummy' library
Provides:   %{name}-profile_tv = %{version}-%{release}
# To support old-snapshot-based package builds
Provides:   libsensor.so.2
# For targets which uses only dummy
# Prevent to install sensord by providing libsensor.so.<major version>
Provides:   libsensor.so.%(echo %{version} | cut -d'.' -f1)


%description dummy
This package provides the dummy library of the sensor internal API.
Installing %{name} replaces this dummy library with the actually functional library.


%package    devel
Summary:    Internal Sensor API (Development)
Group:      System/Development
Requires:   %{name}-dummy = %{version}-%{release}
# To support old-snapshot-based package builds
#Provides:   libsensord-devel

%description devel
Internal Sensor API (Development)

%package -n sensor-test
Summary:    Sensord library
Group:      System/Testing

%description -n sensor-test
Sensor functional testing

%prep
%setup -q

%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`

%cmake . -DMAJORVER=${MAJORVER} -DFULLVER=%{version} -DCMAKE_HAL_LIBDIR_PREFIX=%{_hal_libdir}
make %{?_smp_mflags}

%install
%make_install

mkdir -p %{buildroot}%{_unitdir}

install -m 0644 %SOURCE1 %{buildroot}%{_unitdir}
install -m 0644 %SOURCE2 %{buildroot}%{_unitdir}

mkdir -p %{buildroot}%{_sysconfdir}/sensord
install -m 644 conf/auto_rotation.conf   %{buildroot}/etc/sensord/auto_rotation.conf

%install_service multi-user.target.wants sensord.service
%install_service sockets.target.wants sensord.socket

ln -s libsensor.so.%{version} %{buildroot}/%{_libdir}/libsensor.so.2
ln -s libsensor.so.%{version} %{buildroot}/%{_libdir}/libsensor.so.1

%post
pushd %{_libdir}
chsmack -a "_" libsensor.so.%{version}
popd
/sbin/ldconfig

%preun
echo "You need to reinstall %{name}-dummy to keep using the APIs after uninstalling this."

%files
%manifest packaging/sensord.manifest
%{_libdir}/libsensor.so.%{version}
%{_libdir}/libsensord-shared.so
%{_libdir}/sensor/fusion/libsensor-fusion.so
%{_libdir}/sensor/physical/libsensor-physical.so
%{_bindir}/sensord
%{_unitdir}/sensord.service
%{_unitdir}/sensord.socket
%{_unitdir}/multi-user.target.wants/sensord.service
%{_unitdir}/sockets.target.wants/sensord.socket
%config %{_sysconfdir}/sensord/auto_rotation.conf
%license LICENSE.APLv2


%post   dummy
pushd %{_libdir}
ln -sf libsensor-dummy.so libsensor.so.%{version}
chsmack -a "_" libsensor.so.%{version}
popd
/sbin/ldconfig

%files  dummy
%manifest packaging/sensord.manifest
%exclude %{_libdir}/libsensor.so.%{version}
%{_libdir}/libsensor.so.*
%{_libdir}/libsensor-dummy.so
%license LICENSE.APLv2


%files  devel
%manifest packaging/sensord.manifest
%exclude %{_includedir}/sensor/sensor_hal.h
%{_includedir}/sensor/*.h
%{_libdir}/libsensor.so
%{_libdir}/pkgconfig/sensor.pc


%files -n sensor-test
%{_bindir}/sensorctl


# Dummy packages for Tizen 3.0.
# When building other packages on Tizen 3.0, after building sensord first,
# some dependency conflicts may occur. These dummy packages may fix such
# dependency issues.
%package -n libsensord
Summary:    Dummy libsensord
Requires:   sensord-dummy
Group:      System/Sensor Framework

%description -n libsensord
Dummy libsensord

%files -n libsensord

%package -n libsensord-devel
Summary:    Dummy libsensord-devel
Requires:   sensord-devel
Group:      System/Sensor Framework

%description -n libsensord-devel
Dummy libsensord-devel

%files -n libsensord-devel
