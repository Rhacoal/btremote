#ifndef DEVICELISTDIALOG_H
#define DEVICELISTDIALOG_H

#include <QBluetoothAddress>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothLocalDevice>
#include <QDialog>
#include <QMap>

namespace Ui {
class DeviceListDialog;
}

class DeviceListDialog : public QDialog
{
    Q_OBJECT

public:
    DeviceListDialog(QBluetoothLocalDevice &device, QWidget *parent = nullptr);
    ~DeviceListDialog();
    QBluetoothDeviceInfo selectedAddress();

private:
    Ui::DeviceListDialog *ui;
    QBluetoothLocalDevice &device;
    QBluetoothDeviceDiscoveryAgent* discoveryAgent;
    QMap<QString, QBluetoothDeviceInfo> mapping;
private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo& info);
};

#endif // DEVICELISTDIALOG_H
