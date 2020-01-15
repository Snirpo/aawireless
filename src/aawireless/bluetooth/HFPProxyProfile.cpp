//
// Created by chiel on 13-01-20.
//

#include "HFPProxyProfile.h"
#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>
#include <boost/asio/local/stream_protocol.hpp>
#include <QLocalSocket>
#include <QLocalServer>
#include <aawireless/log/Log.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/sco.h>
#include <BluezQt/Device>
#include <BluezQt/Adapter>

HFPProxyProfile::HFPProxyProfile() {
    setName(QStringLiteral("HandsfreeProfile"));
    //setChannel(0);
}

QDBusObjectPath HFPProxyProfile::objectPath() const {
    return QDBusObjectPath(QStringLiteral("/HandsfreeProfile"));
}

QString HFPProxyProfile::uuid() const {
    return QStringLiteral("0000111e-0000-1000-8000-00805f9b34fb"); // HFP profile uuid
}

void HFPProxyProfile::newConnection(BluezQt::DevicePtr device, const QDBusUnixFileDescriptor &fd,
                                    const QVariantMap &properties, const BluezQt::Request<> &request) {
    AW_LOG(info) << "Creating rfcomm socket";

    if (rfcommSocket) rfcommSocket->close();
    if (scoSocketServer) scoSocketServer->close();

    rfcommSocket = createSocket(fd);
    if (!rfcommSocket->isValid()) {
        request.cancel();
        AW_LOG(error) << "HFP profile rfcomm socket invalid!";
        return;
    }

    AW_LOG(info) << "Listening for SCO connections";
    auto adapterAddress = device->adapter()->address();
    int scoFd = createSCOSocket(adapterAddress);
    scoSocketServer = QSharedPointer<QLocalServer>(new QLocalServer);
    scoSocketServer->connect(scoSocketServer.data(), &QLocalServer::newConnection, this,
                             &HFPProxyProfile::scoNewConnection);

    if (!scoSocketServer->listen(scoFd)) {
        request.cancel();
        AW_LOG(error) << "HFP profile SCO socket invalid!";
        return;
    }

    request.accept();

    emit onNewRfcommSocket(rfcommSocket);
}

void HFPProxyProfile::scoNewConnection() {
    scoSocket = scoSocketServer->nextPendingConnection();
    AW_LOG(info) << "New SCO connection";

    emit onNewSCOSocket(scoSocket);
}

void HFPProxyProfile::requestDisconnection(BluezQt::DevicePtr device, const BluezQt::Request<> &request) {
    AW_LOG(info) << "On request disconnection";
    request.accept();
}

void HFPProxyProfile::release() {
//    rfcommSocket->disconnectFromServer();
//    rfcommSocket.clear();
}

int HFPProxyProfile::createSCOSocket(QString srcAddress) {
    // TODO: move elsewhere
//    int sock = ::socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);
//    if (sock < 0) {
//        AW_LOG(error) << "Could not create SCO socket";
//        return nullptr;
//    }
//
//    const char *srcAddr = srcAddress.toLocal8Bit().data();
//    const char *dstAddr = dstAddress.toLocal8Bit().data();
//    bdaddr_t src;
//    bdaddr_t dst;
//    struct sockaddr_sco addr;
//
//    for (int i = 5; i >= 0; i--, srcAddr += 3)
//        src.b[i] = strtol(srcAddr, NULL, 16);
//    for (int i = 5; i >= 0; i--, dstAddr += 3)
//        dst.b[i] = strtol(dstAddr, NULL, 16);
//
//    socklen_t len = sizeof(addr);
//    memset(&addr, 0, len);
//    addr.sco_family = AF_BLUETOOTH;
//    bacpy(&addr.sco_bdaddr, &src);
//
//    if (::bind(sock, (struct sockaddr *) &addr, len) < 0) {
//        AW_LOG(error) << "Could not bind socket";
//        ::close(sock);
//        return nullptr;
//    }
//
//    memset(&addr, 0, len);
//    addr.sco_family = AF_BLUETOOTH;
//    bacpy(&addr.sco_bdaddr, &dst);
//
//    AW_LOG(info) << "SCO socket connect";
//    int err = ::connect(sock, (struct sockaddr *) &addr, len);
//    if (err < 0 && !(errno == EAGAIN || errno == EINPROGRESS)) {
//        AW_LOG(error) << "Could not connect SCO socket " << errno;
//        ::close(sock);
//        return nullptr;
//    }
//

    int sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET | SOCK_NONBLOCK | SOCK_CLOEXEC, BTPROTO_SCO);
    if (sock < 0) {
        AW_LOG(error) << "Could not create SCO socket";
        return -1;
    }

    AW_LOG(info) << "Creating SCO socket on " << srcAddress.toStdString();
    const char *src_addr = srcAddress.toLocal8Bit().data();
    bdaddr_t src;

    /* don't use ba2str to avoid -lbluetooth */
    for (int i = 5; i >= 0; i--, src_addr += 3)
        src.b[i] = strtol(src_addr, NULL, 16);

    /* Bind to local address */
    struct sockaddr_sco addr;
    memset(&addr, 0, sizeof(addr));
    addr.sco_family = AF_BLUETOOTH;
    bacpy(&addr.sco_bdaddr, &src);

    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        AW_LOG(error) << "Could not bind SCO socket " << errno;
        ::close(sock);
        return -1;
    }

    return sock;
}
