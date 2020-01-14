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

    void onConnected();

    void onDisconnected();

    void onData(QByteArray data);

    void onSCOData(QByteArray data);

private:
    QSharedPointer<QLocalSocket> rfcommSocket;
    QSharedPointer<QLocalServer> scoSocketServer;
    QLocalSocket* scoSocket;

    void socketReadyRead();

    void scoReadyRead();

    void socketDisconnected();

    void scoDisconnected();

    int createSCOSocket(QString srcAddress);

    void scoNewConnection();
};


#endif //AAWIRELESS_HFPPROXYPROFILE_H
