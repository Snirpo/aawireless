//
// Created by chiel on 13-01-20.
//

#include "HFPProxyProfile.h"
#include <QDBusObjectPath>

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
    Profile::newConnection(device, fd, properties, request); //TODO: implement
}

void HFPProxyProfile::requestDisconnection(BluezQt::DevicePtr device, const BluezQt::Request<> &request) {
    Profile::requestDisconnection(device, request); //TODO: implement
}

void HFPProxyProfile::release() {
    Profile::release(); //TODO: implement
}
