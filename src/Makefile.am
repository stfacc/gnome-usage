NULL =
MAINTAINERCLEANFILES = *.stamp

INCLUDES = \
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
	usage-topbar.vala		\
	usage-view.vala			\
	usage-cpu-view.vala		\
	usage-memory-view.vala		\
	usage-storage-view.vala		\
	usage-battery-view.vala		\
	usage-system-monitor.vala	\
	$(NULL)

gnome_usage_SOURCES = \
	config.vapi			\
	egglistbox.vapi			\
	glibtop.vapi			\
	egg-list-box.c			\
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
	$(USAGE_LIBS)			\
	$(NULL)

MAINTAINERCLEANFILES += $(VALA_SOURCES:.vala=.c)

-include $(top_srcdir)/git.mk
