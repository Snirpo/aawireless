//
// Created by chiel on 13-01-20.
//

#ifndef AAWIRELESS_HFPPROXYPROFILE_H
#define AAWIRELESS_HFPPROXYPROFILE_H

#include <BluezQt/Profile>
#include <boost/asio/io_context.hpp>
#include <QSharedPointer>
#include <QtNetwork/QLocalServer>

class HFPProxyProfile : public BluezQt::Profile {
Q_OBJECT

public:
    HFPProxyProfile();

    QDBusObjectPath objectPath() const override;

    QString uuid() const override;

    void newConnection(BluezQt::DevicePtr device, const QDBusUnixFileDescriptor &fd, const QVariantMap &properties,
                       const BluezQt::Request<> &request) override;

    void requestDisconnection(BluezQt::DevicePtr device, const BluezQt::Request<> &request) override;

    void release() override;

public:
signals:
    void onNewRfcommSocket(QSharedPointer<QLocalSocket> rfcommSocket);
    void onNewSCOSocket(QLocalSocket* scoSocket);

private:
    QSharedPointer<QLocalSocket> rfcommSocket;
    QSharedPointer<QLocalServer> scoSocketServer;
    QLocalSocket* scoSocket;

    int createSCOSocket(QString srcAddress);

    void scoNewConnection();
};


#endif //AAWIRELESS_HFPPROXYPROFILE_H
