//
// Created by chiel on 24-12-19.
//

#include <QtBluetooth/QBluetoothServiceInfo>
#include "BluetoothService.h"
#include <aawireless/log/Log.h>
#include <WifiInfoRequestMessage.pb.h>
#include <QtCore/QDataStream>
#include <iomanip>
#include <WifiInfoResponseMessage.pb.h>
#include <WifiSecurityResponseMessage.pb.h>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

namespace aawireless {
    namespace bluetooth {
        BluetoothService::BluetoothService(aawireless::configuration::Configuration &configuration,
                                           aawireless::database::Database &database,
                                           std::string password) :
                server(QBluetoothServiceInfo::RfcommProtocol),
                configuration(configuration),
                database(database),
                password(password) {
            connect(&server, &QBluetoothServer::newConnection, this,
                    &BluetoothService::onClientConnected);
        }

        void BluetoothService::start() {
            AW_LOG(info) << "Start listening for bluetooth connections";
            localDevice.powerOn();
            localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);

            server.listen(localDevice.address());
            registerService(server.serverPort());

            if (!database.getLastBluetoothDevice().empty()) {
                connectDevice(database.getLastBluetoothDevice());
            }
        }

        void BluetoothService::connectDevice(std::string address) {
            AW_LOG(info) << "Connecting to " << address;
            std::replace(address.begin(), address.end(), ':', '_');
            QDBusInterface iface("org.bluez",
                                 std::string("/org/bluez/hci0/dev_").append(address).c_str(),
                                 "org.bluez.Device1",
                                 QDBusConnection::systemBus());
            if (iface.isValid()) {
                QDBusReply<void> reply = iface.call("Connect");
                if (!reply.isValid()) {
                    AW_LOG(error) << reply.error().message().toStdString();
                }
            } else {
                AW_LOG(error) << "Invalid interface" << iface.lastError().message().toStdString();
            }
        }

        void BluetoothService::stop() {
            serviceInfo.unregisterService();
        }

        void BluetoothService::onClientConnected() {
            if (socket != nullptr) {
                socket->deleteLater();
            }

            socket = server.nextPendingConnection();

            database.setLastBluetoothDevice(socket->peerAddress().toString().toStdString());
            database.save();

            if (socket != nullptr) {
                AW_LOG(info) << "[AndroidBluetoothServer] rfcomm client connected, peer name: "
                             << socket->peerName().toStdString();

                connect(socket, &QBluetoothSocket::readyRead, this, &BluetoothService::readSocket);
//                    connect(socket, &QBluetoothSocket::disconnected, this,
//                            QOverload<>::of(&ChatServer::clientDisconnected));

                f1x::aasdk::proto::messages::WifiInfoRequest request;
                request.set_ip_address(configuration.wifiIpAddress);
                request.set_port(configuration.wifiPort);

                sendMessage(request, 1);
            } else {
                AW_LOG(error) << "received null socket during client connection.";
            }
        }

        void BluetoothService::readSocket() {
            buffer += socket->readAll();

            AW_LOG(info) << "Received message";

            if (buffer.length() < 4) {
                AW_LOG(debug) << "Not enough data, waiting for more";
                return;
            }

            QDataStream stream(buffer);
            uint16_t length;
            stream >> length;

            if (buffer.length() < length + 4) {
                AW_LOG(info) << "Not enough data, waiting for more: " << buffer.length();
                return;
            }

            uint16_t messageId;
            stream >> messageId;

            //OPENAUTO_LOG(info) << "[AndroidBluetoothServer] " << length << " " << messageId;

            switch (messageId) {
                case 1:
                    handleWifiInfoRequest(buffer, length);
                    break;
                case 2:
                    handleWifiSecurityRequest(buffer, length);
                    break;
                case 7:
                    handleWifiInfoRequestResponse(buffer, length);
                    break;
                default: {
                    std::stringstream ss;
                    ss << std::hex << std::setfill('0');
                    for (auto &&val : buffer) {
                        ss << std::setw(2) << static_cast<unsigned>(val);
                    }
                    AW_LOG(info) << "Unknown message: " << messageId;
                    AW_LOG(info) << ss.str();
                    break;
                }
            }

            buffer = buffer.mid(length + 4);
        }

