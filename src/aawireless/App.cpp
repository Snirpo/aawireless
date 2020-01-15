//
// Created by chiel on 26-12-19.
//

#include "App.h"
#include <thread>
#include <f1x/aasdk/USB/AOAPDevice.hpp>
#include <f1x/aasdk/TCP/TCPEndpoint.hpp>
#include <aawireless/log/Log.h>
#include <ControlMessageIdsEnum.pb.h>
#include <BluetoothChannelMessageIdsEnum.pb.h>
#include <BluetoothPairingResponseMessage.pb.h>
#include <ServiceDiscoveryResponseMessage.pb.h>
#include <ServiceDiscoveryRequestMessage.pb.h>

namespace aawireless {
    App::App(boost::asio::io_service &ioService,
             f1x::aasdk::usb::IUSBHub::Pointer usbHub,
             boost::asio::ip::tcp::acceptor &acceptor,
             wifi::WifiHotspot &wifiHotspot,
             bluetooth::BluetoothService &bluetoothService,
             bluetooth::HFPProxyService &hfpProxyService,
             connection::ConnectionFactory &connectionFactory,
             configuration::Configuration &configuration)
            : ioService(ioService),
              strand(ioService),
              usbHub(std::move(usbHub)),
              acceptor(acceptor),
              wifiHotspot(wifiHotspot),
              bluetoothService(bluetoothService),
              hfpProxyService(hfpProxyService),
              connectionFactory(connectionFactory),
              configuration(configuration) {
    }

    void App::start() {
        hfpProxyService.start();
        wifiHotspot.start();
        bluetoothService.start();
        strand.dispatch([this, self = this->shared_from_this()]() {
            AW_LOG(info) << "Starting";

            auto promise = f1x::aasdk::usb::IUSBHub::Promise::defer(strand);
            promise->then(std::bind(&App::onUSBDeviceConnected, this->shared_from_this(), std::placeholders::_1),
                          std::bind(&App::onUSBError, this->shared_from_this(), std::placeholders::_1));
            usbHub->start(std::move(promise));
            startServerSocket();
        });
    }

    void App::stop() {
        strand.dispatch([this, self = this->shared_from_this()]() {
            try {
                //TODO: better cleanup
                cleanup();
                bluetoothService.stop();
                hfpProxyService.stop();
                acceptor.cancel();
                usbHub->cancel();
            } catch (...) {
                AW_LOG(error) << "stop: exception caused;";
            }
        });

    }

