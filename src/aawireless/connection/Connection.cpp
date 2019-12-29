//
// Created by chiel on 28-12-19.
//

#include "Connection.h"
#include <aawireless/log/Log.h>

namespace aawireless {
    namespace connection {
        Connection::Connection(boost::asio::io_context &ioService,
                               std::shared_ptr<f1x::aasdk::messenger::ICryptor> cryptor,
                               std::shared_ptr<f1x::aasdk::transport::ITransport> transport,
                               std::shared_ptr<f1x::aasdk::messenger::IMessageInStream> inStream,
                               std::shared_ptr<f1x::aasdk::messenger::IMessageOutStream> outStream) :
                receiveStrand(ioService),
                sendStrand(ioService),
                cryptor(std::move(cryptor)),
                transport(std::move(transport)),
                inStream(std::move(inStream)),
                outStream(std::move(outStream)) {
        }

        void Connection::start() {
            cryptor->init();
            active = true;
        }

        void Connection::stop() {
            receiveStrand.dispatch([this, self = this->shared_from_this()]() {
                AW_LOG(info) << "[AndroidAutoEntity] stop.";
                active = false;

                try {
                    transport->stop();
                    cryptor->deinit();
                } catch (...) {
                    AW_LOG(error) << "[AndroidAutoEntity] exception in stop.";
                }
            });
        }

        void Connection::receive(f1x::aasdk::messenger::ReceivePromise::Pointer promise) {
            receiveStrand.dispatch([this, self = this->shared_from_this(), promise]() {
                if (active) {
                    inStream->startReceive(std::move(promise));
                }
            });
        }

        void Connection::send(f1x::aasdk::messenger::Message::Pointer message,
                              f1x::aasdk::messenger::SendPromise::Pointer promise) {
            outStream->stream(std::move(message), std::move(promise));
        }
    }
}
