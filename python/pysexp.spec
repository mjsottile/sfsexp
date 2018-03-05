Summary: Python binding for libsexp in cmtools
Name: pysexp
Version: 1.0
Release: 1
Vendor: Linux Labs http://www.linuxlabs.com
License: GPL
Group: Development/Libraries/Python
Source: %{name}.tar.gz
Packager: Steven James <pyro@linuxlabs.com>
BuildRoot: /var/tmp/%{name}
Requires: sexpr >= 0.3.2

%description
libsexp is a fast efficient parser for S-expressions in C.
pysexp is the Python binding.

%prep
%setup -q -n %{name}

%build
make

%install
make DESTDIR=$RPM_BUILD_ROOT install

%clean 
rm -rf $RPM_BUILD_ROOT

%post 

%preun

%files
%defattr(-,root,root)
/usr/lib/python2.2/site-packages/pysexp.so

%changelog
* Sat Mar 13 2004 Steven James <pyro@linuxlabs.com>
- Fixed memory leak, made test program much tougher
* Thu Mar 11 2004 Steven James <pyro@linuxlabs.com>
- Initial release
