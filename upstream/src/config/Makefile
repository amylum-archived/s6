it: command include library library.so


s6-config.h: stupid s6-version.stupid s6-config-header s6-config-nosp s6-config-sp.stupid s6-config-footer
	{ \
	  cat s6-config-header ; \
	  ./stupid < s6-version.stupid ; \
	  if test -f flag-slashpackage ; then ./stupid < s6-config-sp.stupid ; else cat s6-config-nosp ; fi ; \
	  cat s6-config-footer ; \
	} > s6-config.h


command:

include: s6-config.h

library:

library.so:
