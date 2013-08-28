
// TODO:
//    - popups for error messages
//    - scrollbars
//    - user-selected output index
//    - script_open should not touch the GUI


#include <QtCore>
#include <QtGui>
//#include <QDebug>
//#include <QtGlobal>


#include <vapoursynth/VSScript.h>

#include "preview.h"


void Preview::ui_init() {
   imglabel = new QLabel(this);

   QHBoxLayout *hLayout = new QHBoxLayout;
   QVBoxLayout *vLayout = new QVBoxLayout;

   hLayout->addStretch(1);
   hLayout->addWidget(imglabel);
   hLayout->addStretch(1);

   vLayout->addStretch(1);
   vLayout->addLayout(hLayout);
   vLayout->addStretch(1);

   frameLabel = new QLabel();
   vLayout->addWidget(frameLabel);

   setLayout(vLayout);

   resize(640, 480);

   setWindowTitle("vsviewer - Preview");

   setSizeGripEnabled(true);
}


void Preview::vs_init() {
   QString fail;
   
   init = vsscript_init();
   if (!init) {
      fail = "Failed to initialise VapourSynth environment.";
      goto done;
   }

   vsapi = vsscript_getVSApi();
   if (!vsapi) {
      fail = "Failed to get VapourSynth API pointer.";
      vsscript_finalize();
      goto done;
   }

   return;

done:
   imglabel->setText(fail);
}


Preview::Preview(QWidget *parent)
 : QDialog(parent),
   init(0),
   vsapi(NULL),
   se(NULL),
   node(NULL),
   currentFrameRef(NULL),
   currentFrameNum(-1),
   lastFrameVisited(-1)
{
   setAttribute(Qt::WA_DeleteOnClose, false);

   ui_init();

   vs_init();
}


Preview::~Preview() {
   if (init) {
      script_close();
      vsscript_finalize();
   }
}


void Preview::openFile(QString script, QString script_name) {
   closeFile();

   int ret = script_open(script, script_name);
   if (!ret) {
      // TODO: error message
      qDebug("script_open failed\n");
      return;
   }

   if (lastFrameVisited > -1) {
      seek(lastFrameVisited);
   } else {
      seek(0);
   }
}


void Preview::closeFile() {
   if (currentFrameNum > -1) {
      lastFrameVisited = currentFrameNum;
   }
   currentFrameNum = -1;

   imglabel->setPixmap(QPixmap());

   script_close();

   frameLabel->setText("");
}


int Preview::script_open(QString script, QString script_name) {
   QString fail;

   int ret = vsscript_evaluateScript(&se, script.toUtf8().constData(), script_name.toUtf8().constData(), efSetWorkingDir);
   // This one returns non-zero on failure...
   if (ret) {
      fail = "Script evaluation failed:\n";
      fail += QString::fromUtf8(vsscript_getError(se));
      imglabel->setText(fail);
      vsscript_freeScript(se);
      se = NULL;
      return 0;
   }

   node = vsscript_getOutput(se, 0);
   if (!node) {
      fail = "Failed to retrieve output node. Invalid index specified?";
      imglabel->setText(fail);
      vsscript_freeScript(se);
      se = NULL;
      return 0;
   }

   // Convert to RGB, swscale willing.
   VSCore *core = vsscript_getCore(se);
   VSPlugin *resizePlugin = vsapi->getPluginNs("resize", core);
   VSMap *resizeArgs = vsapi->createMap();
   vsapi->propSetNode(resizeArgs, "clip", node, paReplace);
   vsapi->freeNode(node);
   vsapi->propSetInt(resizeArgs, "format", /*pfRGB24*/ pfCompatBGR32, paReplace);

   VSMap *resizeRet = vsapi->invoke(resizePlugin, "Bicubic", resizeArgs);
   vsapi->freeMap(resizeArgs);

   const char *resizeError = vsapi->getError(resizeRet);
   if (resizeError) {
      fail = "Failed to convert the script's output to COMPATBGR32:\n";
      fail += QString::fromUtf8(resizeError);
      vsapi->freeMap(resizeRet);
      goto done;
   }

   node = vsapi->propGetNode(resizeRet, "clip", 0, NULL);
   if (!node) {
      fail = "Failed to convert the script's output to COMPATBGR32:\n";
      fail += "resize.Bicubic() did not return a valid clip.";
      vsapi->freeMap(resizeRet);
      goto done;
   }

   vsapi->freeMap(resizeRet);

   vi = vsapi->getVideoInfo(node);

   return 1;

done:
   imglabel->setText(fail);
   vsscript_freeScript(se);
   se = NULL;
   return 0;
}


int Preview::script_close() {
   if (currentFrameRef) {
      vsapi->freeFrame(currentFrameRef);
      currentFrameRef = NULL;
   }

   if (node) {
      vsapi->freeNode(node);
      node = NULL;
   }

   if (se) {
      vsscript_freeScript(se);
      se = NULL;
   }

   return 1;
}


void Preview::seek(int n) {
   if (!node) {
      return;
   }

   if (n == currentFrameNum) {
      return;
   }

   if (n < 0) {
      n = 0;
   }

   if (vi->numFrames && n >= vi->numFrames) {
      n = vi->numFrames - 1;
   }

   char err[100];
   const VSFrameRef *frame = vsapi->getFrame(n, node, err, sizeof(err));
   if (!frame) {
      // FIXME: Shit failed. We should display err somewhere.
      // Not in imglabel, because this is not a fatal error.
      qDebug("Failed to retrieve frame %d:\n%s\n", n, err);
      return;
   }

   int width = vsapi->getFrameWidth(frame, 0);
   int height = vsapi->getFrameHeight(frame, 0);
   int stride = vsapi->getStride(frame, 0);
   const uint8_t *srcp = vsapi->getReadPtr(frame, 0);

   QImage img(srcp, width, height, stride, QImage::Format_RGB32);

   imglabel->setPixmap(QPixmap::fromImage(img));

   vsapi->freeFrame(currentFrameRef);

   currentFrameRef = frame;
   currentFrameNum = n;

   frameLabel->setText(QString("Frame: %1").arg(n));
}


void Preview::keyPressEvent(QKeyEvent * event)
{
   int key = event->key();
   Qt::KeyboardModifiers modifs = event->modifiers();

   if ((modifs & Qt::ShiftModifier) ||
       (modifs & Qt::ControlModifier) ||
       (modifs & Qt::AltModifier) ||
       (modifs & Qt::MetaModifier)) {
      QDialog::keyPressEvent(event);
      return;
   }

   switch (key) {
      case Qt::Key_Left:
         seek(currentFrameNum - 1);
         break;
      case Qt::Key_Right:
         seek(currentFrameNum + 1);
         break;
      case Qt::Key_PageDown:
         seek(currentFrameNum - 10);
         break;
      case Qt::Key_PageUp:
         seek(currentFrameNum + 10);
         break;
      case Qt::Key_Home:
         seek(0);
         break;
      case Qt::Key_End:
         if (vi->numFrames) {
            seek(vi->numFrames - 1);
         }
         break;
      case Qt::Key_G:
         gotoFrame();
         break;
      default:
         QDialog::keyPressEvent(event);
         break;
   }
}


void Preview::gotoFrame() {
   bool okay;
   int n = QInputDialog::getInt(this, "Go to frame", "Enter frame number:", currentFrameNum, 0, vi->numFrames-1, 1, &okay);
   if (okay) {
      seek(n);
   }
}