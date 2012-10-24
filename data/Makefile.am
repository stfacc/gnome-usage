NULL =

desktopdir       = $(datadir)/applications
desktop_in_files = gnome-usage.desktop.in
desktop_DATA     = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	$(desktop_in_files)		\
	$(NULL)

DISTCLEANFILES = \
	$(desktop_DATA)		\
	$(NULL)

-include $(top_srcdir)/git.mk
