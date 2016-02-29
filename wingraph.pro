include(qtvars.pri)

# includes

macx:INCLUDEPATH += /usr/include/malloc

# destination directory and name

DESTDIR=.
TARGET = qwingraph

# resources

RESOURCES += wingraph.qrc

# compilation flags

# since we can not modify some source files, just suppress warnings
win32: {
  CFLAGS+=-wd4013 -wd4090
  QMAKE_CXXFLAGS += $${CFLAGS}
  QMAKE_CFLAGS += $${CFLAGS}
}
else: {
  CFLAGS+=-Wall \
          -fdiagnostics-show-option \
          -Wno-format \
          -Wno-parentheses \
          -Wno-sign-compare \
          -Wno-uninitialized \
          -Wno-unused-variable \
          -Wno-unused-function
  QMAKE_CXXFLAGS_WARN_ON += $${CFLAGS}
  QMAKE_CFLAGS_WARN_ON += $${CFLAGS}
}

#
# project files
#

FORMS += grprintpagesdlg.ui SearchDialog.ui

HEADERS       += mainwindow.h \
                  alloc.h drawstr.h grammar.h timelim.h \
                  draw.h fisheye.h grprint.h options.h timing.h ytab.h \
                  drawchr.h folding.h infobox.h \
                  drawlib.h globals.h main.h steps.h usrsignal.h \
                  grprintpagesdlg.h Searchdlg.h
SOURCES       += wingraph32.cpp \
                mainwindow.cpp \
               alloc.c \
               draw.c \
               drawlib.c \
               drawstr.c \
               draw_wrapper.c \
               fisheye.c \
               folding.c \
               grprint.c \
               grprint2.c \
               grprintstr.c \
               infobox.c \
               lexyy.c \
               vcg_main.c \
               options.c \
               prepare.c \
               step0.c \
               step1.c \
               step2.c \
               step3.c \
               step4.c \
               timelim.c \
               tree.c \
               usrsignal.c \
               ytab.c \
               grprintpagesdlg.cpp \
			   Searchdlg.cpp
