NULL =
MAINTAINERCLEANFILES = *.stamp

AM_CPPFLAGS = \
	-DGETTEXT_PACKAGE=\""$(GETTEXT_PACKAGE)"\" 		\
	-DGNOMELOCALEDIR=\""$(localedir)"\" 			\
	$(NULL)

BUILT_SOURCES = usage-resources.c

bin_PROGRAMS = gnome-usage

VALA_SOURCES = \
	main.vala			\
	usage-application.vala		\
	usage-window.vala		\
	usage-view.vala			\
	usage-cpu-view.vala		\
	usage-memory-view.vala		\
	usage-storage-list.vala		\
	usage-storage-view.vala		\
	usage-network-view.vala		\
	usage-battery-view.vala		\
	usage-system-monitor.vala	\
	usage-element-list.vala		\
	usage-graph-widget.vala		\
	usage-widgets.vala		\
	$(NULL)

gnome_usage_SOURCES = \
	config.vapi		\
	glibtop.vapi		\
	$(VALA_SOURCES)		\
	$(BUILT_SOURCES)	\
	$(NULL)

gnome_usage_VALAFLAGS = \
	--target-glib=2.38 \
	--pkg gtk+-3.0	\
	--pkg posix	\
	--gresources=$(srcdir)/resources/usage.gresource.xml \
	$(NULL)

usage-resources.c: $(srcdir)/resources/usage.gresource.xml $(shell glib-compile-resources --generate-dependencies --sourcedir=$(srcdir)/resources $(srcdir)/resources/usage.gresource.xml)
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(srcdir)/resources --generate-source $<

AM_CFLAGS = \
	$(USAGE_CFLAGS)		\
	$(NULL)

gnome_usage_LDFLAGS = -export-dynamic
gnome_usage_LDADD = \
	-lm				\
	$(USAGE_LIBS)			\
	$(NULL)

EXTRA_DIST = \
	usage.gresource.xml \
	usage-element-row.ui \
	usage-storage-row.ui \
	usage-storage-view.ui \
	usage-storage-list.ui \
	usage-window.ui \
	$(NULL)

MAINTAINERCLEANFILES += $(VALA_SOURCES:.vala=.c)

-include $(top_srcdir)/git.mk
