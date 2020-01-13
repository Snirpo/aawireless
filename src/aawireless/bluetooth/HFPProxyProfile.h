//
// Created by chiel on 13-01-20.
//

#ifndef AAWIRELESS_HFPPROXYPROFILE_H
#define AAWIRELESS_HFPPROXYPROFILE_H

#include <BluezQt/Profile>

class HFPProxyProfile : public BluezQt::Profile {
public:
    HFPProxyProfile();

    QDBusObjectPath objectPath() const override;

    QString uuid() const override;

    void newConnection(BluezQt::DevicePtr device, const QDBusUnixFileDescriptor &fd, const QVariantMap &properties,
                       const BluezQt::Request<> &request) override;

    void requestDisconnection(BluezQt::DevicePtr device, const BluezQt::Request<> &request) override;

    void release() override;
};


#endif //AAWIRELESS_HFPPROXYPROFILE_H
