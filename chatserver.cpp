#include "chatserver.h"
#include <QTcpSocket>
#include <QHostAddress>
#include <QDateTime>
#include <QDebug>

ChatServer::ChatServer(quint16 wsPort, quint16 httpPort, QObject *parent)
    : QObject(parent),
      webSocketServer(new QWebSocketServer("Chat Server", QWebSocketServer::NonSecureMode, this)),
      httpServer(new QTcpServer(this)),
      fileManager(new FileManager(this)),
      messageProcessor(new MessageProcessor(this))
{
    // WebSocket сервер
    if (webSocketServer->listen(QHostAddress::Any, wsPort)) {
        connect(webSocketServer, &QWebSocketServer::newConnection, 
                this, &ChatServer::onNewWebSocketConnection);
        qDebug() << "WebSocket server listening on port" << wsPort;
    }

    // HTTP сервер
    if (httpServer->listen(QHostAddress::Any, httpPort)) {
        connect(httpServer, &QTcpServer::newConnection, 
                this, &ChatServer::onHttpConnection);
        qDebug() << "HTTP server listening on port" << httpPort;
    }
}

ChatServer::~ChatServer()
{
    webSocketServer->close();
    httpServer->close();
    qDeleteAll(clients.keys());
}

void ChatServer::onNewWebSocketConnection()
{
    QWebSocket *client = webSocketServer->nextPendingConnection();
    QString clientInfo = getClientInfo(client);

    clients.insert(client, clientInfo);
    qDebug() << "New client connected:" << clientInfo;

    connect(client, &QWebSocket::textMessageReceived,
            this, &ChatServer::onWebSocketTextMessage);
    connect(client, &QWebSocket::binaryMessageReceived,
            this, &ChatServer::onWebSocketBinaryMessage);
    connect(client, &QWebSocket::disconnected,
            this, &ChatServer::onWebSocketDisconnected);

    sendLastMessages(client);
}

void ChatServer::onWebSocketTextMessage(const QString &message)
{
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    if (client) {
        handleTextMessage(client, message);
    }
}

void ChatServer::onWebSocketBinaryMessage(const QByteArray &message)
{
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    if (client) {
        handleFileMessage(client, message);
    }
}

void ChatServer::onWebSocketDisconnected()
{
    QWebSocket *client = qobject_cast<QWebSocket*>(sender());
    if (client) {
        QString clientInfo = clients.value(client);
        qDebug() << "Client disconnected:" << clientInfo;
        clients.remove(client);
        client->deleteLater();
    }
}

void ChatServer::handleTextMessage(QWebSocket *client, const QString &message)
{
    if (message.startsWith("/files")) {
        sendFileList(client);
        return;
    }

    QString clientInfo = clients.value(client);
    QString currentTime = QDateTime::currentDateTime().toString("dd:MM:yyyy hh:mm:ss.zzz");

    QString formattedMessage = messageProcessor->formatMessage(clientInfo, currentTime, message);
    messageProcessor->logMessage(clientInfo, currentTime, message);

    sendToAllClients(formattedMessage);
}

void ChatServer::handleFileMessage(QWebSocket *client, const QByteArray &message)
{
    if (message.size() <= sizeof(quint32)) return;

    // Парсим бинарное сообщение
    quint32 fileNameSize;
    QDataStream stream(message);
    stream >> fileNameSize;

    if (message.size() < sizeof(quint32) + fileNameSize) return;

    QString fileName = QString::fromUtf8(message.mid(sizeof(quint32), fileNameSize));
    QByteArray fileData = message.mid(sizeof(quint32) + fileNameSize);

    QString clientInfo = clients.value(client);
    QString currentTime = QDateTime::currentDateTime().toString("dd:MM:yyyy hh:mm:ss.zzz");

    // Сохраняем файл
    QString savedFileName = fileManager->saveFile(fileData, fileName, clientInfo);
    if (!savedFileName.isEmpty()) {
        QString fileMessage = QString("%1 (%2 bytes)").arg(fileName).arg(fileData.size());
        QString formattedMessage = messageProcessor->formatMessage(
            clientInfo, currentTime, fileMessage, "file", savedFileName);

        messageProcessor->logMessage(clientInfo, currentTime, fileMessage, "file", savedFileName);
        sendToAllClients(formattedMessage);
    }
}

