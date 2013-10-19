
// TODO:
//    - user-selected output index
//    - emit refreshWanted signal when F5 is pressed
//    - install a message handler that uses a message box to display vapoursynth's qFatal messages


#include <QtCore>
#include <QtGui>
//#include <QDebug>
//#include <QtGlobal>


#include <VSScript.h>

#include "preview.h"


void Preview::ui_init() {
   imglabel = new QLabel;
   imglabel->setAlignment(Qt::AlignCenter);

   QScrollArea *scrollArea = new QScrollArea;
   scrollArea->setFrameShape(QFrame::NoFrame);
   scrollArea->setFocusPolicy(Qt::NoFocus);
   scrollArea->setAlignment(Qt::AlignCenter);
   scrollArea->setWidgetResizable(true);

   scrollArea->setWidget(imglabel);

   QVBoxLayout *vLayout = new QVBoxLayout;

   vLayout->addWidget(scrollArea);

   frameLabel = new QLabel();
   vLayout->addWidget(frameLabel);

   setLayout(vLayout);

   resize(640, 480);

   setWindowTitle("vsviewer - Preview");

   setSizeGripEnabled(true);
}


void VS_CC messageHandler(int msgType, const char *msg, void *userData) {
   Preview *previewWindow = (Preview *)userData;
   previewWindow->messageHandler(msgType, msg);
}


void Preview::messageHandler(int msgType, const char *msg) const {
   QString message;

   switch (msgType) {
      case mtDebug:
         message = "Debug";
         break;
      case mtWarning:
         message = "Warning";
         break;
      case mtCritical:
         message = "Critical";
         break;
      case mtFatal:
         message = "Fatal";
         break;
   }

   message += " message from VapourSynth:\n";
   message += msg;

   if (msgType == mtFatal) {
      message += "\n\nGoodbye.";
   }

   QMessageBox box;
   box.setText(message);
   box.exec();
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

   //vsapi->setMessageHandler(::messageHandler, (void *)this);

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

   QString ret = script_open(script, script_name);
   if (!ret.isNull()) {
      errmsg(ret);
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


QString Preview::script_open(QString script, QString script_name) {
   QString fail;

   int ret = vsscript_evaluateScript(&se, script.toUtf8().constData(), script_name.toUtf8().constData(), efSetWorkingDir);
   // This one returns non-zero on failure...
   if (ret) {
      fail = "Script evaluation failed:\n";
      fail += QString::fromUtf8(vsscript_getError(se));
      vsscript_freeScript(se);
      se = NULL;
      return fail;
   }

   node = vsscript_getOutput(se, 0);
   if (!node) {
      fail = "Failed to retrieve output node. Invalid index specified?";
      vsscript_freeScript(se);
      se = NULL;
      return fail;
   }

   // Convert to RGB, swscale willing.
   VSCore *core = vsscript_getCore(se);
   VSPlugin *resizePlugin = vsapi->getPluginByNs("resize", core);
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

   return fail;

done:
   vsscript_freeScript(se);
   se = NULL;
   return fail;
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
      errmsg(QString("Failed to retrieve frame %1:\n%2").arg(n).arg(QString::fromUtf8(err)));
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


void Preview::errmsg(QString msg) {
   QMessageBox box(this);
   box.setText(msg);
   box.exec();
}
