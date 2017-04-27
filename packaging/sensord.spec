Name:       sensord
Summary:    Sensor daemon
Version:    4.0.0
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
BuildRequires:  pkgconfig(libsystemd-daemon)
BuildRequires:  pkgconfig(cynara-creds-socket)
BuildRequires:  pkgconfig(cynara-client)
BuildRequires:  pkgconfig(cynara-session)

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

%description dummy
This package provides the dummy library of the sensor internal API.
Installing %{name} replaces this dummy library with the actually functional library.


%package    devel
Summary:    Internal Sensor API (Development)
Group:      System/Development
Requires:   %{name}-dummy = %{version}-%{release}

%description devel
Internal Sensor API (Development)


%package -n sensor-hal-devel
Summary:    Sensord HAL interface
Group:      System/Development

%description -n sensor-hal-devel
Sensord HAL interface


%package -n sensor-test
Summary:    Sensord library
Group:      System/Testing

%description -n sensor-test
Sensor functional testing


%prep
%setup -q

%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`

%cmake . -DMAJORVER=${MAJORVER} -DFULLVER=%{version}
make %{?_smp_mflags}

%install
%make_install

mkdir -p %{buildroot}%{_unitdir}

install -m 0644 %SOURCE1 %{buildroot}%{_unitdir}
install -m 0644 %SOURCE2 %{buildroot}%{_unitdir}

%install_service multi-user.target.wants sensord.service
%install_service sockets.target.wants sensord.socket

ln -s libsensor.so.%{version} %{buildroot}/%{_libdir}/libsensor.so.2
ln -s libsensor.so.%{version} %{buildroot}/%{_libdir}/libsensor.so.1

%post
pushd %{_libdir}
ln -sf libsensor-genuine.so.%{version} libsensor.so.%{version}
chsmack -a "_" libsensor.so.%{version}
popd
/sbin/ldconfig

%preun
echo "You need to reinstall %{name}-dummy to keep using the APIs after uninstalling this."

%files
%manifest packaging/sensord.manifest
%{_libdir}/libsensor-genuine.so.*
%{_libdir}/libsensord-shared.so
%{_libdir}/sensor/fusion/libsensor-fusion.so
%{_bindir}/sensord
%{_unitdir}/sensord.service
%{_unitdir}/sensord.socket
%{_unitdir}/multi-user.target.wants/sensord.service
%{_unitdir}/sockets.target.wants/sensord.socket
%license LICENSE.APLv2


%post   dummy
/sbin/ldconfig

%files  dummy
%manifest packaging/sensord.manifest
%{_libdir}/libsensor.so.*
%license LICENSE.APLv2


%files  devel
%manifest packaging/sensord.manifest
%exclude %{_includedir}/sensor/sensor_hal.h
%{_includedir}/sensor/*.h
%{_libdir}/libsensor.so
%{_libdir}/pkgconfig/sensor.pc


%files -n sensor-hal-devel
%manifest packaging/sensord.manifest
%{_includedir}/sensor/sensor_hal*.h


%files -n sensor-test
%{_bindir}/sensorctl
