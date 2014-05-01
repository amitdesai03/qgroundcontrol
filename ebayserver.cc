#include "ebayserver.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <cstdio>
#include <QDebug>
#include <QApplication>

EBayServer* EBayServer::instance()
{
    static EBayServer* _instance = 0;
    if(_instance == 0) {
        _instance = new EBayServer();

        /* Set the application as parent to ensure that this object
         * will be destroyed when the main application exits */
        _instance->setParent(qApp);
        qDebug() << "EBAY:" << "Created new EBay Server instance!";
    }
    else
    {
        qDebug() << "EBAY:" << "Returning existing EBay Server instance!";
    }
    return _instance;
}

EBayServer::EBayServer()
{
    tcpserver = new QTcpServer(this);
    tcpserver->listen(QHostAddress::Any, 1234);
    qDebug() << "EBAY:" << "TCP Port:1234 configured";

    connect(tcpserver, SIGNAL(newConnection()),
            this, SLOT(on_newConnection()));
}

void EBayServer::on_newConnection()
{
    socket = tcpserver->nextPendingConnection();
    if(socket->state() == QTcpSocket::ConnectedState)
    {
        qDebug() << "EBAY:" << "New TCP connection made!";
    }
    connect(socket, SIGNAL(disconnected()),
            this, SLOT(on_disconnected()));
    connect(socket, SIGNAL(readyRead()),
            this, SLOT(on_readyRead()));
}

void EBayServer::on_readyRead()
{
    while(socket->canReadLine())
    {
        QByteArray ba = socket->readLine();
        if(strcmp(ba.constData(), "!exit\n") == 0)
        {
            socket->disconnectFromHost();
            break;
        }
        qDebug() << "EBAY:" << "TCP Command from client:" << ba.constData();
        QString response = processCommand(ba.constData());
        socket->write(response.toLocal8Bit().data());
    }
}

void EBayServer::on_disconnected()
{
    disconnect(socket, SIGNAL(disconnected()));
    disconnect(socket, SIGNAL(readyRead()));
    socket->deleteLater();
    qDebug() << "EBAY:" << "TCP Connection disconnected!";
}

QString EBayServer::processCommand(const char* command )
{
    //return QString::fromStdString("37.375283,-121.923097");
    UASInterface* uas = UASManager::instance()->getActiveUAS();
    if(!uas)
    {
        qDebug() << "EBAY:" << "No Active UAS";
        return QString::fromStdString("Fail");
    }

    QString qCom = QString::fromLocal8Bit(command);

    if(qCom.contains("info-uas", Qt::CaseInsensitive))
    {

        qDebug() << "EBAY:" << "Current location:" << QString("%1").arg(uas->getLatitude(), 0, 'g', 13) << ":" << uas->getLongitude() <<":" <<uas->getAltitude();
        QString result = QString("%1").arg(uas->getLatitude(), 0, 'g', 13) + ","+ QString("%1").arg(uas->getLongitude(), 0, 'g', 13);

        return result;

    }
    else if(qCom.startsWith("set-location-uas", Qt::CaseInsensitive))
    {
        QStringList list = qCom.split(":");
        qDebug() << "EBAY:" << "Target location set to:" << list[1].toDouble()  << ":" << list[2].toDouble() << ":" << list[3].toDouble();

        Waypoint* wp = new Waypoint(0, list[1].toDouble(), list[2].toDouble(), list[3].toDouble(), 0.0, 0.25);
        uas->getWaypointManager()->addWaypointEditable(wp);
        uas->getWaypointManager()->writeWaypoints();
    }
    else if(qCom.startsWith("land", Qt::CaseInsensitive))
    {
        uas->land();
        qDebug() << "EBAY:" << "Landing!";
    }
    else if(qCom.startsWith("launch", Qt::CaseInsensitive))
    {
        qDebug() << "EBAY:" << "Launched!";
        uas->launch();
    }
    else if(qCom.startsWith("kill", Qt::CaseInsensitive))
    {
        qDebug() << "EBAY:" << "Emergency shutdown!";
        uas->shutdown();
    }
    return QString::fromStdString("Success");
}
