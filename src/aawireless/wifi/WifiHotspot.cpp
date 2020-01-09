//
// Created by chiel on 08-01-20.
//

#include <aawireless/log/Log.h>
#include "WifiHotspot.h"
#include "boost/process/async_system.hpp"
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <QUuid>

using namespace NetworkManager;

namespace aawireless {
    namespace wifi {
        WifiHotspot::WifiHotspot(boost::asio::io_context &ioService,
                                 aawireless::configuration::Configuration &configuration,
                                 std::string password) :
                ioService(ioService),
                configuration(configuration),
                password(password) {

        }

        void WifiHotspot::start() {
            AW_LOG(info) << "Starting hotspot";

            auto settings = std::make_unique<ConnectionSettings>(ConnectionSettings::Wireless);

            WirelessDevice::Ptr wifiDevice;
            auto deviceName = QString::fromStdString(configuration.wifiDevice);
            Device::List deviceList = NetworkManager::networkInterfaces();

            if (!configuration.wifiDevice.empty()) {
                for (auto dev : deviceList) {
                    if (dev->type() == Device::Wifi && dev->interfaceName() == deviceName) {
                        wifiDevice = qobject_cast<WirelessDevice *>(dev);
                        break;
                    }
                }
                if (wifiDevice == nullptr) {
                    AW_LOG(error) << "Wireless device " << configuration.wifiDevice << " not found!";
                    return;
                }
            } else {
                AW_LOG(info) << "Wireless device not defined in configuration, getting first wireless device";
                for (auto dev : deviceList) {
                    if (dev->type() == Device::Wifi) {
                        wifiDevice = qobject_cast<WirelessDevice *>(dev);
                        break;
                    }
                }
            }

            if (!wifiDevice) {
                AW_LOG(error) << "No Wifi device found";
                return;
            }

            auto ssid = QString::fromStdString(configuration.wifiSSID);
            // Now we will prepare our new connection, we have to specify ID and create new UUID
            settings->setId(ssid);
            settings->setUuid(QUuid::createUuid().toString().mid(1, QUuid::createUuid().toString().length() - 2));
            settings->setAutoconnect(false);

            // For wireless setting we have to specify SSID
            auto wirelessSetting = settings->setting(Setting::Wireless).dynamicCast<WirelessSetting>();
            wirelessSetting->setSsid(ssid.toUtf8());
            wirelessSetting->setMode(WirelessSetting::NetworkMode::Ap);

            auto ipv4Setting = settings->setting(Setting::Ipv4).dynamicCast<Ipv4Setting>();
            ipv4Setting->setMethod(NetworkManager::Ipv4Setting::Shared);
            ipv4Setting->setInitialized(true);

            // Optional password setting. Can be skipped if you do not need encryption.
            auto wifiSecurity = settings->setting(Setting::WirelessSecurity).dynamicCast<WirelessSecuritySetting>();
            wifiSecurity->setKeyMgmt(WirelessSecuritySetting::WpaPsk);
            wifiSecurity->setPsk(QString::fromStdString(password));
            wifiSecurity->setInitialized(true);

            wirelessSetting->setSecurity("802-11-wireless-security");
            wirelessSetting->setInitialized(true);

            // We try to add and activate our new wireless connection
            auto reply = NetworkManager::addAndActivateConnection(settings->toMap(), wifiDevice->uni(), QString());
            reply.waitForFinished();
            if (reply.isValid()) {
                AW_LOG(info) << "Created wifi hotspot";
            } else {
                AW_LOG(error) << "Could not create Wifi hotspot " + reply.error().message().toStdString();
            }
        }

    }
}
