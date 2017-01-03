Name:       sensord
Summary:    Sensor daemon
Version:    2.0.10
Release:    1
Group:      System/Sensor Framework
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    sensord.service
Source2:    sensord_command.socket
Source3:    sensord_event.socket

BuildRequires:  cmake
BuildRequires:  libattr-devel
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(libsystemd-daemon)
BuildRequires:  pkgconfig(cynara-creds-socket)
BuildRequires:  pkgconfig(cynara-client)
BuildRequires:  pkgconfig(cynara-session)

Provides:   %{name}-profile_tv = %{version}-%{release}
# For backward compatibility
Provides:   libsensord = %{version}-%{release}

%description
Sensor daemon

%package    genuine
Summary:    Genuine Sensor Framework service daemon and shared library
Requires:   %{name} = %{version}-%{release}
Provides:   %{name}-profile_mobile = %{version}-%{release}
Provides:   %{name}-profile_wearable = %{version}-%{release}
Provides:   %{name}-profile_ivi = %{version}-%{release}
Provides:   %{name}-profile_common = %{version}-%{release}

%description genuine
Binary replacement for sensord.
This genuine sensord package contains actually working shared library
of the sensor internal APIs and the sensor service daemon.
If you want to keep using %{name} after uninstalling this, you need to reinstall %{name}.

%package    devel
Summary:    Internal Sensor API (Development)
Group:      System/Development
Requires:   %{name} = %{version}-%{release}

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

# This dummy package will be removed later.
%package -n libsensord-devel
Summary:    Dummy package for backward compatibility
Requires:   sensord-devel

%description -n libsensord-devel
Some packages require libsensord-devel directly, and it causes local gbs build failures
with the old build snapshots. This is a temporal solution to handle such cases.

%prep
%setup -q
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DLIBDIR=%{_libdir} \
        -DMAJORVER=${MAJORVER} -DFULLVER=%{version}

%build
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}%{_unitdir}

install -m 0644 %SOURCE1 %{buildroot}%{_unitdir}
install -m 0644 %SOURCE2 %{buildroot}%{_unitdir}
install -m 0644 %SOURCE3 %{buildroot}%{_unitdir}

%install_service multi-user.target.wants sensord.service
%install_service sockets.target.wants sensord_event.socket
%install_service sockets.target.wants sensord_command.socket

ln -s libsensor.so.2 %{buildroot}%{_libdir}/libsensor.so.1

%post
/sbin/ldconfig

%files
%manifest packaging/sensord.manifest
%{_libdir}/libsensor.so.*
%license LICENSE.APLv2

%post   genuine
pushd %{_libdir}
ln -sf libsensor-genuine.so.%{version} libsensor.so.%{version}
chsmack -a "_" libsensor.so.%{version}
popd
/sbin/ldconfig

%preun  genuine
echo "You need to reinstall %{name}, if you need to keep using the APIs after uinstalling this."

%files  genuine
%manifest packaging/sensord.manifest
%{_libdir}/libsensord-shared.so
%{_libdir}/libsensor-genuine.so.*
%{_bindir}/sensord
%{_unitdir}/sensord.service
%{_unitdir}/sensord_command.socket
%{_unitdir}/sensord_event.socket
%{_unitdir}/multi-user.target.wants/sensord.service
%{_unitdir}/sockets.target.wants/sensord_command.socket
%{_unitdir}/sockets.target.wants/sensord_event.socket

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

%files -n libsensord-devel
%license LICENSE.APLv2
