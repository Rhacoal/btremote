#include "devicelistdialog.h"
#include "ui_devicelistdialog.h"

#include <QBluetoothLocalDevice>

DeviceListDialog::DeviceListDialog(QBluetoothLocalDevice &device, QWidget *parent) :
    QDialog(parent), ui(new Ui::DeviceListDialog), device(device) {
    ui->setupUi(this);
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    discoveryAgent->setInquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry);
    connect(discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)));
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DeviceListDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DeviceListDialog::reject);
    discoveryAgent->start();
}

DeviceListDialog::~DeviceListDialog() {
    discoveryAgent->stop();
    delete discoveryAgent;
    delete ui;
}

QBluetoothDeviceInfo DeviceListDialog::selectedAddress() {
    auto selection = ui->deviceListWidget->selectedItems();
    if (selection.size() == 1) {
        auto iter = mapping.find(selection[0]->text());
        if (iter != mapping.end()) {
            return *iter;
        }
    }
    return QBluetoothDeviceInfo();
}

void DeviceListDialog::deviceDiscovered(const QBluetoothDeviceInfo& info) {
    QString label = QString("%1 %2").arg(info.address().toString()).arg(info.name());
    QList<QListWidgetItem *> items = ui->deviceListWidget->findItems(label, Qt::MatchExactly);
    if (items.empty()) {
        mapping[label] = info;
        QListWidgetItem *item = new QListWidgetItem(label);
        QBluetoothLocalDevice::Pairing pairingStatus = device.pairingStatus(info.address());
        if (pairingStatus == QBluetoothLocalDevice::Paired || pairingStatus == QBluetoothLocalDevice::AuthorizedPaired )
            item->setTextColor(QColor(Qt::green));
        else
            item->setTextColor(QColor(Qt::black));
        ui->deviceListWidget->addItem(item);
    }
}
