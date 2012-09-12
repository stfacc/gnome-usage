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

gnome_usage_SOURCES = \
	config.vapi		\
	main.vala		\
	usage-application.vala	\
	usage-window.vala	\
	$(NULL)

gnome_usage_VALAFLAGS = \
	--pkg gtk+-3.0	\
	$(NULL)

AM_CFLAGS = \
	$(USAGE_CFLAGS)		\
	$(NULL)

gnome_usage_LDADD = \
	$(USAGE_LIBS)			\
	$(NULL)

MAINTAINERCLEANFILES += $(gnome_usage_SOURCES:.vala=.c)

-include $(top_srcdir)/git.mk
