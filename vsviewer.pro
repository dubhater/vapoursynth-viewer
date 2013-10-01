HEADERS = src/vsviewer.h \
          src/preview.h \
          src/textedit.h \
          src/pythonhighlighter.h

SOURCES = src/vsviewer.cpp \
          src/preview.cpp \
          src/textedit.cpp \
          src/pythonhighlighter.cpp \
          src/main.cpp

unix {
   CONFIG += link_pkgconfig
   PKGCONFIG += vapoursynth-script
}
