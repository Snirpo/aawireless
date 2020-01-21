//
// Created by chiel on 17-01-20.
//

#include "BluetoothSocket.h"
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <QtCore/QSocketNotifier>
#include <qplatformdefs.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <aawireless/log/Log.h>

namespace aawireless {
    namespace bluetooth {

        BluetoothSocket::BluetoothSocket() {
            readNotifier = std::make_unique<QSocketNotifier>(socket, QSocketNotifier::Read);
            connect(readNotifier.get(), SIGNAL(activated(int)), this, SLOT(readNotify()));
            connectWriteNotifier = std::make_unique<QSocketNotifier>(socket, QSocketNotifier::Write);
            connect(connectWriteNotifier.get(), SIGNAL(activated(int)), this, SLOT(writeNotify));
            connectWriteNotifier->setEnabled(false);
            readNotifier->setEnabled(false);
        }


        //TODO: do something with security?
        //void BluetoothSocket::connectToAddress() {
            // apply preferred security level
            // ignore QBluetooth::Authentication -> not used anymore by kernel
//            struct bt_security security;
//            memset(&security, 0, sizeof(security));
//
//            if (secFlags & Security::Authorization)
//                security.level = BT_SECURITY_LOW;
//            if (secFlags & Security::Encryption)
//                security.level = BT_SECURITY_MEDIUM;
//            if (secFlags & Security::Secure)
//                security.level = BT_SECURITY_HIGH;
//
//            if (setsockopt(socket, SOL_BLUETOOTH, BT_SECURITY,
//                           &security, sizeof(security)) != 0) {
//                AW_LOG(error) << "Cannot set connection security level, closing socket for safety"
//                              << qt_error_string(errno).toStdString();
//                socketError = UnknownSocketError;
//                return;
//            }
      //  }

        void BluetoothSocket::writeNotify() {
            if (state == ConnectingState) {
                int errorno, len;
                len = sizeof(errorno);
                ::getsockopt(socket, SOL_SOCKET, SO_ERROR, &errorno, (socklen_t *) &len);
                if (errorno) {
                    AW_LOG(error) << "Could not complete connection to socket " << qt_error_string(errorno).toStdString();
                    setSocketError(UnknownSocketError);
                    return;
                }

                setSocketState(ConnectedState);

                connectWriteNotifier->setEnabled(false);
            } else {
                if (txBuffer.size() == 0) {
                    connectWriteNotifier->setEnabled(false);
                    return;
                }

                char buf[1024];

                int size = txBuffer.read(buf, 1024);
                //TODO: int writtenBytes = qt_safe_write(socket, buf, size);
                int writtenBytes = 0;
                if (writtenBytes < 0) {
                    switch (errno) {
                        case EAGAIN:
                            writtenBytes = 0;
                            txBuffer.ungetBlock(buf, size);
                            break;
                        default:
                            // every other case returns error
                            setSocketError(NetworkError);
                            break;
                    }
                } else {
                    if (writtenBytes < size) {
                        // add remainder back to buffer
                        char *remainder = buf + writtenBytes;
                        txBuffer.ungetBlock(remainder, size - writtenBytes);
                    }
                }

                if (txBuffer.size()) {
                    connectWriteNotifier->setEnabled(true);
                } else if (state == ClosingState) {
                    connectWriteNotifier->setEnabled(false);
                    close();
                }
            }
        }

        void BluetoothSocket::readNotify() {
            char *writePointer = buffer.reserve(QPRIVATELINEARBUFFER_BUFFERSIZE);
            int readFromDevice = ::read(socket, writePointer, QPRIVATELINEARBUFFER_BUFFERSIZE);
            buffer.chop(QPRIVATELINEARBUFFER_BUFFERSIZE - (readFromDevice < 0 ? 0 : readFromDevice));
            if (readFromDevice <= 0) {
                int errsv = errno;
                readNotifier->setEnabled(false);
                connectWriteNotifier->setEnabled(false);
                AW_LOG(error) << "Could not read from device " << qt_error_string(errsv).toStdString();
                if (errsv == EHOSTDOWN)
                    setSocketError(HostNotFoundError);
                else if (errsv == ECONNRESET)
                    setSocketError(RemoteHostClosedError);
                else
                    setSocketError(UnknownSocketError);
            } else {
                emit readyRead();
            }
        }

