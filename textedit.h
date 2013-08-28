#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QtCore>
#include <QtGui>


class TextEdit : public QPlainTextEdit {
   Q_OBJECT

      /*
   public:
      TextEdit(QWidget *parent = 0);
      */

   private:
      void keyPressEvent(QKeyEvent *event);
};

#endif
