#include <QCoreApplication>
#include <QString>
#include "chatserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    quint16 httpPort = 8080;
    quint16 wsPort = 8081;

    QStringList arguments = app.arguments();
    for (int i = 1; i < arguments.size(); ++i) {
        if (arguments[i].startsWith("--http=")) {
            httpPort = arguments[i].mid(7).toUShort();
        } else if (arguments[i].startsWith("--ws=")) {
            wsPort = arguments[i].mid(5).toUShort();
        }
    }

    qDebug() << "Starting ChatServer:\nHTTP port:" << httpPort << "\nWebSocket port:" << wsPort;

    ChatServer server(wsPort, httpPort);
    return app.exec();
}