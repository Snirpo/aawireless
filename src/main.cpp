#include <iostream>
#include <QCoreApplication>
#include "aawireless/bluetooth/BluetoothService.h"

int main(int argc, char *argv[]) {
    QCoreApplication qApplication(argc, argv);

    auto bluetoothService = std::make_shared<aawireless::bluetooth::BluetoothService>();
    bluetoothService->start();

    qApplication.exec();

    return 0;
}
