NULL =
MAINTAINERCLEANFILES = *.stamp

AM_CPPFLAGS = \
	-DPREFIX=\""$(prefix)"\" 				\
	-DGETTEXT_PACKAGE=\""$(GETTEXT_PACKAGE)"\" 		\
	-DSYSCONFDIR=\""$(sysconfdir)"\"			\
	-DLIBDIR=\""$(libdir)"\"				\
	-DDATADIR=\""$(datadir)"\"				\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" 	\
	$(NULL)

bin_PROGRAMS = gnome-usage

VALA_SOURCES = \
	main.vala			\
	usage-application.vala		\
	usage-window.vala		\
	usage-view.vala			\
	usage-cpu-view.vala		\
	usage-memory-view.vala		\
	usage-storage-view.vala		\
	usage-network-view.vala		\
	usage-battery-view.vala		\
	usage-system-monitor.vala	\
	usage-element-widget.vala	\
	usage-graph-widget.vala		\
	usage-widgets.vala		\
	$(NULL)

gnome_usage_SOURCES = \
	config.vapi		\
	glibtop.vapi		\
	$(VALA_SOURCES)		\
	$(NULL)

gnome_usage_VALAFLAGS = \
	--pkg gtk+-3.0	\
	--pkg posix	\
	$(NULL)

AM_CFLAGS = \
	$(USAGE_CFLAGS)		\
	$(NULL)

gnome_usage_LDADD = \
	-lm				\
	$(USAGE_LIBS)			\
	$(NULL)

MAINTAINERCLEANFILES += $(VALA_SOURCES:.vala=.c)

-include $(top_srcdir)/git.mk
