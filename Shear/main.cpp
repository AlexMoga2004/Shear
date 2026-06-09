#include "Shear.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Shear window;
    window.show();
    return app.exec();
}
