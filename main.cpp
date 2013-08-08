#include <QApplication>

#include "vsviewer.h"


int main(int argv, char **args)
{
   QApplication app(argv, args);
   app.setOrganizationName("vsviewer");
   app.setApplicationName("vsviewer");

   VSViewer viewer;

   viewer.show();

   return app.exec();
}
