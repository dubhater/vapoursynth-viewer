HEADERS = vsviewer.h \
          preview.h \
          textedit.h \
          pythonhighlighter.h

SOURCES = vsviewer.cpp \
          preview.cpp \
          textedit.cpp \
          pythonhighlighter.cpp \
          main.cpp

unix {
   CONFIG += link_pkgconfig
   PKGCONFIG += vapoursynth-script
}
