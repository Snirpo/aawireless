//
// Created by chiel on 26-12-19.
//

#include "App.h"
#include <thread>
#include <f1x/aasdk/USB/AOAPDevice.hpp>
#include <f1x/aasdk/TCP/TCPEndpoint.hpp>
#include <aawireless/log/Log.h>

namespace aawireless {
    App::App(boost::asio::io_service &ioService,
             f1x::aasdk::usb::IUSBHub &usbHub,
             boost::asio::ip::tcp::acceptor &acceptor,
             aawireless::bluetooth::BluetoothService &bluetoothService,
             aawireless::connection::ConnectionFactory &connectionFactory)
            : ioService(ioService),
              strand(ioService),
              usbHub(usbHub),
              acceptor(acceptor),
              bluetoothService(bluetoothService),
              connectionFactory(connectionFactory) {
    }

    void App::start() {
        strand.dispatch([this, self = this->shared_from_this()]() {
            AW_LOG(info) << "[App] Starting";

            auto promise = f1x::aasdk::usb::IUSBHub::Promise::defer(strand);
            promise->then([this, self = this->shared_from_this()](f1x::aasdk::usb::DeviceHandle deviceHandle) {
                              usbConnection = connectionFactory.create(deviceHandle);
                          },
                          [this, self = this->shared_from_this()](const f1x::aasdk::error::Error &error) {
                              AW_LOG(error) << "[App] usb hub error: " << error.what();

                              if (error != f1x::aasdk::error::ErrorCode::OPERATION_ABORTED &&
                                  error != f1x::aasdk::error::ErrorCode::OPERATION_IN_PROGRESS) {
                                  try {
                                      // this->waitForDevice();
                                  } catch (...) {
                                      AW_LOG(error)
                                          << "[App] onUSBHubError: exception caused by this->waitForDevice();";
                                  }
                              }
                          });
            usbHub.start(std::move(promise));
            startServerSocket();
            bluetoothService.start();
        });
    }

    void App::stop() {
        strand.dispatch([this, self = this->shared_from_this()]() {
            try {
                bluetoothService.stop();
                acceptor.cancel();
                usbHub.cancel();
            } catch (...) {
                AW_LOG(error) << "[App] stop: exception caused;";
            }
        });

    }

    void App::startServerSocket() {
        strand.dispatch([this, self = this->shared_from_this()]() {
            AW_LOG(info) << "Listening for WIFI clients on port 5000";
            auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioService);
            acceptor.async_accept(
                    *socket,
                    std::bind(&App::handleNewClient, this, socket, std::placeholders::_1)
            );
        });
    }

    void
    App::handleNewClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code &err) {
        strand.dispatch([this, self = this->shared_from_this(), socket, err]() {
            if (!err) {
                AW_LOG(info) << "WIFI Client connected";
                socketConnection = connectionFactory.create(std::move(socket));
            } else {
                AW_LOG(error) << "Socket connection error: " << err;
            }
        });
    }

    void App::tryStartProxy() {
        if (usbConnection != nullptr && socketConnection != nullptr) {
            //TODO: start error handling
            usbConnection->start();
            socketConnection->start();

            startUSBReceive();
            startTCPReceive();
        }

    }

    void App::onUSBReceive(f1x::aasdk::messenger::Message::Pointer message) {
        auto promise = f1x::aasdk::messenger::SendPromise::defer(strand);
        promise->then([]() {}, std::bind(&App::onError, this->shared_from_this(), std::placeholders::_1));

        //TODO: handle messages
        socketConnection->send(message, promise);
    }

    void App::onTCPReceive(f1x::aasdk::messenger::Message::Pointer message) {
        auto promise = f1x::aasdk::messenger::SendPromise::defer(strand);
        promise->then([]() {}, std::bind(&App::onError, this->shared_from_this(), std::placeholders::_1));

        //TODO: handle messages
        usbConnection->send(message, promise);
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
        if (usbConnection != nullptr) {
            usbConnection->stop();
            usbConnection = nullptr;
        }
        if (socketConnection != nullptr) {
            socketConnection->stop();
            socketConnection = nullptr;
        }
    }
}