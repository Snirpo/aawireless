//
// Created by chiel on 24-12-19.
//

#ifndef AAWIRELESS_BLUETOOTHSERVICE_H
#define AAWIRELESS_BLUETOOTHSERVICE_H

#include <QtBluetooth/QBluetoothServer>
#include <google/protobuf/message.h>
#include <QtBluetooth/QBluetoothLocalDevice>
#include <aawireless/configuration/Configuration.h>

namespace aawireless {
    namespace bluetooth {
        class BluetoothService : public QObject {
        Q_OBJECT

        public:
            BluetoothService(aawireless::configuration::Configuration &configuration);

            void start();

            void stop();

            std::string getAddress();

        private slots:

            void onClientConnected();

        private:
            QBluetoothLocalDevice localDevice;
            QBluetoothServiceInfo serviceInfo;
            QBluetoothServer server;
            QByteArray buffer;
            QBluetoothSocket *socket = nullptr;
            aawireless::configuration::Configuration &configuration;

            void readSocket();

            void sendMessage(google::protobuf::Message &message, uint16_t type);

            void handleWifiInfoRequest(QByteArray &buffer, uint16_t length);

            void handleWifiSecurityRequest(QByteArray &buffer, uint16_t length);

            void handleWifiInfoRequestResponse(QByteArray &buffer, uint16_t length);

            void registerService(quint16 port);
        };
    }
}


#endif //AAWIRELESS_BLUETOOTHSERVICE_H
