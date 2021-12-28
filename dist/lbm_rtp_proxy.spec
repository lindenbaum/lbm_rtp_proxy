Name:		 lbm_rtp_proxy

####################################
# Specify the current version here:
Version:	 1.0.2
Release:	 1
####################################

Summary:	 Lindenbaum RTP proxy/relay kernel module
Group:		 System Environment/Daemons
BuildArch:	 noarch
License:	 GPLv3
URL:		 https://github.com/lindenbaum/lbm_rtp_proxy
Source0:	 https://github.com/lindenbaum/lbm_rtp_proxy/archive/%{version}.tar.gz
BuildRequires:	 redhat-rpm-config
Requires:	 gcc make
Requires:        kernel-devel
Requires(post):	 dkms
Requires(preun): dkms

%description
A kernel module to proxy/relay and route RTP/RTCP traffic.

%prep
export LBM_RTP_PROXY_VERSION=%{version}-%{release}
%setup -q

%install
mkdir -p %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}
install -D -p -m644 Makefile %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/Makefile
install -D -p -m644 src/checksum.c %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/checksum.c
install -D -p -m644 src/checksum.h %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/checksum.h
install -D -p -m644 src/command.c %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/command.c
install -D -p -m644 src/command.h %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/command.h
install -D -p -m644 src/config.c %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/config.c
install -D -p -m644 src/config.h %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/config.h
install -D -p -m644 src/debug.c %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/debug.c
install -D -p -m644 src/debug.h %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/debug.h
install -D -p -m644 src/mangle.c %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/mangle.c
install -D -p -m644 src/mangle.h %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/mangle.h
install -D -p -m644 src/module.c %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/module.c
install -D -p -m644 src/module.h %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/module.h
install -D -p -m644 src/procfs.c %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/procfs.c
install -D -p -m644 src/procfs.h %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/procfs.h
install -D -p -m644 src/rewrite.c %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/rewrite.c
install -D -p -m644 src/rewrite.h %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/rewrite.h
install -D -p -m644 src/rtcp_packet.h %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/rtcp_packet.h
install -D -p -m644 src/rtp_packet.h %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/rtp_packet.h
install -D -p -m644 src/table.c %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/table.c
install -D -p -m644 src/table.h %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/src/table.h
install -D -p -m644 dist/dkms.conf.in %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/dkms.conf
sed -i -e "s/__VSN__/%{version}-%{release}/g" %{buildroot}%{_usrsrc}/%{name}-%{version}-%{release}/dkms.conf
install -D -p -m644 dist/lbm_rtp_proxy.conf %{buildroot}%{_sysconfdir}/modules-load.d/lbm_rtp_proxy.conf

%post
dkms add -m %{name} -v %{version}-%{release} --rpm_safe_upgrade &&
dkms build -m %{name} -v %{version}-%{release} --rpm_safe_upgrade &&
dkms install -m %{name} -v %{version}-%{release} --rpm_safe_upgrade --force
true

%preun
dkms uninstall -m %{name} -v %{version}-%{release} --rpm_safe_upgrade
dkms remove -m %{name} -v %{version}-%{release} --rpm_safe_upgrade --all
rmmod %{name}
true

%files
%{_usrsrc}/%{name}-%{version}-%{release}/
%{_sysconfdir}/modules-load.d/lbm_rtp_proxy.conf

%changelog
