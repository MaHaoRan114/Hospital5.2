#include "PersonalCenter.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    PersonalCenter window;
    window.show();
    return app.exec();
}
