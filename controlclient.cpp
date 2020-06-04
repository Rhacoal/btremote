/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "controlclient.h"

#include <QtCore/qmetaobject.h>

const QBluetoothUuid ControlClient::SPP(QLatin1String("00001101-0000-1000-8000-00805F9834FB"));

ControlClient::ControlClient(QObject *parent)
    :   QObject(parent), socket(nullptr)
{
}

ControlClient::~ControlClient() {
    stopClient();
}

//! [startClient]
void ControlClient::startClient(const QBluetoothServiceInfo &remoteService) {
    if (socket)
        return;

    // Connect to service
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    qDebug() << "Create socket";
    socket->connectToService(remoteService);
    qDebug() << "ConnectToService done";

    connect(socket, &QBluetoothSocket::readyRead, this, &ControlClient::readSocket);
    connect(socket, &QBluetoothSocket::connected, this, QOverload<>::of(&ControlClient::connected));
    connect(socket, &QBluetoothSocket::disconnected, this, &ControlClient::disconnected);
    connect(socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error),
            this, &ControlClient::onSocketErrorOccurred);

}

void ControlClient::startClient(const QBluetoothAddress& remoteAddress)  {
    if (socket)
        return;

    // Connect to service
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    qDebug() << "Create socket";
    socket->connectToService(remoteAddress, SPP, QIODevice::ReadWrite);
    qDebug() << "ConnectToService done";

    connect(socket, &QBluetoothSocket::readyRead, this, &ControlClient::readSocket);
    connect(socket, &QBluetoothSocket::connected, this, QOverload<>::of(&ControlClient::connected));
    connect(socket, &QBluetoothSocket::disconnected, this, &ControlClient::disconnected);
    connect(socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error),
            this, &ControlClient::onSocketErrorOccurred);
}
//! [startClient]

//! [stopClient]
void ControlClient::stopClient() {
    if (socket) {
        socket->abort();
        socket->close(); // 为了避免一个 assertion failure, 调用一下强制设置 flag
        disconnect(socket, &QBluetoothSocket::readyRead, this, &ControlClient::readSocket);
        disconnect(socket, &QBluetoothSocket::connected, this, QOverload<>::of(&ControlClient::connected));
        disconnect(socket, &QBluetoothSocket::disconnected, this, &ControlClient::disconnected);
        disconnect(socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error),
                this, &ControlClient::onSocketErrorOccurred);
        delete socket;
        socket = nullptr;
    }
}
//! [stopClient]

//! [readSocket]
void ControlClient::readSocket() {
    if (!socket)
        return;
    while (socket->canReadLine()) {
        QByteArray line = socket->readLine();
        emit messageReceived(socket->peerName(),
                             QString::fromUtf8(line.constData(), line.length()));
    }
}
//! [readSocket]

//! [sendMessage]
void ControlClient::sendMessage(const QByteArray& data) {
    qDebug() << QString(data);
    if (!socket) {
        return;
    }
    socket->write(data);
}
//! [sendMessage]

void ControlClient::onSocketErrorOccurred(QBluetoothSocket::SocketError error) {
    if (error == QBluetoothSocket::NoSocketError)
        return;

    QMetaEnum metaEnum = QMetaEnum::fromType<QBluetoothSocket::SocketError>();
    if (metaEnum.isValid()) {
        QString errorString;// = socket->peerAddress().toString();
        //errorString += QLatin1Char(' ');
        errorString += metaEnum.valueToKey(error);
        errorString += QLatin1String(" occurred");
        emit socketErrorOccurred(errorString);
        return;
    }
    emit socketErrorOccurred(QString::number(error));

}

//! [connected]
void ControlClient::connected() {
    emit connected(socket->peerName());
}
//! [connected]
