NULL =

LDADD = $(LIBPCAP_LIBS) $(NETWORK_ANALYZER_LIBS) -lm

AM_CFLAGS = $(LIBPCAP_CFLAGS) $(NETWORK_ANALYZER_CFLAGS)

bin_PROGRAMS = gnome-network-analyzer

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

dbusconfdir = $(sysconfdir)/dbus-1/system.d
dbusconf_DATA = org.gnome.NetworkAnalyzer.conf

-include $(top_srcdir)/git.mk
