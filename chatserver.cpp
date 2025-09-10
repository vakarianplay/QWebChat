#include "chatserver.h"
#include <QFile>
#include <QHostAddress>
#include <QDateTime>
#include <QDebug>

ChatServer::ChatServer(quint16 wsPort, quint16 httpPort, QObject *parent)
    : QObject(parent),
    wsServer(new QWebSocketServer("Chat WebSocket Server", QWebSocketServer::NonSecureMode, this)),
    httpServer(new QTcpServer(this)),
    chatHistoryFile("chat_history.csv") {

    if (chatHistoryFile.open(QIODevice::ReadWrite | QIODevice::Append)) {
        if (chatHistoryFile.size() == 0) {
            QTextStream out(&chatHistoryFile);
            out << "IP,Date,Message\n";
        }
    }
    chatHistoryFile.close();

    if (wsServer->listen(QHostAddress::Any, wsPort)) {
        connect(wsServer, &QWebSocketServer::newConnection, this, &ChatServer::onNewConnection);
    }

    if (httpServer->listen(QHostAddress::Any, httpPort)) {
        connect(httpServer, &QTcpServer::newConnection, this, &ChatServer::onHttpConnection);
    }
}

ChatServer::~ChatServer() {
    wsServer->close();
    httpServer->close();
    chatHistoryFile.close();
    qDeleteAll(clients);
}

void ChatServer::writeToCsv(const QString &ip, const QString &date, const QString &message) {
    if (chatHistoryFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream out(&chatHistoryFile);

        QString escapedMessage = message;
        escapedMessage.replace("\"", "\"\""); // Заменяем кавычки для корректной обработки
        escapedMessage = "\"" + escapedMessage + "\""; // Оборачиваем сообщение в кавычки
        out << QString("%1,%2,%3\n").arg(ip, date, escapedMessage);

        chatHistoryFile.close(); // Закрываем файл после записи
    } else {
        qDebug() << "Не удалось открыть файл chat_history.csv для записи!";
    }
}

// Обработка нового WebSocket соединения
void ChatServer::onNewConnection() {
    auto client = wsServer->nextPendingConnection();
    clients.append(client);

    // Получение IP-адреса клиента
    QHostAddress clientAddress = client->peerAddress();
    QString ipAddress = clientAddress.toString();
    clientIPs[client] = ipAddress;

    connect(client, &QWebSocket::textMessageReceived, this, &ChatServer::onTextMessageReceived);
    connect(client, &QWebSocket::disconnected, this, &ChatServer::onClientDisconnected);

    qDebug() << "Новый WebSocket клиент подключился с IP:" << ipAddress;

    sendLastMessages(client);
}

void ChatServer::sendLastMessages(QWebSocket *client) {
    if (!chatHistoryFile.exists() || !chatHistoryFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Не удалось открыть файл chat_history.csv для чтения!";
        return;
    }

    // Читаем весь файл построчно
    QList<QString> lines;
    QTextStream in(&chatHistoryFile);
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }

    chatHistoryFile.close();

    // Проверяем, есть ли в файле сообщения (пропускаем заголовок "IP,Date,Message")
    if (lines.size() <= 1) {
        return; // В файле только заголовок или он пуст
    }

    // Берём последние 10 сообщений (исключая заголовок)
    int startIndex = qMax(1, lines.size() - 50); // 1, чтобы пропустить заголовок
    for (int i = startIndex; i < lines.size(); ++i) {
        QStringList parts = lines[i].split(',', Qt::KeepEmptyParts);

        // Если формат некорректен, пропускаем строку
        if (parts.size() < 3) {
            continue;
        }

        QString ip = parts[0];
        QString date = parts[1];
        QString message = parts[2];

        // Воссоздаём сообщение в том же формате, что используется для отправки сообщений
        QString fullMessage = QString("<span style=\"color: blue; font-weight: bold;\">[%1]</span> "
                                      "<span style=\"color: gray;\">[%2]</span> %3")
                                  .arg(ip, date, message);

        // Отправляем сообщение клиенту
        client->sendTextMessage(fullMessage);
    }
}

void ChatServer::onTextMessageReceived(const QString &message) {
    auto senderClient = qobject_cast<QWebSocket *>(sender());
    if (senderClient) {
        QString senderIP = clientIPs.value(senderClient, "Unknown");

        QString currentTime = QDateTime::currentDateTime()
                                  .toString("dd:MM:yyyy hh:mm:ss.zzz");

        QString fullMessage = QString("<span style=\"color: blue; font-weight: bold;\">[%1]</span> "
                                      "<span style=\"color: gray;\">[%2]</span> %3")
                                  .arg(senderIP)
                                  .arg(currentTime)
                                  .arg(message);

        writeToCsv(senderIP, currentTime, message);

        for (QWebSocket *client : qAsConst(clients)) {
            client->sendTextMessage(fullMessage);
        }
    }
}

// Обработка отключения WebSocket клиента
void ChatServer::onClientDisconnected() {
    auto client = qobject_cast<QWebSocket *>(sender());
    if (client) {
        QString clientIP = clientIPs.value(client, "Unknown");

        qDebug() << "WebSocket клиент отключился с IP:" << clientIP;

        clients.removeAll(client);
        clientIPs.remove(client);
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
        content.replace("{ws_port}", QByteArray::number(wsServer->serverPort()));
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