    void App::startServerSocket() {
        strand.dispatch([this, self = this->shared_from_this()]() {
            AW_LOG(info) << "Listening for WIFI clients on port 5000";
            auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioService);
            acceptor.async_accept(
                    *socket,
                    std::bind(&App::onNewSocket, this, socket, std::placeholders::_1)
            );
        });
    }

    void
    App::onNewSocket(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code &err) {
        strand.dispatch([this, self = this->shared_from_this(), socket, err]() {
            if (!err) {
                AW_LOG(info) << "WIFI Client connected";
                socketConnection = connectionFactory.create(std::move(socket));
                tryStartProxy();
            } else {
                AW_LOG(error) << "Socket connection error: " << err;
            }
        });
    }

    void App::tryStartProxy() {
        if (usbConnection != nullptr && socketConnection != nullptr) {
            active = true;

            //TODO: start error handling
            usbConnection->start();
            socketConnection->start();

            startUSBReceive();
            startTCPReceive();
        }

    }

    void App::onUSBReceive(f1x::aasdk::messenger::Message::Pointer message) {
        if (active) {
            auto promise = f1x::aasdk::messenger::SendPromise::defer(strand);
            promise->then([]() {}, std::bind(&App::onError, this->shared_from_this(), std::placeholders::_1));

            if (message->getChannelId() == f1x::aasdk::messenger::ChannelId::CONTROL) {
                f1x::aasdk::messenger::MessageId messageId(message->getPayload());
                f1x::aasdk::common::DataConstBuffer payload(message->getPayload(), messageId.getSizeOf());
                if (messageId.getId() == f1x::aasdk::proto::ids::ControlMessage::SERVICE_DISCOVERY_RESPONSE) {
                    f1x::aasdk::proto::messages::ServiceDiscoveryResponse response;
                    response.ParseFromArray(payload.cdata, payload.size);

                    for (auto &channel : *response.mutable_channels()) {
                        if (channel.channel_id() ==
                            static_cast<uint32_t>(f1x::aasdk::messenger::ChannelId::BLUETOOTH)) {
                            f1x::aasdk::proto::data::BluetoothChannel *bluetoothChannel = channel.mutable_bluetooth_channel();
                            bluetoothChannel->set_adapter_address(bluetoothService.getAddress()); //TODO: set address
                            bluetoothChannel->clear_supported_pairing_methods();
                            bluetoothChannel->add_supported_pairing_methods(
                                    f1x::aasdk::proto::enums::BluetoothPairingMethod_Enum_HFP);
                            bluetoothChannel->add_supported_pairing_methods(
                                    f1x::aasdk::proto::enums::BluetoothPairingMethod_Enum_A2DP);
                        }
                    }

                    socketConnection->send(message, promise);
                    startUSBReceive();
                    return;
                }
            }

            //TODO: handle messages
            socketConnection->send(message, promise);
            startUSBReceive();
        }
    }

    void App::onTCPReceive(f1x::aasdk::messenger::Message::Pointer message) {
        if (active) {
            auto promise = f1x::aasdk::messenger::SendPromise::defer(strand);
            promise->then([]() {}, std::bind(&App::onError, this->shared_from_this(), std::placeholders::_1));

            if (message->getChannelId() == f1x::aasdk::messenger::ChannelId::BLUETOOTH) {
                f1x::aasdk::messenger::MessageId messageId(message->getPayload());
                f1x::aasdk::common::DataConstBuffer payload(message->getPayload(), messageId.getSizeOf());
                if (messageId.getId() == f1x::aasdk::proto::ids::BluetoothChannelMessage::PAIRING_REQUEST) {
                    f1x::aasdk::proto::messages::BluetoothPairingResponse response;
                    //TODO: not hardcoded?
                    response.set_already_paired(true);
                    response.set_status(f1x::aasdk::proto::enums::BluetoothPairingStatus::OK);
                    auto msg(std::make_shared<f1x::aasdk::messenger::Message>(
                            f1x::aasdk::messenger::ChannelId::BLUETOOTH,
                            f1x::aasdk::messenger::EncryptionType::ENCRYPTED,
                            f1x::aasdk::messenger::MessageType::SPECIFIC));
                    msg->insertPayload(f1x::aasdk::messenger::MessageId(
                            f1x::aasdk::proto::ids::BluetoothChannelMessage::PAIRING_RESPONSE).getData());
                    msg->insertPayload(response);

                    socketConnection->send(std::move(msg), std::move(promise));
                    startTCPReceive();
                    return;
                }
            }

            usbConnection->send(std::move(message), std::move(promise));
            startTCPReceive();
        }
    }

    void App::startUSBReceive() {
        auto receivePromise = f1x::aasdk::messenger::ReceivePromise::defer(strand);
        receivePromise->then(std::bind(&App::onUSBReceive, this->shared_from_this(), std::placeholders::_1),
                             std::bind(&App::onError, this->shared_from_this(), std::placeholders::_1));
        usbConnection->receive(receivePromise);
    }

    void App::startTCPReceive() {
        auto receivePromise = f1x::aasdk::messenger::ReceivePromise::defer(strand);
        receivePromise->then(std::bind(&App::onTCPReceive, this->shared_from_this(), std::placeholders::_1),
                             std::bind(&App::onError, this->shared_from_this(), std::placeholders::_1));
        socketConnection->receive(receivePromise);
    }

    void App::onError(const f1x::aasdk::error::Error &error) {
        cleanup();
        AW_LOG(error) << "Connection error: " << error.getNativeCode();
    }

    void App::cleanup() {
        active = false;
        if (usbConnection != nullptr) {
            usbConnection->stop();
            usbConnection = nullptr;
        }
        if (socketConnection != nullptr) {
            socketConnection->stop();
            socketConnection = nullptr;
        }
    }

    void App::onUSBDeviceConnected(f1x::aasdk::usb::DeviceHandle deviceHandle) {
        usbConnection = connectionFactory.create(deviceHandle);
        tryStartProxy();
    }

    void App::onUSBError(const f1x::aasdk::error::Error &error) {
        AW_LOG(error) << "usb hub error: " << error.what();

        if (error != f1x::aasdk::error::ErrorCode::OPERATION_ABORTED &&
            error != f1x::aasdk::error::ErrorCode::OPERATION_IN_PROGRESS) {
            try {
                // this->waitForDevice();
            } catch (...) {
                AW_LOG(error)
                    << "onUSBHubError: exception caused by this->waitForDevice();";
            }
        }
    }
}