        void BluetoothService::handleWifiInfoRequest(QByteArray &buffer, uint16_t length) {
            f1x::aasdk::proto::messages::WifiInfoRequest msg;
            msg.ParseFromArray(buffer.data() + 4, length);
            AW_LOG(info) << "WifiInfoRequest: " << msg.DebugString();

            f1x::aasdk::proto::messages::WifiInfoResponse response;
            response.set_ip_address(configuration.wifiIpAddress);
            response.set_port(configuration.wifiPort);
            response.set_status(f1x::aasdk::proto::messages::WifiInfoResponse_Status_STATUS_SUCCESS);

            sendMessage(response, 7);
        }

        void BluetoothService::handleWifiSecurityRequest(QByteArray &buffer, uint16_t length) {
            f1x::aasdk::proto::messages::WifiSecurityReponse response;

            response.set_ssid(configuration.wifiSSID);
            response.set_bssid(configuration.wifiBSSID);
            response.set_key(configuration.wifiPassphrase);
            response.set_security_mode(
                    f1x::aasdk::proto::messages::WifiSecurityReponse_SecurityMode_WPA2_PERSONAL); //TODO: make configurable?
            response.set_access_point_type(f1x::aasdk::proto::messages::WifiSecurityReponse_AccessPointType_STATIC);

            sendMessage(response, 3);
        }

        void BluetoothService::sendMessage(google::protobuf::Message &message, uint16_t type) {
            int byteSize = message.ByteSize();
            QByteArray out(byteSize + 4, 0);
            QDataStream ds(&out, QIODevice::ReadWrite);
            ds << (uint16_t) byteSize;
            ds << type;
            message.SerializeToArray(out.data() + 4, byteSize);

            std::stringstream ss;
            ss << std::hex << std::setfill('0');
            for (auto &&val : out) {
                ss << std::setw(2) << static_cast<unsigned>(val);
            }
            AW_LOG(info) << "Writing message: " << ss.str();

            auto written = socket->write(out);
            if (written > -1) {
                AW_LOG(info) << "Bytes written: " << written;
            } else {
                AW_LOG(info) << "Could not write data";
            }
        }

        void BluetoothService::handleWifiInfoRequestResponse(QByteArray &buffer, uint16_t length) {
            f1x::aasdk::proto::messages::WifiInfoResponse msg;
            msg.ParseFromArray(buffer.data() + 4, length);
            AW_LOG(info) << "WifiInfoResponse: " << msg.DebugString();
        }

        void BluetoothService::registerService(quint16 port) {
            const QBluetoothUuid serviceUuid(QLatin1String("4de17a00-52cb-11e6-bdf4-0800200c9a66"));

            QBluetoothServiceInfo::Sequence classId;
            classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
            serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList, classId);
            classId.prepend(QVariant::fromValue(serviceUuid));
            serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);
            serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, "AAWireless Bluetooth Service");
            serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription,
                                     "AndroidAuto WiFi projection automatic setup");
            serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider, "AAWireless");
            serviceInfo.setServiceUuid(serviceUuid);

            QBluetoothServiceInfo::Sequence publicBrowse;
            publicBrowse << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));
            serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList, publicBrowse);

            QBluetoothServiceInfo::Sequence protocolDescriptorList;
            QBluetoothServiceInfo::Sequence protocol;
            protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap));
            protocolDescriptorList.append(QVariant::fromValue(protocol));
            protocol.clear();
            protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
                     << QVariant::fromValue(port);
            protocolDescriptorList.append(QVariant::fromValue(protocol));
            serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);

            serviceInfo.registerService(localDevice.address());
        }

        std::string BluetoothService::getAddress() {
            return localDevice.address().toString().toStdString();
        }
    }
}
