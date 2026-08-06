Name:       harbour-wallpaper-loop

Summary:    Cycle folder images as Ambience wallpaper
Version:    1.3.0
Release:    1
License:    BSD-3-Clause
URL:        https://github.com/coagulalabs/harbour-wallpaper-loop
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  desktop-file-utils

%description
Wallpaper Loop cycles through images in a folder and applies each one
as your Sailfish Ambience wallpaper on a timer. Choose interval, order,
and scaling. Native Silica UI with cover actions for next/previous.


%prep
%setup -q -n %{name}-%{version}

%build

%qmake5 

%make_build


%install
%qmake5_install


desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

install -D -m 0644 rpm/harbour-wallpaper-loop.profile \
  %{buildroot}%{_sysconfdir}/sailjail/applications/harbour-wallpaper-loop.profile

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_sysconfdir}/sailjail/applications/%{name}.profile
%{_datadir}/icons/hicolor/*/apps/%{name}.png
