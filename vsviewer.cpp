// TODO:
//    - line numbers
//    - highlight current line
//    - syntax highlighting


#include <QtCore>
#include <QtGui>
//#include <QDebug>
//#include <QtGlobal>

#include "vsviewer.h"
#include "preview.h"
#include "textedit.h"


void VSViewer::ui_init() {
   // Set up the menus.
   fileNew = new QAction("&New script", this);
   fileNew->setShortcut(QKeySequence("Ctrl+N"));

   fileOpen = new QAction("&Open script...", this);
   fileOpen->setShortcut(QKeySequence("Ctrl+O"));

   fileSave = new QAction("&Save script", this);
   fileSave->setShortcut(QKeySequence("Ctrl+S"));

   fileSaveAs = new QAction("Save script &as...", this);

   fileReload = new QAction("&Reload script", this);
   fileReload->setShortcut(QKeySequence("Ctrl+R"));
   fileReload->setEnabled(false);

   filePreview = new QAction("&Preview", this);
   filePreview->setShortcut(QKeySequence("F5"));

   QAction *fileQuit = new QAction("&Quit", this);
   fileQuit->setShortcut(QKeySequence("Ctrl+Q"));


   connect(fileNew, SIGNAL(triggered()),
              this, SLOT(onFileNew()));
   connect(fileOpen, SIGNAL(triggered()),
               this, SLOT(onFileOpen()));
   connect(fileSave, SIGNAL(triggered()),
               this, SLOT(onFileSave()));
   connect(fileSaveAs, SIGNAL(triggered()),
                 this, SLOT(onFileSaveAs()));
   connect(fileReload, SIGNAL(triggered()),
                 this, SLOT(onFileReload()));
   connect(filePreview, SIGNAL(triggered()),
                  this, SLOT(onFilePreview()));
   connect(fileQuit, SIGNAL(triggered()),
               this, SLOT(close()));


   QMenu *fileMenu = menuBar()->addMenu("&File");
   fileMenu->addAction(fileNew);
   fileMenu->addAction(fileOpen);
   fileMenu->addAction(fileSave);
   fileMenu->addAction(fileSaveAs);
   fileMenu->addAction(fileReload);
   fileMenu->addSeparator();
   fileMenu->addAction(filePreview);
   fileMenu->addSeparator();
   fileMenu->addAction(fileQuit);


   textEdit = new TextEdit();
   textEdit->setFont(QFont("monospace", 9));
   textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

   connect(textEdit->document(), SIGNAL(modificationChanged(bool)),
                           this, SLOT(set_title(bool)));

   setCentralWidget(textEdit);

   set_title(false);

   resize(800, 600);
}


VSViewer::VSViewer()
 : preview(NULL),
   scriptName("Untitled"),
   scriptExists(false)
{
   ui_init();
}


void VSViewer::onFileNew() {
   if (textEdit->document()->isModified()) {
      int ret = confirmDiscard();
      switch (ret) {
         case QMessageBox::Save:
            onFileSave();
            break;
         case QMessageBox::Discard:
            break;
         case QMessageBox::Cancel:
            return;
            break;
      }
   }

   scriptName = "Untitled";
   scriptExists = false;

   textEdit->setPlainText("");
   textEdit->document()->setModified(false);

   set_title(false);

   fileReload->setEnabled(false);
}


void VSViewer::onFileOpen() {
   QString name = QFileDialog::getOpenFileName(this, "Open VapourSynth script", QString(), QString(), 0, QFileDialog::DontUseNativeDialog);

   if (name.isNull()) {
      return;
   }

   if (textEdit->document()->isModified()) {
      int ret = confirmDiscard();
      switch (ret) {
         case QMessageBox::Save:
            onFileSave();
            break;
         case QMessageBox::Discard:
            break;
         case QMessageBox::Cancel:
            return;
            break;
      }
   }

   openFile(name);
}


void VSViewer::openFile(QString name) {
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
   textEdit->setPlainText(stream.readAll());
   file.close();

   textEdit->document()->setModified(false);

   fileReload->setEnabled(true);

   scriptName = name;
   scriptExists = true;

   set_title(false);
}


void VSViewer::onFileSave() {
   if (scriptExists) {
      saveFile(scriptName);

      set_title(false);
   } else {
      onFileSaveAs();
   }
}


void VSViewer::onFileSaveAs() {
   QString name = QFileDialog::getSaveFileName(this, "Save VapourSynth script", QString(), QString(), 0, QFileDialog::DontUseNativeDialog);

   if (name.isNull()) {
      return;
   }

   saveFile(name);

   scriptName = name;
   scriptExists = true;

   fileReload->setEnabled(true);

   set_title(false);
}


void VSViewer::saveFile(QString name) {
   QFile file(name);
   bool ret = file.open(QIODevice::WriteOnly);
   if (!ret) {
      errmsg(file.errorString());
      return;
   }

   QTextStream stream(&file);
   stream.setCodec("UTF-8");
   stream << textEdit->toPlainText();
   file.close();

   textEdit->document()->setModified(false);
}


void VSViewer::onFileReload() {
   if (scriptExists) {
      if (textEdit->document()->isModified()) {
         QMessageBox box(this);
         box.setText("Discard changes?");
         box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
         box.setDefaultButton(QMessageBox::Yes);
         int ret = box.exec();

         if (ret == QMessageBox::No) {
            return;
         }
      }

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


void VSViewer::set_title(bool modified) {
   QString title = "vsviewer - ";
   if (modified) {
      title += "*";
   }
   title += QFileInfo(scriptName).fileName();

   setWindowTitle(title);
}


void VSViewer::errmsg(QString msg) {
   QMessageBox box(this);
   box.setText(msg);
   box.exec();
}


int VSViewer::confirmDiscard() {
   QMessageBox box(this);
   box.setText("The script has been modified.");
   box.setInformativeText("Do you want to save your changes?");
   box.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
   box.setDefaultButton(QMessageBox::Save);
   return box.exec();
}


void VSViewer::closeEvent(QCloseEvent *event) {
   if (textEdit->document()->isModified()) {
      int ret = confirmDiscard();
      switch (ret) {
         case QMessageBox::Save:
            onFileSave();
            event->accept();
            break;
         case QMessageBox::Discard:
            event->accept();
            break;
         case QMessageBox::Cancel:
            event->ignore();
            break;
      }
   }
}
