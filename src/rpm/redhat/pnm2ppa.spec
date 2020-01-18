%define _mandir /usr/man

Name: pnm2ppa
Summary: Drivers for printing to HP PPA printers
Serial: 1
Obsoletes: ppa
Obsoletes: pbm2ppa
Version: 1.04
Release: 1
URL: http://sourceforge.net/projects/pnm2ppa 
Source: http://download.sourceforge.net/pnm2ppa/pnm2ppa-%{version}.tar.gz
Source1: http://www.httptech.com/ppa/files/ppa-0.8.6.tar.gz
Patch2: pbm2ppa-20000205.diff
Patch3: pnm2ppa-redhat.patch
Copyright: GPL
Group: Applications/Publishing
Buildroot: /var/tmp/pnm2ppa-buildroot
%define topdir pnm2ppa-%{version}

%description
pnm2ppa is a color driver for HP PPA host-based printers such
as the HP710C, 712C, 720C, 722C, 820Cse, 820Cxi, 1000Cse, and 1000Cxi.
It accepts ghostscript output in PNM formats, and sends it to the printer
in PPA format.  The  older (black only) driver pbm2ppa is also included.

Install pnm2ppa if you have a PPA printer and need to print.

For further information, see the pnm2ppa project homepage at 
http://sourceforge.net/projects/pnm2ppa .

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n %{topdir}

#pbm2ppa source
%setup -T -D -a 1 -n %{topdir}
%patch2 -p0
%patch3 -p1 -b .rh

%build
make 
cd pbm2ppa-0.8.6
make


%install
install -d $RPM_BUILD_ROOT/usr/bin
install -d $RPM_BUILD_ROOT/etc
install -d $RPM_BUILD_ROOT%{_mandir}/man1
make INSTALLDIR=$RPM_BUILD_ROOT/usr/bin CONFDIR=$RPM_BUILD_ROOT/etc \
    MANDIR=$RPM_BUILD_ROOT%{_mandir}/man1 install 
install -m 0755 utils/Linux/detect_ppa $RPM_BUILD_ROOT/usr/bin/
install -m 0755 utils/Linux/test_ppa $RPM_BUILD_ROOT/usr/bin/
install -m 0755 pbm2ppa-0.8.6/pbm2ppa  $RPM_BUILD_ROOT/usr/bin/
install -m 0755 pbm2ppa-0.8.6/pbmtpg   $RPM_BUILD_ROOT/usr/bin/
install -m 0644 pbm2ppa-0.8.6/pbm2ppa.conf   $RPM_BUILD_ROOT/etc
install -m 0644 pbm2ppa-0.8.6/pbm2ppa.1   $RPM_BUILD_ROOT%{_mandir}/man1

chmod 644 docs/en/LICENSE
mkdir -p pbm2ppa
for file in CALIBRATION CREDITS INSTALL INSTALL-MORE LICENSE README ; do
  install -m 0644 pbm2ppa-0.8.6/$file pbm2ppa/$file
done

#---omit this section from Official RedHat/Mandrake releases!--------------
install -d $RPM_BUILD_ROOT/usr/lib/rhs/rhs-printfilters
install -m 0755 rhs-printfilters/ppaprint $RPM_BUILD_ROOT/usr/lib/rhs/rhs-printfilters/ppaprint-latest
install -m 0644 rhs-printfilters/printerdb-ppa $RPM_BUILD_ROOT/usr/lib/rhs/rhs-printfilters
install -m 0644 rhs-printfilters/README.ppa $RPM_BUILD_ROOT/usr/lib/rhs/rhs-printfilters
#-------------------------------------------------


%clean
rm -rf $RPM_BUILD_ROOT 

%files 
%defattr(-,root,root)
%doc docs/en/CREDITS docs/en/INSTALL docs/en/LICENSE docs/en/README
%doc docs/en/RELEASE-NOTES docs/en/TODO
%doc docs/en/INSTALL.REDHAT.txt docs/en/COLOR.txt docs/en/CALIBRATION.txt
%doc docs/en/INSTALL.REDHAT.html docs/en/COLOR.html docs/en/CALIBRATION.html
%doc test.ps
%doc pbm2ppa
/usr/bin/pnm2ppa
/usr/bin/pbm2ppa
/usr/bin/pbmtpg
/usr/bin/calibrate_ppa
/usr/bin/test_ppa
/usr/bin/detect_ppa
%{_mandir}/man1/pnm2ppa.1*
%{_mandir}/man1/pbm2ppa.1*
%config /etc/pnm2ppa.conf
%config /etc/pbm2ppa.conf


#---omit from  RedHat--------------------
%doc docs/en/INSTALL.MANDRAKE.html docs/en/INSTALL.MANDRAKE.txt
#------omit from Official RedHat/Mandrake releases!
/usr/lib/rhs/rhs-printfilters/ppaprint-latest
/usr/lib/rhs/rhs-printfilters/printerdb-ppa
/usr/lib/rhs/rhs-printfilters/README.ppa
#-----------------------------------------



%changelog
* Tue Nov 24 2000 Duncan Haldane <duncan_haldane@users.sourceforge.net>
- updated for 1.04 release.

* Tue Oct 24 2000 Duncan Haldane <duncan_haldane@users.sourceforge.net>
- updated for 1.01 release; calibrate renamed to calibrate_ppa.

* Tue Jul 11 2000 Duncan Haldane <duncan_haldane@users.sourceforge.net>
- updated for 1.0 release.

* Sun Apr 09 2000 <duncan_haldane@users.sourceforge.net>
- added optional updated rhs-printfilter  files

* Thu Feb 10 2000 Bill Nottingham <notting@redhat.com>
- adopt upstream package

* Sun Feb 6 2000 <duncan_haldane@users.sourceforge.net>
- new pnm2ppa release,  and add pbm2ppa driver.

* Thu Jan 7 2000 <duncan_haldane@users.sourceforge.net>
- created rpm



