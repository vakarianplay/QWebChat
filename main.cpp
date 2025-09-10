#include <QCoreApplication>
#include <QString>
#include "chatserver.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    quint16 httpPort = 8080;
    quint16 wsPort = 8081;

    QStringList arguments = app.arguments();
    if (arguments.size() > 1) {
        for (int i = 1; i < arguments.size(); ++i) {
            QString arg = arguments[i];
            if (arg.startsWith("--http=")) {
                httpPort = arg.mid(QString("--http=").size()).toUShort();
            } else if (arg.startsWith("--ws=")) {
                wsPort = arg.mid(QString("--ws=").size()).toUShort();
            }
        }
    }

    qDebug() << "Run ChatServer:" << "\nHTTP port:" << httpPort << "\nWebSocket port:" << wsPort;

    ChatServer chatServer(wsPort, httpPort);

    return app.exec();
}