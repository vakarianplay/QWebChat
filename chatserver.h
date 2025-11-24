#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QTcpServer>
#include <QMap>
#include "filemanager.h"
#include "messageprocessor.h"

class ChatServer : public QObject
{
    Q_OBJECT

public:
    explicit ChatServer(quint16 wsPort, quint16 httpPort, QObject *parent = nullptr);
    ~ChatServer();

private slots:
    void onNewWebSocketConnection();
    void onWebSocketTextMessage(const QString &message);
    void onWebSocketBinaryMessage(const QByteArray &message);
    void onWebSocketDisconnected();

    void onHttpConnection();

private:
    QWebSocketServer *webSocketServer;
    QTcpServer *httpServer;

    FileManager *fileManager;
    MessageProcessor *messageProcessor;

    QMap<QWebSocket*, QString> clients;

    void handleTextMessage(QWebSocket *client, const QString &message);
    void handleFileMessage(QWebSocket *client, const QByteArray &message);
    void sendToAllClients(const QString &message);
    void sendFileList(QWebSocket *client);
    void sendLastMessages(QWebSocket *client);

    void processHttpRequest(QTcpSocket *socket, const QString &request);
    void serveHtmlPage(QTcpSocket *socket);
    void serveFile(QTcpSocket *socket, const QString &fileName);

    QString getClientInfo(QWebSocket *client) const;
};

#endif // CHATSERVER_H