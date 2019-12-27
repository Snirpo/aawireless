//
// Created by chiel on 26-12-19.
//

#ifndef AAWIRELESS_APP_H
#define AAWIRELESS_APP_H

#include <f1x/aasdk/USB/USBWrapper.hpp>
#include <f1x/aasdk/TCP/TCPWrapper.hpp>
#include <f1x/aasdk/USB/IUSBHub.hpp>
#include <f1x/aasdk/USB/IConnectedAccessoriesEnumerator.hpp>
#include <aawireless/bluetooth/BluetoothService.h>
#include <aawireless/connection/ConnectionFactory.h>

namespace aawireless {
    class App : public std::enable_shared_from_this<App> {
    public:
        App(boost::asio::io_service &ioService,
            f1x::aasdk::usb::IUSBHub &usbHub,
            boost::asio::ip::tcp::acceptor &acceptor,
            aawireless::bluetooth::BluetoothService &bluetoothService,
            aawireless::connection::ConnectionFactory &connectionFactory);

        void start();

        void stop();

    private:
        boost::asio::io_service &ioService;
        boost::asio::io_service::strand strand;
        f1x::aasdk::usb::IUSBHub &usbHub;
        boost::asio::ip::tcp::acceptor &acceptor;
        aawireless::bluetooth::BluetoothService &bluetoothService;
        aawireless::connection::ConnectionFactory &connectionFactory;

        std::shared_ptr<aawireless::connection::Connection> usbConnection;
        std::shared_ptr<aawireless::connection::Connection> socketConnection;

        void startServerSocket();
        void tryCreateConnection();

        void
        handleNewClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code &err);
    };
}


#endif //AAWIRELESS_APP_H
