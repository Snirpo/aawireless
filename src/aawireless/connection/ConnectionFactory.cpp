//
// Created by chiel on 28-12-19.
//

#include "ConnectionFactory.h"
#include <f1x/aasdk/Transport/SSLWrapper.hpp>
#include <f1x/aasdk/Messenger/Cryptor.hpp>
#include <f1x/aasdk/Messenger/Messenger.hpp>
#include <f1x/aasdk/Messenger/MessageInStream.hpp>
#include <f1x/aasdk/Messenger/MessageOutStream.hpp>
#include <f1x/aasdk/Transport/TCPTransport.hpp>
#include <f1x/aasdk/TCP/TCPEndpoint.hpp>
#include <f1x/aasdk/USB/IAOAPDevice.hpp>
#include <f1x/aasdk/Transport/USBTransport.hpp>
#include <f1x/aasdk/USB/AOAPDevice.hpp>

namespace aawireless {
    namespace connection {
        ConnectionFactory::ConnectionFactory(boost::asio::io_service &ioService,
                                             f1x::aasdk::tcp::TCPWrapper &tcpWrapper,
                                             f1x::aasdk::usb::USBWrapper &usbWrapper) :
                ioService(ioService), tcpWrapper(tcpWrapper), usbWrapper(usbWrapper) {
        }

        std::shared_ptr<Connection>
        ConnectionFactory::create(f1x::aasdk::usb::DeviceHandle deviceHandle) {
            auto aoapDevice(f1x::aasdk::usb::AOAPDevice::create(usbWrapper, ioService, deviceHandle));
            auto transport(std::make_shared<f1x::aasdk::transport::USBTransport>(ioService, std::move(aoapDevice)));
            return create(std::move(transport));
        }

        std::shared_ptr<Connection> ConnectionFactory::create(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
            auto endpoint(std::make_shared<f1x::aasdk::tcp::TCPEndpoint>(tcpWrapper, std::move(socket)));
            auto transport(std::make_shared<f1x::aasdk::transport::TCPTransport>(ioService, std::move(endpoint)));
            return create(std::move(transport));
        }

        std::shared_ptr<Connection>
        ConnectionFactory::create(std::shared_ptr<f1x::aasdk::transport::ITransport> transport) {
            auto sslWrapper(std::make_shared<f1x::aasdk::transport::SSLWrapper>());
            auto cryptor(std::make_shared<f1x::aasdk::messenger::Cryptor>(std::move(sslWrapper)));

            auto inStream(std::make_shared<f1x::aasdk::messenger::MessageInStream>(ioService, transport, cryptor));
            auto outStream(std::make_shared<f1x::aasdk::messenger::MessageOutStream>(ioService, transport,
                                                                                     cryptor));

            return std::make_shared<Connection>(ioService,
                                                std::move(cryptor),
                                                std::move(transport),
                                                std::move(inStream),
                                                std::move(outStream));
        }
    }
}
