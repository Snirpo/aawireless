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
             std::shared_ptr<f1x::aasdk::usb::IUSBHub> usbHub)
            : ioService_(ioService),
              strand_(ioService_),
              usbHub_(std::move(usbHub)),
              acceptor_(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 5000)) {
    }

    void App::start() {
        strand_.dispatch([this, self = this->shared_from_this()]() {
            AW_LOG(info) << "[App] Waiting for device...";

            auto promise = f1x::aasdk::usb::IUSBHub::Promise::defer(strand_);
            promise->then(std::bind(&App::aoapDeviceHandler, this->shared_from_this(), std::placeholders::_1),
                          std::bind(&App::onUSBHubError, this->shared_from_this(), std::placeholders::_1));
            usbHub_->start(std::move(promise));
            startServerSocket();
        });
    }

    void App::stop() {
        strand_.dispatch([this, self = this->shared_from_this()]() {
            try {
                usbHub_->cancel();
            } catch (...) {
                AW_LOG(error) << "[App] stop: exception caused by usbHub_->cancel();";
            }
        });

    }

    void App::aoapDeviceHandler(f1x::aasdk::usb::DeviceHandle deviceHandle) {
        AW_LOG(info) << "[App] Device connected.";
    }

    void App::startServerSocket() {
        strand_.dispatch([this, self = this->shared_from_this()]() {
            AW_LOG(info) << "Listening for WIFI clients on port 5000";
            auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioService_);
            acceptor_.async_accept(
                    *socket,
                    std::bind(&App::handleNewClient, this, socket, std::placeholders::_1)
            );
        });
    }

    void
    App::handleNewClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code &err) {
        AW_LOG(info) << "WIFI Client connected";
        if (!err) {
            //start(std::move(socket));
        }
    }

    void App::onUSBHubError(const f1x::aasdk::error::Error &error) {
        AW_LOG(error) << "[App] usb hub error: " << error.what();

        if (error != f1x::aasdk::error::ErrorCode::OPERATION_ABORTED &&
            error != f1x::aasdk::error::ErrorCode::OPERATION_IN_PROGRESS) {
            try {
                // this->waitForDevice();
            } catch (...) {
                AW_LOG(error) << "[App] onUSBHubError: exception caused by this->waitForDevice();";
            }
        }
    }

}