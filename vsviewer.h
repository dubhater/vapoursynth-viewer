#ifndef VSVIEWER_H
#define VSVIEWER_H

#include <QtCore>
#include <QtGui>

#include <vapoursynth/VSScript.h>

#include "preview.h"

class VSViewer : public QMainWindow {
   Q_OBJECT

   public slots:
      void onFileOpen();
      void onFileReload();
      void onFilePreview();

   public:
      VSViewer();

   private:
      QAction *fileOpen;
      QAction *fileReload;
      QAction *filePreview;

      QPlainTextEdit *textEdit;
      Preview *preview;


      QString scriptName;

      void ui_init();

      void openFile(QString name);
      void set_title(QString script_name, bool modified);
};

#endif
