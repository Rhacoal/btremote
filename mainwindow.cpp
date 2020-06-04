#include "devicelistdialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QBluetoothServiceDiscoveryAgent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // check if device is available
    if (localDevice.isValid()) {
        localDevice.powerOn();
        localDevice.setHostMode(QBluetoothLocalDevice::HostMode::HostDiscoverable);
        ui->statusBar->showMessage(tr("本机名称: ") + localDevice.name());
    } else {
        QMessageBox::critical(this, tr("错误"), tr("无法找到蓝牙设备。请确认蓝牙适配器被正确安装。"));
        exit(0);
    }

    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::openDeviceListDialog);
    //this, &MainWindow::disconnect
    connect(ui->disconnectButton, &QPushButton::clicked, [this](){disconnect();});
    connect(ui->speedUpButton, &QPushButton::clicked, this, &MainWindow::speedUp);
    connect(ui->speedDownButton, &QPushButton::clicked, this, &MainWindow::speedDown);

    // 当松开, 发送一个 "Q;"
    connect(ui->wButton, &QPushButton::released, this, [this](){
        handleKeyRelease(Qt::Key_W);
    });
    connect(ui->aButton, &QPushButton::released, this, [this](){
        handleKeyRelease(Qt::Key_A);
    });
    connect(ui->sButton, &QPushButton::released, this, [this](){
        handleKeyRelease(Qt::Key_S);
    });
    connect(ui->dButton, &QPushButton::released, this, [this](){
        handleKeyRelease(Qt::Key_D);
    });

    // 当按下, 发送对应的方向信号. 不过还是用键盘比较方便，建议用键盘.
    connect(ui->wButton, &QPushButton::pressed, this, [this](){
        handleKeyPress(Qt::Key_W);
    });
    connect(ui->aButton, &QPushButton::pressed, this, [this](){
        handleKeyPress(Qt::Key_A);
    });
    connect(ui->sButton, &QPushButton::pressed, this, [this](){
        handleKeyPress(Qt::Key_S);
    });
    connect(ui->dButton, &QPushButton::pressed, this, [this](){
        handleKeyPress(Qt::Key_D);
    });
    connect(ui->waButton, &QPushButton::pressed, this, [this](){
        handleKeyPress(Qt::Key_Q);
    });
    connect(ui->wdButton, &QPushButton::pressed, this, [this](){
        handleKeyPress(Qt::Key_E);
    });

    // 当按下，发送对应的开关信号.
    connect(ui->sprayToggleButton, &QPushButton::toggled, this, [this](bool checked){
        sendToggle(TOGGLE_SPRAY, checked);
    });

    connect(&client, &ControlClient::socketErrorOccurred, this, &MainWindow::onSocketError);
    connect(&client, SIGNAL(connected(QString)), this, SLOT(onConnected(QString)));
    connect(&client, &ControlClient::disconnected, this, &MainWindow::onDisconnected);
    connect(&client, &ControlClient::messageReceived, this, &MainWindow::onMessageReceived);
    setSpeed(MIN_SPEED);
}

MainWindow::~MainWindow() {
    active = false;
    disconnect();
    delete ui;
}

void MainWindow::sendDirection(const btremote::Direction& direction) {
    char command[3] = {direction.commandChar, static_cast<char>(speed + '0'), ';'};
    client.sendMessage(QByteArray(command, 3));
}

void MainWindow::sendToggle(char func, bool status) {
    char command[3] = {func, (status ? '1' : '0'), ';'};
    client.sendMessage(QByteArray(command, 3));
}

void MainWindow::sendCommand(const char* command) {
    client.sendMessage(QByteArray(command, static_cast<int>(strlen(command))));
}

void MainWindow::setSpeed(int value) {
    speed = value;
    ui->speedLabel->setText(QString::number(speed));
}

void MainWindow::openDeviceListDialog() {
    DeviceListDialog dialog(localDevice, this);
    // dialog.setDeviceList(devices);
    if (dialog.exec()) {
        QBluetoothDeviceInfo address = dialog.selectedAddress();
        if (address.isValid()) {
            connectToDevice(address);
        }
    }
}

void MainWindow::connectToDevice(QBluetoothDeviceInfo address) {
    if (currentDevice.isValid()) {
        qDebug() << "disconnecting from " << currentDevice.address();
        currentDevice = QBluetoothDeviceInfo();
        client.stopClient();
        qDebug() << "disconnected.";
    }
    currentDevice = address;
    QBluetoothServiceInfo info;
    info.setDevice(address);
    ui->statusBar->showMessage(QString(u8"Connecting to %1").arg(address.address().toString()));
    qDebug() << "connecting to " << address.address().toString();
    client.startClient(address.address());
}

