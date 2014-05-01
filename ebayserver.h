#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include "UASManager.h"
#include "UASInterface.h"
#include "Imagery.h"

class QTcpServer;
class QTcpSocket;

class EBayServer : public QObject
{
    Q_OBJECT
public:
    static EBayServer* instance();
    void listen();
signals:

public slots:
    void on_newConnection();
    void on_readyRead();
    void on_disconnected();

protected:
    EBayServer();

private:
    static EBayServer* _instance;
    QTcpServer* tcpserver;
    QTcpSocket* socket;
    QString processCommand(const char*);

};

#endif // SERVER_H
