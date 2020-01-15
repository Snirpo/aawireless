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
#include <aawireless/configuration/Configuration.h>
#include <aawireless/wifi/WifiHotspot.h>
#include <aawireless/bluetooth/HFPProxyService.h>

namespace aawireless {
    class App : public std::enable_shared_from_this<App> {
    public:
        App(boost::asio::io_service &ioService,
                f1x::aasdk::usb::IUSBHub::Pointer usbHub,
            boost::asio::ip::tcp::acceptor &acceptor,
            wifi::WifiHotspot &wifiHotspot,
            bluetooth::BluetoothService &bluetoothService,
            bluetooth::HFPProxyService &hfpProxyService,
            connection::ConnectionFactory &connectionFactory,
            configuration::Configuration &configuration);

        void start();

        void stop();

    private:
        boost::asio::io_service &ioService;
        boost::asio::io_service::strand strand;
        f1x::aasdk::usb::IUSBHub::Pointer usbHub;
        boost::asio::ip::tcp::acceptor &acceptor;
        aawireless::wifi::WifiHotspot &wifiHotspot;
        aawireless::bluetooth::BluetoothService &bluetoothService;
        aawireless::bluetooth::HFPProxyService &hfpProxyService;
        aawireless::connection::ConnectionFactory &connectionFactory;
        configuration::Configuration &configuration;

        std::shared_ptr<aawireless::connection::Connection> usbConnection;
        std::shared_ptr<aawireless::connection::Connection> socketConnection;
        bool active = false;

        void startServerSocket();

        void tryStartProxy();

        void startUSBReceive();

        void startTCPReceive();

        void onUSBReceive(f1x::aasdk::messenger::Message::Pointer message);

        void onTCPReceive(f1x::aasdk::messenger::Message::Pointer message);

        void onError(const f1x::aasdk::error::Error &error);

        void cleanup();

        void
        onNewSocket(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code &err);

        void onUSBDeviceConnected(f1x::aasdk::usb::DeviceHandle deviceHandle);

        void onUSBError(const f1x::aasdk::error::Error &error);
    };
}


#endif //AAWIRELESS_APP_H