void MainWindow::reconnect() {
    if (active && currentDevice.isValid() && ui->autoReconnectButton->isChecked()) {
        qDebug() << "reconnecting...";
        connectToDevice(currentDevice);
    }
    reconnectionPending = false;
}

void MainWindow::disconnect() {
    if (currentDevice.isValid()) {
        QBluetoothDeviceInfo temp = currentDevice;
        currentDevice = QBluetoothDeviceInfo();
        client.stopClient();
        currentDevice = temp;
    }
}

void MainWindow::speedUp() {
    if (speed < MAX_SPEED) {
        setSpeed(speed + 1);
    }
}

void MainWindow::speedDown() {
    if (speed > MIN_SPEED) {
        setSpeed(speed - 1);
    }
}

void MainWindow::onSocketError(QString error) {
    ui->statusBar->showMessage(tr("Error: ") + error);
    qDebug() << tr("Error: ") << error;
    onDisconnected();
}

void MainWindow::onConnected(const QString& peer) {
    ui->statusBar->showMessage(tr("Connected to ") + peer);
}

void MainWindow::onDisconnected() {
    if (active) {
        ui->statusBar->showMessage(tr("Disconnected from ") + currentDevice.address().toString());
        if (!reconnectionPending) {
            QTimer::singleShot(RECONNECT_TIMEOUT, this, &MainWindow::reconnect);
        }
        reconnectionPending = true;
    }
}

void MainWindow::onMessageReceived(const QString& peerName, const QString& message) {
    if (message.size() > 0) {
        if (message[0] == QChar('V')) {
            voltage = message.mid(1);
        } else if (message[0] == QChar('T')) {
            temperature = message.mid(1);
        } else if (message[0] == QChar('H')) {
            humidity = message.mid(1);
        }
    }
    QString statusString;
    if (!voltage.isNull()) {
        statusString += tr("电压: ") + voltage;
    }
    if (!temperature.isNull()) {
        statusString += tr("温度: ") + temperature;
    }
    if (!humidity.isNull()) {
        statusString += tr("湿度: ") + humidity;
    }
    ui->statusLabel->setText(statusString);
}

void MainWindow::keyPressEvent(QKeyEvent* ev) {
    if (ev) {
        handleKeyPress(ev->key());
    }
}

void MainWindow::handleKeyPress(int key) {
    switch(key) {
    case Qt::Key_W: case Qt::Key_Up:
        ui->wButton->setChecked(true);
        sendDirection(btremote::Direction::Front);
        break;
    case Qt::Key_A: case Qt::Key_Left:
        ui->aButton->setChecked(true);
        sendDirection(btremote::Direction::Left);
        break;
    case Qt::Key_S: case Qt::Key_Back:
        ui->sButton->setChecked(true);
        sendDirection(btremote::Direction::Back);
        break;
    case Qt::Key_D: case Qt::Key_Right:
        ui->dButton->setChecked(true);
        sendDirection(btremote::Direction::Right);
        break;
    case Qt::Key_Q:
        ui->waButton->setChecked(true);
        sendDirection(btremote::Direction::LeftTurn);
        break;
    case Qt::Key_E:
        ui->wdButton->setChecked(true);
        sendDirection(btremote::Direction::RightTurn);
        break;
    case Qt::Key_Z:
        speedUp();
        break;
    case Qt::Key_X:
        speedDown();
        break;        
    }
}

void MainWindow::handleKeyRelease(int key) {
    switch (key) {
    case Qt::Key_P:
        ui->sprayToggleButton->setChecked(!(ui->sprayToggleButton->isChecked()));
        break;
    case Qt::Key_W: case Qt::Key_Up:
        ui->wButton->setChecked(false);
        sendCommand(btremote::CarStop);
        break;
    case Qt::Key_A: case Qt::Key_Left:
        ui->aButton->setChecked(false);
        sendCommand(btremote::CarStop);
        break;
    case Qt::Key_S: case Qt::Key_Back:
        ui->sButton->setChecked(false);
        sendCommand(btremote::CarStop);
        break;
    case Qt::Key_D: case Qt::Key_Right:
        ui->dButton->setChecked(false);
        sendCommand(btremote::CarStop);
        break;
    case Qt::Key_Q:
        ui->waButton->setChecked(false);
        sendCommand(btremote::CarStop);
        break;
    case Qt::Key_E:
        ui->wdButton->setChecked(false);
        sendCommand(btremote::CarStop);
        break;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent* ev) {
    if(ev->isAutoRepeat()) {
        ev->ignore();
    } else {
        handleKeyRelease(ev->key());
    }
}
