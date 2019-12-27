//
// Created by chiel on 28-12-19.
//

#ifndef AAWIRELESS_CONNECTIONFACTORY_H
#define AAWIRELESS_CONNECTIONFACTORY_H


#include <boost/asio/io_service.hpp>
#include <aawireless/connection/Connection.h>
#include <f1x/aasdk/TCP/TCPWrapper.hpp>
#include <f1x/aasdk/USB/USBWrapper.hpp>
#include <f1x/aasdk/Transport/ITransport.hpp>
#include <f1x/aasdk/USB/IAOAPDevice.hpp>
#include <f1x/aasdk/TCP/ITCPEndpoint.hpp>

namespace aawireless {
    namespace connection {
        class ConnectionFactory {
        public:
            ConnectionFactory(boost::asio::io_service &ioService,
                              f1x::aasdk::tcp::TCPWrapper &tcpWrapper,
                              f1x::aasdk::usb::USBWrapper &usbWrapper);

            std::shared_ptr<Connection> create(f1x::aasdk::usb::DeviceHandle deviceHandle);

            std::shared_ptr<Connection> create(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

        private:
            boost::asio::io_service &ioService;
            f1x::aasdk::tcp::TCPWrapper &tcpWrapper;
            f1x::aasdk::usb::USBWrapper &usbWrapper;

            std::shared_ptr<Connection> create(std::shared_ptr<f1x::aasdk::transport::ITransport> transport);
        };
    }
}

#endif //AAWIRELESS_CONNECTIONFACTORY_H
