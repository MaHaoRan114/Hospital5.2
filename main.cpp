#include "consultmainwindow.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug() << "启动医生问诊系统...";

    // 直接启动咨询主窗口，假设当前登录医生为"张医生"
    // 实际应用中应该从登录界面获取
    ConsultMainWindow w("张医生");
    w.show();

    return a.exec();
}
