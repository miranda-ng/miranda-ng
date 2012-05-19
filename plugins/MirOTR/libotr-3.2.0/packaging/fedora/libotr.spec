Summary: Off-The-Record Messaging library and toolkit
Name: libotr
Version: 3.1.0
Release: 1%{?dist}
License: GPL and LGPL
Group: System Environment/Libraries
Source0: http://otr.cypherpunks.ca/%{name}-%{version}.tar.gz
Url: http://otr.cypherpunks.ca/
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Provides: libotr-toolkit = %{version}
Obsoletes: libotr-toolkit < %{version}
Requires: libgcrypt >= 1.2.0, 
BuildRequires: libgcrypt-devel >= 1.2.0, libgpg-error-devel 

%description
Off-the-Record Messaging Library and Toolkit
This is a library and toolkit which implements Off-the-Record (OTR) Messaging.
OTR allows you to have private conversations over IM by providing Encryption,
Authentication, Deniability and Perfect forward secrecy.

%package devel
Summary: Development library and include files for libotr
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}, libgcrypt-devel >= 1.2.0

%description devel

The devel package contains the libotr library and the include files

%prep
%setup -q

%build

%configure --with-pic --disable-rpath
make %{?_smp_mflags} all

%install
rm -rf $RPM_BUILD_ROOT
make \
	DESTDIR=$RPM_BUILD_ROOT \
	LIBINSTDIR=%{_libdir} \
	install
rm -rf $RPM_BUILD_ROOT%{_libdir}/*.la

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc AUTHORS README COPYING COPYING.LIB NEWS Protocol*
%{_libdir}/libotr.so.*
%{_bindir}/*
%{_mandir}/man1/*

%files devel
%defattr(-,root,root,-)
%doc ChangeLog
%{_libdir}/libotr.so
%{_libdir}/libotr.a
%dir %{_includedir}/libotr
%{_includedir}/libotr/*
%{_datadir}/aclocal/*


%changelog
* Thu Jul 26 2007 Paul Wouters <paul@cypherpunks.ca> 3.1.0-preview2
- Upgraded to current version

* Mon Oct 17 2005 Paul Wouters <paul@cypherpunks.ca> 3.0.0-1
- Minor change to allow for new documentation files. Fixed Requires:

* Sat Jun 19 2005 Paul Wouters <paul@cypherpunks.ca>
- Fixed defattr, groups, description and duplicate files in devel

* Fri Jun 17 2005 Tom "spot" Callaway <tcallawa@redhat.com>
- reworked for Fedora Extras

* Tue May  3 2005 Ian Goldberg <ian@cypherpunks.ca>
- Bumped version number to 2.0.2

* Wed Feb 16 2005 Ian Goldberg <ian@cypherpunks.ca>
- Bumped version number to 2.0.1

* Tue Feb  8 2005 Ian Goldberg <ian@cypherpunks.ca>
- Bumped version number to 2.0.0

* Wed Feb  2 2005 Ian Goldberg <ian@cypherpunks.ca>
- Added libotr.m4 to the devel package
- Bumped version number to 1.99.0

* Wed Jan 19 2005 Paul Wouters <paul@cypherpunks.ca>
- Updated spec file for the gaim-otr libotr split

* Tue Dec 21 2004 Ian Goldberg <otr@cypherpunks.ca>
- Bumped to version 1.0.2.

* Fri Dec 17 2004 Paul Wouters <paul@cypherpunks.ca>
- instll fix for x86_64

* Sun Dec 12 2004 Ian Goldberg <otr@cypherpunks.ca>
- Bumped to version 1.0.0.

* Fri Dec 10 2004 Ian Goldberg <otr@cypherpunks.ca>
- Bumped to version 0.9.9rc2. 

* Thu Dec  9 2004 Ian Goldberg <otr@cypherpunks.ca>
- Added CFLAGS to "make all", removed DESTDIR

* Wed Dec  8 2004 Ian Goldberg <otr@cypherpunks.ca>
- Bumped to version 0.9.9rc1. 

* Fri Dec  3 2004 Ian Goldberg <otr@cypherpunks.ca>
- Bumped to version 0.9.1. 

* Wed Dec  1 2004 Paul Wouters <paul@cypherpunks.ca>
- Bumped to version 0.9.0. 
- Fixed install for tools and cos
- Added Obsoletes: target for otr-plugin so rpm-Uhv gaim-otr removes it.

* Mon Nov 22 2004 Ian Goldberg <otr@cypherpunks.ca>
- Bumped version to 0.8.1
* Sun Nov 21 2004 Paul Wouters <paul@cypherpunks.ca>
- Initial version

