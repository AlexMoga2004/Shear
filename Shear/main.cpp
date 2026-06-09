#include "Shear.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("ShearProject");
    QCoreApplication::setApplicationName("Shear");

    Shear window;
    window.show();
    return app.exec();
}
