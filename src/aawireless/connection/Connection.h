//
// Created by chiel on 28-12-19.
//

#ifndef AAWIRELESS_CONNECTION_H
#define AAWIRELESS_CONNECTION_H


#include <boost/asio/io_service.hpp>
#include <f1x/aasdk/Messenger/MessageInStream.hpp>
#include <f1x/aasdk/Messenger/MessageOutStream.hpp>

namespace aawireless {
    namespace connection {
        class Connection : std::enable_shared_from_this<Connection> {
        public:
            Connection(boost::asio::io_context &ioService,
                       std::shared_ptr<f1x::aasdk::messenger::ICryptor> cryptor,
                       std::shared_ptr<f1x::aasdk::transport::ITransport> transport,
                       std::shared_ptr<f1x::aasdk::messenger::IMessageInStream> inStream,
                       std::shared_ptr<f1x::aasdk::messenger::IMessageOutStream> outStream);

            void start();

            void stop();

            void
            send(f1x::aasdk::messenger::Message::Pointer message, f1x::aasdk::messenger::SendPromise::Pointer promise);

            void receive(f1x::aasdk::messenger::ReceivePromise::Pointer promise);

        private:
            boost::asio::io_service::strand receiveStrand;
            boost::asio::io_service::strand sendStrand;
            std::shared_ptr<f1x::aasdk::messenger::ICryptor> cryptor;
            std::shared_ptr<f1x::aasdk::transport::ITransport> transport;
            std::shared_ptr<f1x::aasdk::messenger::IMessageInStream> inStream;
            std::shared_ptr<f1x::aasdk::messenger::IMessageOutStream> outStream;
            bool active = false;

            void handleMessage(const f1x::aasdk::messenger::Message::Pointer message,
                               f1x::aasdk::messenger::ReceivePromise::Pointer promise);

            void onHandshake(const f1x::aasdk::common::DataConstBuffer &payload,
                             f1x::aasdk::io::Promise<void>::Pointer promise);

            void onHandshakeError(const f1x::aasdk::error::Error &error);
        };
    }
}

#endif //AAWIRELESS_CONNECTION_H
