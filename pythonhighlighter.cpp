// TODO:
//    - """multiline
//         strings"""


#include <QtGui>

#include "pythonhighlighter.h"


PythonHighlighter::PythonHighlighter(QTextDocument *parent)
 : QSyntaxHighlighter(parent)
{
   HighlightingRule rule;

   keywordFormat.setForeground(Qt::darkBlue);
   keywordFormat.setFontWeight(QFont::Bold);
   QStringList keywordPatterns;

   keywordPatterns
      << "\\bFalse\\b"
      << "\\bNone\\b"
      << "\\bTrue\\b"
      << "\\band\\b"
      << "\\bas\\b"
      << "\\bassert\\b"
      << "\\bbreak\\b"
      << "\\bclass\\b"
      << "\\bcontinue\\b"
      << "\\bdef\\b"
      << "\\bdel\\b"
      << "\\belif\\b"
      << "\\belse\\b"
      << "\\bexcept\\b"
      << "\\bfinally\\b"
      << "\\bfor\\b"
      << "\\bfrom\\b"
      << "\\bglobal\\b"
      << "\\bif\\b"
      << "\\bimport\\b"
      << "\\bin\\b"
      << "\\bis\\b"
      << "\\blambda\\b"
      << "\\bnonlocal\\b"
      << "\\bnot\\b"
      << "\\bor\\b"
      << "\\bpass\\b"
      << "\\braise\\b"
      << "\\breturn\\b"
      << "\\btry\\b"
      << "\\bwhile\\b"
      << "\\bwith\\b"
      << "\\byield\\b";

   for (int i = 0; i < keywordPatterns.size(); i++) {
      rule.pattern = QRegExp(keywordPatterns[i]);
      rule.format = keywordFormat;
      highlightingRules.append(rule);
   }

   numberFormat.setForeground(Qt::darkMagenta);
   rule.pattern = QRegExp("[0-9]+");
   rule.format = numberFormat;
   highlightingRules.append(rule);

   quotationFormat.setForeground(Qt::darkMagenta);
   rule.pattern = QRegExp("\".*\"");
   rule.format = quotationFormat;
   highlightingRules.append(rule);
   rule.pattern = QRegExp("'.*'");
   highlightingRules.append(rule);

   commentFormat.setForeground(Qt::darkGray);
   rule.pattern = QRegExp("#[^\n]*");
   rule.format = commentFormat;
   highlightingRules.append(rule);
}

void PythonHighlighter::highlightBlock(const QString &text)
{
   for (int i = 0; i < highlightingRules.size(); i++) {
      QRegExp expression(highlightingRules[i].pattern);
      int index = expression.indexIn(text);
      while (index >= 0) {
         int length = expression.matchedLength();
         setFormat(index, length, highlightingRules[i].format);
         index = expression.indexIn(text, index + length);
      }
   }
   /*
   setCurrentBlockState(0);

   int startIndex = 0;
   if (previousBlockState() != 1)
      startIndex = commentStartExpression.indexIn(text);

   while (startIndex >= 0) {
      int endIndex = commentEndExpression.indexIn(text, startIndex);
      int commentLength;
      if (endIndex == -1) {
         setCurrentBlockState(1);
         commentLength = text.length() - startIndex;
      } else {
         commentLength = endIndex - startIndex
            + commentEndExpression.matchedLength();
      }
      setFormat(startIndex, commentLength, multiLineCommentFormat);
      startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
   }
   */
}
