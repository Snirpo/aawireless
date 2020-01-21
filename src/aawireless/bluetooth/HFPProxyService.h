//
// Created by chiel on 15-01-20.
//

#ifndef AAWIRELESS_HFPPROXYSERVICE_H
#define AAWIRELESS_HFPPROXYSERVICE_H


#include <BluezQt/BluezQt/Manager>
#include <boost/shared_ptr.hpp>
#include "HFPProxyProfile.h"
#include <QLocalSocket>

namespace aawireless {
    namespace bluetooth {
        class HFPProxyService : public QObject {
        Q_OBJECT

        public:
            HFPProxyService(std::shared_ptr<BluezQt::Manager> btManager);

            void start();

            void stop();

            void connectToDevice(QString address);

            void onProfileReady(BluezQt::PendingCall* call);

        private:
            std::shared_ptr<BluezQt::Manager> btManager;
            std::shared_ptr<HFPProxyProfile> hfpProxyProfile;

            void newRfcommSocket(QSharedPointer<QLocalSocket> socket);
        };
    }
}


#endif //AAWIRELESS_HFPPROXYSERVICE_H
