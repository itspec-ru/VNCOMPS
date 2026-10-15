Name:       com.e1c.AddInNative
Summary:    1C AddInNative
Version:    0.1
Release:    1
License:    BSD-3-Clause
URL:        https://1c.ru
Source0:    %{name}-%{version}.tar.bz2

Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(auroraapp)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)

%description
Шаблон внешней компоненты для ОС Аврора

%prep
%autosetup

%build
%qmake5
%make_build
