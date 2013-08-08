#ifndef VSVIEWER_H
#define VSVIEWER_H

#include <QtCore>
#include <QtGui>

#include <vapoursynth/VSScript.h>

class VSViewer : public QMainWindow {
   Q_OBJECT

   public slots:
      void onFileOpen();
      void onFileClose();
      void onFileReload();
      void onGotoFrame();

   public:
      VSViewer();
      ~VSViewer();

   private:
      QAction *fileOpen;
      QAction *fileClose;
      QAction *fileReload;
      QAction *fileGotoFrame;
      QLabel *imglabel;
      QLabel *frameLabel;

      int init;
      const VSAPI *vsapi;
      VSScript *se;
      VSNodeRef *node;

      const VSFrameRef *currentFrameRef;
      int currentFrameNum;
      int lastFrameVisited;

      QString scriptName;

      void ui_init();
      void vs_init();

      void openFile(QString name);
      int script_open(QString script_name);
      int script_close();

      void seek(int n);

      void keyPressEvent(QKeyEvent * event);
};

#endif
