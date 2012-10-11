NULL =

LDADD = $(LIBPCAP_LIBS) $(NETWORK_ANALYZER_LIBS) -lm

AM_CFLAGS = $(LIBPCAP_CFLAGS) $(NETWORK_ANALYZER_CFLAGS)

libexec_PROGRAMS = gnome-network-analyzer

c_sources = \
	na-application.c	\
	na-connection.c		\
	na-conninode.c		\
	na-inodeproc.c		\
	na-local-addresses.c	\
	na-pcap.c		\
	na-packet.c		\
	na-process.c		\
	$(NULL)

gnome_network_analyzer_SOURCES = \
	$(c_sources)		\
	$(c_sources:.c=.h)	\
	main.c			\
	$(NULL)

# D-BUS service file
%.service: %.service.in ../config.log
	$(AM_V_GEN) $(SED) -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

servicedir = $(prefix)/share/dbus-1/system-services
service_in_file = org.gnome.NetworkAnalyzer.service.in
service_DATA = org.gnome.NetworkAnalyzer.service

dbus_conf_file = org.gnome.NetworkAnalyzer.conf
dbusconfdir = $(sysconfdir)/dbus-1/system.d
dbusconf_DATA = $(dbus_conf_file)

EXTRA_DIST = \
	$(service_in_file)	\
	$(dbus_conf_file)	\
	$(NULL)

DISTCLEANFILES = \
	$(service_DATA)	\
	$(NULL)

-include $(top_srcdir)/git.mk
