#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QMap>
#include <QFile>

class ChatServer : public QObject {
    Q_OBJECT

public:
    explicit ChatServer(quint16 wsPort, quint16 httpPort, QObject *parent = nullptr);
    ~ChatServer();

private slots:
    // WebSocket
    void onNewConnection();
    void onTextMessageReceived(const QString &message);
    void onClientDisconnected();

    // HTTP
    void onHttpConnection();

private:
    QWebSocketServer *wsServer;               
    QTcpServer *httpServer;                    
    QList<QWebSocket *> clients;               
    QMap<QWebSocket *, QString> clientIPs;    

    QFile chatHistoryFile;                    

    void writeToCsv(const QString &ip, const QString &date, const QString &message);   
    void sendLastMessages(QWebSocket *client);                                        
};


#endif // CHATSERVER_H
