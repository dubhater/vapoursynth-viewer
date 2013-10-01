#include <QtCore>
#include <QtGui>

#include "textedit.h"


void TextEdit::keyPressEvent(QKeyEvent *event) {
   int key = event->key();
   Qt::KeyboardModifiers modifs = event->modifiers();

   if ((modifs & Qt::ShiftModifier) ||
       (modifs & Qt::ControlModifier) ||
       (modifs & Qt::AltModifier) ||
       (modifs & Qt::MetaModifier)) {
      QPlainTextEdit::keyPressEvent(event);
      return;
   }

   switch (key) {
      case Qt::Key_Tab: {
         int pos = textCursor().positionInBlock();
         int spaces = 4 - (pos % 4);
         insertPlainText(QString(spaces, ' '));
         break;
      }
      default:
         QPlainTextEdit::keyPressEvent(event);
         break;
   }
}
