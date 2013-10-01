#ifndef PYTHONHIGHLIGHTER_H
#define PYTHONHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>


class QTextDocument;


class PythonHighlighter : public QSyntaxHighlighter {
   Q_OBJECT

   public:
      PythonHighlighter(QTextDocument *parent = 0);

   protected:
      void highlightBlock(const QString &text);

   private:
      struct HighlightingRule {
         QRegExp pattern;
         QTextCharFormat format;
      };

      QVector<HighlightingRule> highlightingRules;

      QTextCharFormat keywordFormat;
      QTextCharFormat quotationFormat;
      QTextCharFormat numberFormat;
      QTextCharFormat commentFormat;
};

#endif
