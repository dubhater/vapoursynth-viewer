// TODO:
//    - save and save as
//    - modified flag
//    - line numbers
//    - highlight current line
//    - syntax highlighting
//    - confirm closing a script and quitting


#include <QtCore>
#include <QtGui>
//#include <QDebug>
//#include <QtGlobal>

#include "vsviewer.h"
#include "preview.h"
#include "textedit.h"


void VSViewer::ui_init() {
   // Set up the menus.
   fileOpen = new QAction("&Open script...", this);
   fileOpen->setShortcut(QKeySequence("Ctrl+O"));

   fileReload = new QAction("&Reload script", this);
   fileReload->setShortcut(QKeySequence("Ctrl+R"));
   fileReload->setEnabled(false);

   filePreview = new QAction("&Preview", this);
   filePreview->setShortcut(QKeySequence("F5"));

   QAction *fileQuit = new QAction("&Quit", this);
   fileQuit->setShortcut(QKeySequence("Ctrl+Q"));

   connect(fileOpen, SIGNAL(triggered()),
               this, SLOT(onFileOpen()));
   connect(fileReload, SIGNAL(triggered()),
                 this, SLOT(onFileReload()));
   connect(filePreview, SIGNAL(triggered()),
                  this, SLOT(onFilePreview()));
   connect(fileQuit, SIGNAL(triggered()),
               this, SLOT(close()));


   QMenu *fileMenu = menuBar()->addMenu("&File");
   fileMenu->addAction(fileOpen);
   fileMenu->addAction(fileReload);
   fileMenu->addSeparator();
   fileMenu->addAction(filePreview);
   fileMenu->addSeparator();
   fileMenu->addAction(fileQuit);


   textEdit = new TextEdit();
   textEdit->setFont(QFont("monospace", 9));
   textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

   setCentralWidget(textEdit);

   set_title(scriptName, false);

   resize(800, 600);
}


VSViewer::VSViewer()
 : preview(NULL),
   scriptName("Untitled")
{
   ui_init();
}


void VSViewer::onFileOpen() {
   QString name = QFileDialog::getOpenFileName(this, "Open VapourSynth script", QString(), QString(), 0, QFileDialog::DontUseNativeDialog);

   if (name.isNull()) {
      return;
   }

   openFile(name);
}


void VSViewer::openFile(QString name) {
   // confirm discarding the changes
   //

   QFile file(name);
   bool ret = file.open(QIODevice::ReadOnly);
   if (!ret) {
      errmsg(file.errorString());
      return;
   }

   if (file.size() > 16*1024*1024) {
      errmsg("Script size is over 16 MiB. That is insane.");
      file.close();
      return;
   }

   QTextStream stream(&file);
   stream.setCodec("UTF-8");
   QString script = stream.readAll();
   file.close();
   textEdit->setPlainText(script);

   fileReload->setEnabled(true);

   set_title(name, false);

   scriptName = name;
}


void VSViewer::onFileReload() {
   if (!scriptName.isNull()) {
      openFile(scriptName);
   }
}


void VSViewer::onFilePreview() {
   if (!preview) {
      preview = new Preview(this);
   }

   preview->openFile(textEdit->toPlainText(), scriptName);

   preview->show();
   preview->raise();
   preview->activateWindow();
}


void VSViewer::set_title(QString script_name, bool modified) {
   QString title = "vsviewer - ";
   if (modified) {
      title += "*";
   }
   title += QFileInfo(script_name).fileName();

   setWindowTitle(title);
}


void VSViewer::errmsg(QString msg) {
   QMessageBox box(this);
   box.setText(msg);
   box.exec();
}
