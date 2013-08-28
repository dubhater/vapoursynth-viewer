#ifndef VSVIEWER_H
#define VSVIEWER_H

#include <QtCore>
#include <QtGui>

#include <vapoursynth/VSScript.h>

#include "preview.h"
#include "textedit.h"

class VSViewer : public QMainWindow {
   Q_OBJECT

   public slots:
      void onFileNew();
      void onFileOpen();
      void onFileSave();
      void onFileSaveAs();
      void onFileReload();
      void onFilePreview();

   public:
      VSViewer();

   private:
      QAction *fileNew;
      QAction *fileOpen;
      QAction *fileSave;
      QAction *fileSaveAs;
      QAction *fileReload;
      QAction *filePreview;

      TextEdit *textEdit;
      Preview *preview;


      QString scriptName;
      bool scriptExists;

      void ui_init();

      void openFile(QString name);
      void saveFile(QString name);

      void set_title(QString script_name, bool modified);

      void errmsg(QString msg);
};

#endif
