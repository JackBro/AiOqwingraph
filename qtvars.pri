QT += core \
    gui
QT_NAMESPACE = QT

DEFINES += NO_OBSOLETE_FUNCS

win32 {
    DEFINES += __NT__ \
        _CRT_SECURE_NO_WARNINGS \
        __VC__
    SYSNAME = win
    COMPILER_NAME = vc
    CFLAGS += /GS \
              /wd4946 \  # reinterpret_cast used between related classes
              /wd4826    # Conversion from 'ptr32' to 'int64' is sign-extended. This may cause unexpected runtime behavior.
}
!mac:unix {
    DEFINES += __LINUX__ _FORTIFY_SOURCE=0
    SYSNAME = linux
    COMPILER_NAME = gcc
    # avoid linking GLIBC_2.11 symbols (longjmp_chk)
    CFLAGS += -D_FORTIFY_SOURCE=0
}
mac { # scope name must be 'mac'
    DEFINES += __MAC__
    SYSNAME = mac
    COMPILER_NAME = gcc
    CONFIG += macx # as it turned out, macx is also required :(
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.3
    QMAKE_INFO_PLIST = Info.plist

    # we compile and link a 32-bit application:
    !x64 {
      CFLAGS = -m32
      QMAKE_LFLAGS_DEBUG += -m32
      QMAKE_LFLAGS_RELEASE += -m32
    }
}

CONFIG(debug, debug|release) {
  mac|unix {
      CFLAGS += -fvisibility=hidden
      QMAKE_LFLAGS_DEBUG +=
      QMAKE_LFLAGS_RELEASE += -s
  }
  DEFINES += _DEBUG
}
else {
  OPTSUF=_opt
}

x64 {
    DEFINES += __EA64__ __X64__
    SUFF64 = 64x
    ADRSIZE = 64
    TARGET_PROCESSOR_NAME = x64
}
else {
  TARGET_PROCESSOR_NAME = x86
  is64 {
    DEFINES += __EA64__
    SUFF64 = 64
    ADRSIZE = 64
  }
  else {
    ADRSIZE = 32
  }
}

BINDIR=$${TARGET_PROCESSOR_NAME}_$${SYSNAME}_$${COMPILER_NAME}$${OPTSUF}
SYSDIR=$${TARGET_PROCESSOR_NAME}_$${SYSNAME}_$${COMPILER_NAME}_$${ADRSIZE}$${OPTSUF}
OBJDIR=obj/$${SYSDIR}/

linux:LIBS += -z \
    defs \
    -z \
    origin \
    -z \
    now \
    -Wl,-rpath=\'\$\$ORIGIN\'

MOC_DIR = $${OBJDIR}
OBJECTS_DIR = $${OBJDIR}
RCC_DIR = $${OBJDIR}
UI_DIR = $${OBJDIR}

# suppress warnings
!win32:QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-sign-compare
