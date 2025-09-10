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
        escapedMessage.replace("\"", "\"\"");
        escapedMessage = "\"" + escapedMessage + "\"";
        out << QString("%1,%2,%3\n").arg(ip, date, escapedMessage);

        chatHistoryFile.close();
    }
}

void ChatServer::onNewConnection() {
    auto client = wsServer->nextPendingConnection();
    clients.append(client);

    // Получение IP-адреса клиента
    QHostAddress clientAddress = client->peerAddress();
    QString ipAddress = clientAddress.toString();
    clientIPs[client] = ipAddress;

    connect(client, &QWebSocket::textMessageReceived, this, &ChatServer::onTextMessageReceived);
    connect(client, &QWebSocket::disconnected, this, &ChatServer::onClientDisconnected);

    qDebug() << "New client:" << ipAddress;

    sendLastMessages(client);
}

void ChatServer::sendLastMessages(QWebSocket *client) {
    if (!chatHistoryFile.exists() || !chatHistoryFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QList<QString> lines;
    QTextStream in(&chatHistoryFile);
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }

    chatHistoryFile.close();

    if (lines.size() <= 1) {
        return;
    }

    int startIndex = qMax(1, lines.size() - 50);
    for (int i = startIndex; i < lines.size(); ++i) {
        QStringList parts = lines[i].split(',', Qt::KeepEmptyParts);

        if (parts.size() < 3) {
            continue;
        }

        QString ip = parts[0];
        QString date = parts[1];
        QString message = parts[2];

        QString fullMessage = QString("<span style=\"color: blue; font-weight: bold;\">[%1]</span> "
                                      "<span style=\"color: gray;\">[%2]</span> %3")
                                  .arg(ip, date, message);

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

void ChatServer::onClientDisconnected() {
    auto client = qobject_cast<QWebSocket *>(sender());
    if (client) {
        QString clientIP = clientIPs.value(client, "Unknown");

        qDebug() << "WebSocket client disconnected:" << clientIP;

        clients.removeAll(client);
        clientIPs.remove(client);
        client->deleteLater();
    }
}

void ChatServer::onHttpConnection() {
    QTcpSocket *socket = httpServer->nextPendingConnection();
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    if (!socket->waitForReadyRead(5000)) {
        socket->close();
        return;
    }

    QByteArray request = socket->readAll();
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
        QByteArray response = "HTTP/1.1 404 Not Found\r\n";
        response += "Content-Length: 0\r\n";
        response += "\r\n";
        socket->write(response);
        socket->waitForBytesWritten();
    }

    socket->close();
}