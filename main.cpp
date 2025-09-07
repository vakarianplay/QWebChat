#include <QCoreApplication>
#include "chatserver.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // Создаем объект сервера
    quint16 wsPort = 9090;    // Порт WebSocket
    quint16 httpPort = 10101;  // Порт HTTP
    ChatServer chatServer(wsPort, httpPort);

    return app.exec();
}