        void BluetoothSocket::abort() {
            readNotifier = nullptr;
            connectWriteNotifier = nullptr;

            // We don't transition through Closing for abort, so
            // we don't call disconnectFromService or
            // QBluetoothSocket::close
            QT_CLOSE(socket);
            socket = -1;

            setSocketState(UnconnectedState);
            emit disconnected();
        }

        qint64 BluetoothSocket::writeData(const char *data, qint64 maxSize) {
            if (state != ConnectedState) {
                AW_LOG(error) << "Cannot write while not connected";
                setSocketError(OperationError);
                return -1;
            }

            if (!connectWriteNotifier)
                return -1;

            if (txBuffer.size() == 0) {
                connectWriteNotifier->setEnabled(true);
                QMetaObject::invokeMethod(this, "writeNotify", Qt::QueuedConnection);
            }

            char *txbuf = txBuffer.reserve(maxSize);
            memcpy(txbuf, data, maxSize);

            return maxSize;
        }

        qint64 BluetoothSocket::readData(char *data, qint64 maxSize) {
            if (state != ConnectedState) {
                AW_LOG(error) << "Cannot read while not connected";
                setSocketError(OperationError);
                return -1;
            }

            if (!buffer.isEmpty()) {
                int i = buffer.read(data, maxSize);
                return i;
            }

            return 0;
        }

        void BluetoothSocket::close() {
            if (txBuffer.size() > 0)
                connectWriteNotifier->setEnabled(true);
            else
                abort();
        }

        void BluetoothSocket::setSocketError(SocketError _error) {
            socketError = _error;
            emit error(socketError);
        }

        void BluetoothSocket::setSocketState(SocketState _state) {
            const auto old = state;
            if (_state == old)
                return;
            state = _state;

            emit stateChanged(state);
            if (state == ConnectedState) {
                emit connected();
            } else if ((old == ConnectedState
                        || old == ClosingState)
                       && state == UnconnectedState) {
                emit disconnected();
            }
            if (state == ListeningState) {
                // TODO: look at this, is this really correct?
                // if we're a listening socket we can't handle connects?
                if (readNotifier) {
                    readNotifier->setEnabled(false);
                }
            }
        }

        void inline BluetoothSocket::convertAddress(std::string address, bdaddr_t out) {
            const char *src_addr = address.c_str();

            /* don't use ba2str to avoid -lbluetooth */
            for (int i = 5; i >= 0; i--, src_addr += 3)
                out.b[i] = strtol(src_addr, NULL, 16);
        }

        void BluetoothSocket::connectRfcomm(std::string address, uint8_t channel) {
            struct sockaddr_rc addr;
            memset(&addr, 0, sizeof(addr));
            addr.rc_family = AF_BLUETOOTH;
            addr.rc_channel = channel;
            convertAddress(address, addr.rc_bdaddr);

            socket = ::socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

            connectWriteNotifier->setEnabled(true);
            readNotifier->setEnabled(true);

            int result = ::connect(socket, (sockaddr *)&addr, sizeof(addr));

            if (result >= 0 || (result == -1 && errno == EINPROGRESS)) {
                setSocketState(ConnectingState);
            } else {
                AW_LOG(error) << "Could not open socket " << qt_error_string(errno).toStdString();
                setSocketError(UnknownSocketError);
            }
        }

        void BluetoothSocket::connectSCO(std::string address) {
            struct sockaddr_sco addr;
            memset(&addr, 0, sizeof(addr));
            addr.sco_family = AF_BLUETOOTH;
            convertAddress(address, addr.sco_bdaddr);

            socket = ::socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);

            connectWriteNotifier->setEnabled(true);
            readNotifier->setEnabled(true);

            int result = ::connect(socket, (sockaddr *)&addr, sizeof(addr));

            if (result >= 0 || (result == -1 && errno == EINPROGRESS)) {
                setSocketState(ConnectingState);
            } else {
                AW_LOG(error) << "Could not open socket " << qt_error_string(errno).toStdString();
                setSocketError(UnknownSocketError);
            }
        }
    }
}

