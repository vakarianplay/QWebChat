#include "chatserver.h"
#include <QFile>
#include <QDebug>

ChatServer::ChatServer(quint16 wsPort, quint16 httpPort, QObject *parent)
    : QObject(parent),
    wsServer(new QWebSocketServer("Chat WebSocket Server", QWebSocketServer::NonSecureMode, this)),
    httpServer(new QTcpServer(this)) {
    // Запуск WebSocket сервера
    if (wsServer->listen(QHostAddress::Any, wsPort)) {
        qDebug() << "WebSocket сервер запущен на порту" << wsServer->serverPort();
        connect(wsServer, &QWebSocketServer::newConnection, this, &ChatServer::onNewConnection);
    } else {
        qDebug() << "Ошибка запуска WebSocket сервера!";
    }

    // Запуск HTTP сервера
    if (httpServer->listen(QHostAddress::Any, httpPort)) {
        qDebug() << "HTTP сервер запущен на порту" << httpServer->serverPort();
        connect(httpServer, &QTcpServer::newConnection, this, &ChatServer::onHttpConnection);
    } else {
        qDebug() << "Ошибка запуска HTTP сервера!";
    }
}

ChatServer::~ChatServer() {
    wsServer->close();
    httpServer->close();
    qDeleteAll(clients);
}

// Обработка нового WebSocket соединения
void ChatServer::onNewConnection() {
    auto client = wsServer->nextPendingConnection();
    clients.append(client);

    connect(client, &QWebSocket::textMessageReceived, this, &ChatServer::onTextMessageReceived);
    connect(client, &QWebSocket::disconnected, this, &ChatServer::onClientDisconnected);

    qDebug() << "Новый WebSocket клиент подключился";
}

// Обработка полученного сообщения через WebSocket
void ChatServer::onTextMessageReceived(const QString &message) {
    auto senderClient = qobject_cast<QWebSocket *>(sender());
    if (senderClient) {
        qDebug() << "Получено сообщение:" << message;

        // Распространяем сообщение всем клиентам
        for (QWebSocket *client : qAsConst(clients)) {
            client->sendTextMessage(message);
        }
    }
}

// Обработка отключения WebSocket клиента
void ChatServer::onClientDisconnected() {
    auto client = qobject_cast<QWebSocket *>(sender());
    if (client) {
        qDebug() << "WebSocket клиент отключился";
        clients.removeAll(client);
        client->deleteLater();
    }
}

// Обработка нового HTTP соединения (раздача файлов)
void ChatServer::onHttpConnection() {
    QTcpSocket *socket = httpServer->nextPendingConnection();
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    if (!socket->waitForReadyRead(5000)) {
        socket->close();
        return;
    }

    // Читаем HTTP-запрос
    QByteArray request = socket->readAll();
    qDebug() << "HTTP запрос:" << request;

    // Предоставляем файлы (здесь раздаём index.html)
    QFile file("index.html");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray content = file.readAll();
        file.close();

        QByteArray response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Content-Length: " + QByteArray::number(content.size()) + "\r\n";
        response += "\r\n";
        response += content;

        socket->write(response);
        socket->waitForBytesWritten();
    } else {
        // Если файл не найден
        QByteArray response = "HTTP/1.1 404 Not Found\r\n";
        response += "Content-Length: 0\r\n";
        response += "\r\n";

        socket->write(response);
        socket->waitForBytesWritten();
    }

    socket->close();
}