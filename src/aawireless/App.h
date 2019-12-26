//
// Created by chiel on 26-12-19.
//

#ifndef AAWIRELESS_APP_H
#define AAWIRELESS_APP_H

#include <f1x/aasdk/USB/USBWrapper.hpp>
#include <f1x/aasdk/TCP/TCPWrapper.hpp>
#include <f1x/aasdk/USB/IUSBHub.hpp>
#include <f1x/aasdk/USB/IConnectedAccessoriesEnumerator.hpp>

namespace aawireless {
    class App : public std::enable_shared_from_this<App> {
    public:
        App(boost::asio::io_service &ioService,
            std::shared_ptr<f1x::aasdk::usb::IUSBHub> usbHub);

        void start();

        void stop();

    private:
        boost::asio::io_service &ioService_;
        boost::asio::io_service::strand strand_;
        std::shared_ptr<f1x::aasdk::usb::IUSBHub> usbHub_;
        boost::asio::ip::tcp::acceptor acceptor_;

        void aoapDeviceHandler(f1x::aasdk::usb::DeviceHandle deviceHandle);

        void onUSBHubError(const f1x::aasdk::error::Error &error);

        void startServerSocket();

        void
        handleNewClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code &err);
    };
}


#endif //AAWIRELESS_APP_H
