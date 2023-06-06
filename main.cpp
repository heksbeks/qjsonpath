#include <QCoreApplication>
#include "qjsonpath.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QJsonPath::unittest();

    return a.exec();
}
