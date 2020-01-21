//
// Created by chiel on 17-01-20.
//

#ifndef AAWIRELESS_BLUETOOTHSOCKET_H
#define AAWIRELESS_BLUETOOTHSOCKET_H

#include <QtNetwork/QAbstractSocket>
#include <QtCore/QSocketNotifier>
#include <boost/shared_ptr.hpp>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sco.h>

#ifndef QPRIVATELINEARBUFFER_BUFFERSIZE
#define QPRIVATELINEARBUFFER_BUFFERSIZE Q_INT64_C(16384)
#endif
#include "qprivatelinearbuffer_p.h"

namespace aawireless {
    namespace bluetooth {
        class BluetoothSocket : QObject {
        Q_OBJECT

        public:
            enum SocketState {
                UnconnectedState = QAbstractSocket::UnconnectedState,
                ServiceLookupState = QAbstractSocket::HostLookupState,
                ConnectingState = QAbstractSocket::ConnectingState,
                ConnectedState = QAbstractSocket::ConnectedState,
                BoundState = QAbstractSocket::BoundState,
                ClosingState = QAbstractSocket::ClosingState,
                ListeningState = QAbstractSocket::ListeningState
            };

            enum SocketError {
                NoSocketError = -2,
                UnknownSocketError = QAbstractSocket::UnknownSocketError, //-1
                RemoteHostClosedError = QAbstractSocket::RemoteHostClosedError, //1
                HostNotFoundError = QAbstractSocket::HostNotFoundError, //2
                ServiceNotFoundError = QAbstractSocket::SocketAddressNotAvailableError, //9
                NetworkError = QAbstractSocket::NetworkError, //7
                UnsupportedProtocolError = 8,
                OperationError = QAbstractSocket::OperationError //19
                //New enums (independent of QAbstractSocket) should be added from 100 onwards
            };

            enum Security {
                NoSecurity = 0x00,
                Authorization = 0x01,
                Authentication = 0x02,
                Encryption = 0x04,
                Secure = 0x08
            };

            BluetoothSocket();

            void connectRfcomm(std::string address, uint8_t channel);
            void connectSCO(std::string address);

            signals:
            void connected();
            void disconnected();
            void error(SocketError error);
            void stateChanged(SocketState state);
            void readyRead();

        private:
            QPrivateLinearBuffer buffer;
            QPrivateLinearBuffer txBuffer;
            Security secFlags = Authorization;
            int socket;
            SocketError socketError = NoSocketError;
            SocketState state = UnconnectedState;
            std::unique_ptr<QSocketNotifier> readNotifier;
            std::unique_ptr<QSocketNotifier> connectWriteNotifier;


            void setSocketError(SocketError _error);

            void setSocketState(SocketState state);

            void readNotify();
            void abort();

            qint64 writeData(const char *data, qint64 maxSize);

            qint64 readData(char *data, qint64 maxSize);

            void close();

            void writeNotify();

            static void inline convertAddress(std::string address, bdaddr_t out);
        };
    }
}


#endif //AAWIRELESS_BLUETOOTHSOCKET_H
