#ifndef PREVIEW_H
#define PREVIEW_H

#include <QtCore>
#include <QtGui>

#include <vapoursynth/VSScript.h>

class Preview : public QDialog {
   Q_OBJECT

//   public slots:

   public:
      Preview(QWidget *parent = 0);
      ~Preview();

      void openFile(QString script, QString script_name);

   private:
      QLabel *imglabel;
      QLabel *frameLabel;

      int init;
      const VSAPI *vsapi;
      VSScript *se;
      VSNodeRef *node;
      const VSVideoInfo *vi;

      const VSFrameRef *currentFrameRef;
      int currentFrameNum;
      int lastFrameVisited;

      void ui_init();
      void vs_init();

      void closeFile();

      int script_open(QString script, QString script_name);
      int script_close();

      void seek(int n);

      void keyPressEvent(QKeyEvent * event);

      void gotoFrame();
};

#endif
