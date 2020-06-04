#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "controlclient.h"
#include "util.h"

#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;


private slots:
    void sendDirection(const btremote::Direction& direction);
    void sendToggle(char func, bool status);
    void sendCommand(const char* command);
    void setSpeed(int value);
    void openDeviceListDialog();
    void connectToDevice(QBluetoothDeviceInfo address);
    void reconnect();
    void disconnect();
    void speedUp();
    void speedDown();
    void onSocketError(QString error);
    void onConnected(const QString& peer);
    void onDisconnected();
    void onMessageReceived(const QString& peerName, const QString& message);
    virtual void keyPressEvent(QKeyEvent *ev) override;
    virtual void keyReleaseEvent(QKeyEvent *ev) override;
    void handleKeyPress(int key);
    void handleKeyRelease(int key);

private:
    void initializeToggles();

    Ui::MainWindow *ui;
    ControlClient client;
    QBluetoothLocalDevice localDevice;
    QBluetoothDeviceInfo currentDevice;
    int speed = 0;

    // 保证在析构过程中不会错误触发一些东西
    bool active = true;
    // 保证不会同时有两个重连请求被同时预约
    bool reconnectionPending = false;

    QString voltage, temperature, humidity;

    static const int RECONNECT_TIMEOUT = 2000;
    static const int MAX_SPEED = 5;
    static const int MIN_SPEED = 1;

    static const char TOGGLE_SPRAY = 'P';
    static const char TOGGLE_DIFF = 'C';
};

#endif // MAINWINDOW_H
