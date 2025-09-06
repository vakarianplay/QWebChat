#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QMap>

class ChatServer : public QObject {
    Q_OBJECT

public:
    explicit ChatServer(quint16 wsPort, quint16 httpPort, QObject *parent = nullptr);
    ~ChatServer();

private slots:
    // WebSocket события
    void onNewConnection();
    void onTextMessageReceived(const QString &message);
    void onClientDisconnected();

    // HTTP события
    void onHttpConnection();

private:
    QWebSocketServer *wsServer;                // WebSocket сервер
    QTcpServer *httpServer;                    // HTTP сервер
    QList<QWebSocket *> clients;               // Список WebSocket клиентов
    QMap<QWebSocket *, QString> clientIPs;     // Карта клиента и его IP-адреса
};

#endif // CHATSERVER_H