void ChatServer::sendToAllClients(const QString &message)
{
    for (QWebSocket *client : clients.keys()) {
        if (client->isValid()) {
            client->sendTextMessage(message);
        }
    }
}

void ChatServer::sendFileList(QWebSocket *client)
{
    QStringList files = fileManager->getFileList();
    QString message = "<span style=\"color: gray;\">Доступные файлы:</span><br>";

    if (files.isEmpty()) {
        message += "<span style=\"color: gray;\">Файлов нет</span>";
    } else {
        for (const QString &fileName : files) {
            QFileInfo fileInfo(fileManager->getFileList().contains(fileName) ? fileName : "");
            message += QString("<a href=\"/files/%1\" download=\"%2\">📎 %3</a><br>")
                          .arg(fileName, fileInfo.fileName(), fileInfo.fileName());
        }
    }

    client->sendTextMessage(message);
}

void ChatServer::sendLastMessages(QWebSocket *client)
{
    QString history = messageProcessor->getLastMessages();
    if (!history.isEmpty()) {
        QStringList lines = history.split('\n');
        for (const QString &line : lines) {
            QStringList parts = line.split(',');
            if (parts.size() >= 3) {
                QString formatted = messageProcessor->formatMessage(
                    parts[0], parts[1], parts[2], 
                    parts.size() > 3 ? parts[3] : "text",
                    parts.size() > 4 ? parts[4] : "");
                client->sendTextMessage(formatted);
            }
        }
    }
}

void ChatServer::onHttpConnection()
{
    QTcpSocket *socket = httpServer->nextPendingConnection();
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    if (socket->waitForReadyRead(5000)) {
        QString request = QString::fromUtf8(socket->readAll());
        processHttpRequest(socket, request);
    }

    socket->close();
}

void ChatServer::processHttpRequest(QTcpSocket *socket, const QString &request)
{
    if (request.startsWith("GET /files/")) {
        QString fileName = request.section(' ', 1, 1).mid(7);
        serveFile(socket, fileName);
    } else if (request.startsWith("GET /")) {
        serveHtmlPage(socket);
    } else {
        socket->write("HTTP/1.1 404 Not Found\r\n\r\n");
    }
}

void ChatServer::serveHtmlPage(QTcpSocket *socket)
{
    QFile file("index.html");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray content = file.readAll();
        content.replace("{ws_port}", QByteArray::number(webSocketServer->serverPort()));

        QByteArray response = "HTTP/1.1 200 OK\r\n"
                             "Content-Type: text/html\r\n"
                             "Content-Length: " + QByteArray::number(content.size()) + "\r\n"
                             "\r\n" + content;

        socket->write(response);
    } else {
        socket->write("HTTP/1.1 404 Not Found\r\n\r\n");
    }
}

void ChatServer::serveFile(QTcpSocket *socket, const QString &fileName)
{
    if (fileManager->fileExists(fileName)) {
        QByteArray content = fileManager->loadFile(fileName);
        QString mimeType = fileManager->getMimeType(fileName);

        QByteArray response = "HTTP/1.1 200 OK\r\n"
                             "Content-Type: " + mimeType.toUtf8() + "\r\n"
                             "Content-Length: " + QByteArray::number(content.size()) + "\r\n"
                             "Content-Disposition: attachment; filename=\"" + fileName.toUtf8() + "\"\r\n"
                             "\r\n" + content;

        socket->write(response);
    } else {
        socket->write("HTTP/1.1 404 Not Found\r\n\r\nFile not found");
    }
}

QString ChatServer::getClientInfo(QWebSocket *client) const
{
    return client->peerAddress().toString();
}