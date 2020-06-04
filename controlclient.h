#ifndef CONTROLCLIENT_H
#define CONTROLCLIENT_H

#include <QBluetoothServiceInfo>
#include <QBluetoothSocket>
#include <QObject>
#include <QWidget>

class ControlClient : public QObject
{
    Q_OBJECT
public:
    explicit ControlClient(QObject *parent = nullptr);
    ~ControlClient();

    void startClient(const QBluetoothServiceInfo &remoteService);
    void startClient(const QBluetoothAddress &remoteAddress);
    void stopClient();

public slots:
    void sendMessage(const QByteArray& data);

signals:
    void messageReceived(const QString &sender, const QString &message);
    void connected(const QString &name);
    void disconnected();
    void socketErrorOccurred(const QString &errorString);

private slots:
    void readSocket();
    void connected();
    void onSocketErrorOccurred(QBluetoothSocket::SocketError);

private:
    QBluetoothSocket *socket = nullptr;
    static const QBluetoothUuid SPP;

};

#endif // CONTROLCLIENT_H
