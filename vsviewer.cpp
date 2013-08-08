


#include <QtCore>
#include <QtGui>
//#include <QDebug>
//#include <QtGlobal>

//#include <cstring>

#include <vapoursynth/VSScript.h>

#include "vsviewer.h"


void VSViewer::ui_init() {
   // Set up the menus.
   fileOpen = new QAction("&Open script...", this);
   fileOpen->setShortcut(QKeySequence("Ctrl+O"));

   fileClose = new QAction("&Close script", this);
   fileClose->setShortcut(QKeySequence("Ctrl+W"));
   fileClose->setEnabled(false);

   fileReload = new QAction("&Reload script", this);
   fileReload->setShortcut(QKeySequence("Ctrl+R"));
   fileReload->setEnabled(false);

   fileGotoFrame = new QAction("&Go to frame...", this);
   fileGotoFrame->setShortcut(QKeySequence("Ctrl+G"));
   fileGotoFrame->setEnabled(false);

   QAction *fileQuit = new QAction("&Quit", this);
   fileQuit->setShortcut(QKeySequence("Ctrl+Q"));

   connect(fileOpen, SIGNAL(triggered()),
               this, SLOT(onFileOpen()));
   connect(fileClose, SIGNAL(triggered()),
                this, SLOT(onFileClose()));
   connect(fileReload, SIGNAL(triggered()),
                 this, SLOT(onFileReload()));
   connect(fileGotoFrame, SIGNAL(triggered()),
                    this, SLOT(onGotoFrame()));
   connect(fileQuit, SIGNAL(triggered()),
               this, SLOT(close()));


   QMenu *fileMenu = menuBar()->addMenu("&File");
   fileMenu->addAction(fileOpen);
   fileMenu->addAction(fileClose);
   fileMenu->addAction(fileReload);
   fileMenu->addSeparator();
   fileMenu->addAction(fileGotoFrame);
   fileMenu->addSeparator();
   fileMenu->addAction(fileQuit);


   imglabel = new QLabel(this);

   QHBoxLayout *hLayout = new QHBoxLayout;
   QVBoxLayout *vLayout = new QVBoxLayout;

   hLayout->addStretch(1);
   hLayout->addWidget(imglabel);
   hLayout->addStretch(1);

   vLayout->addStretch(1);
   vLayout->addLayout(hLayout);
   vLayout->addStretch(1);

   QWidget *centralWidget = new QWidget();
   centralWidget->setLayout(vLayout);

   setCentralWidget(centralWidget);

   frameLabel = new QLabel();
   
   statusBar()->addWidget(frameLabel);

   resize(800, 600);
}


void VSViewer::vs_init() {
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
   fileOpen->setEnabled(false);
   fileClose->setEnabled(false);
}


VSViewer::VSViewer()
 : init(0),
   vsapi(NULL),
   se(NULL),
   node(NULL),
   currentFrameRef(NULL),
   currentFrameNum(-1),
   lastFrameVisited(-1)
{
   ui_init();

   vs_init();
}


VSViewer::~VSViewer() {
   if (init) {
      onFileClose();
      vsscript_finalize();
   }
}


void VSViewer::onFileOpen() {
   QString name = QFileDialog::getOpenFileName(this, "Open VapourSynth script", QString(), QString(), 0, QFileDialog::DontUseNativeDialog);
   if (name.isNull()) {
      return;
   }

   scriptName = name;

   openFile(name);

}


void VSViewer::openFile(QString name) {
   onFileClose();

   int ret = script_open(name);
   fileClose->setEnabled(ret);
   fileReload->setEnabled(ret);
   fileGotoFrame->setEnabled(ret);
   if (!ret) {
      // TODO: error message
      return;
   }

   if (lastFrameVisited > -1) {
      seek(lastFrameVisited);
   } else {
      seek(0);
   }
}


void VSViewer::onFileClose() {
   if (currentFrameNum > -1) {
      lastFrameVisited = currentFrameNum;
   }
   currentFrameNum = -1;

   imglabel->setPixmap(QPixmap());

   script_close();

   frameLabel->setText("");

   fileClose->setEnabled(false);
   fileReload->setEnabled(false);
   fileGotoFrame->setEnabled(false);
}


void VSViewer::onFileReload() {
   if (!scriptName.isNull()) {
      openFile(scriptName);
   }
}


int VSViewer::script_open(QString script_name) {
   QString fail;

   int ret = vsscript_evaluateFile(&se, script_name.toUtf8(), efSetWorkingDir);
   // This one returns non-zero on failure...
   if (ret) {
      fail = "Script evaluation failed:\n";
      fail += QString::fromUtf8(vsscript_getError(se));
      imglabel->setText(fail);
      vsscript_freeScript(se);
      return 0;
   }

   node = vsscript_getOutput(se, 0); // TODO: user-selected output index
   if (!node) {
      fail = "Failed to retrieve output node. Invalid index specified?";
      imglabel->setText(fail);
      vsscript_freeScript(se);
      return 0;
   }

   // Convert to RGB, swscale willing.
   VSCore *core = vsscript_getCore(se);
   VSPlugin *resizePlugin = vsapi->getPluginNs("resize", core);
   VSMap *resizeArgs = vsapi->createMap();
   vsapi->propSetNode(resizeArgs, "clip", node, paReplace);
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
   return 1;

done:
   imglabel->setText(fail);
   vsscript_freeScript(se);
   return 0;
}


int VSViewer::script_close() {
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


void VSViewer::seek(int n) {
   if (!node) {
      return;
   }

   if (n == currentFrameNum) {
      return;
   }

   const VSVideoInfo *vi = vsapi->getVideoInfo(node);

   if (n < 0) {
      n = 0;
   }

   if (vi->numFrames && n >= vi->numFrames) {
      n = vi->numFrames - 1;
   }

   char err[100];
   const VSFrameRef *frame = vsapi->getFrame(n, node, err, sizeof(err));
   if (!frame) {
      // Shit failed. We should display err somewhere.
      // Not in imglabel, because this is not a fatal error.
      qDebug("Failed to retrieve frame %d.\n", n);
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


void VSViewer::keyPressEvent(QKeyEvent * event)
{
   int key = event->key();
   //Qt::KeyboardModifiers modifs = event->modifiers();

   switch (key)
   {
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
         {
         const VSVideoInfo *vi = vsapi->getVideoInfo(node);
         if (vi->numFrames) {
            seek(vi->numFrames - 1);
         }
         break;
         }
      default:
         QMainWindow::keyPressEvent(event);
         break;
   }
}


void VSViewer::onGotoFrame() {
   const VSVideoInfo *vi = vsapi->getVideoInfo(node);
   bool okay;
   int n = QInputDialog::getInt(this, "Go to frame", "Enter frame number:", currentFrameNum, 0, vi->numFrames-1, 1, &okay);
   if (okay) {
      seek(n);
   }
}
