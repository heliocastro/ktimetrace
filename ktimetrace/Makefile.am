####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = ktimetrace

## INCLUDES were found outside kdevelop specific part

ktimetrace_SOURCES = ktracecolordialog.cpp zoomdialog.cpp kttsettings.cpp bufferdialog.cpp tracewidget.cpp view.cpp data.cpp dialog.cpp main.cpp adc.cpp ktimetrace.cpp acqbuffer.cpp 
ktimetrace_LDADD   = -lz -lcomedi -lpthread $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)


EXTRA_DIST = acqbuffer.cpp acqbuffer.h ktimetrace.cpp adc.cpp adc.h main.cpp resource.h ktimetrace.h dialog.cpp dialog.h data.cpp data.h view.cpp view.h tracewidget.cpp tracewidget.h bufferdialog.cpp bufferdialog.h kttsettings.cpp kttsettings.h zoomdialog.cpp zoomdialog.h ktracecolordialog.cpp ktracecolordialog.h 

####### kdevelop will overwrite this part!!! (end)############
# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

# the library search path. 
ktimetrace_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDE´s XML GUI builing
#rcdir = $(kde_datadir)/ktimetrace
#rc_DATA = ktimetraceui.rc

#WARNING: if you use a ui.rc file above, use:

# messages: rc.cpp

# instead of 

# messages:

messages: 
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/ktimetrace.pot; \
	fi

