//
// Created by chiel on 08-01-20.
//

#include <aawireless/log/Log.h>
#include "WifiHotspot.h"
#include "boost/process/async_system.hpp"

namespace aawireless {
    namespace wifi {
        WifiHotspot::WifiHotspot(boost::asio::io_context &ioService,
                                 aawireless::configuration::Configuration &configuration,
                                 std::string password) :
                ioService(ioService),
                configuration(configuration),
                password(password){
        }

        void WifiHotspot::start() {
            AW_LOG(info) << "Starting hotspot";
            boost::process::async_system(ioService, [](boost::system::error_code error, int status){
                if (error) {
                    AW_LOG(error) << "Could not create Wifi hotspot, please restart " << std::to_string(status);
                }
            }, configuration.wifiHotspotScript, "-n", configuration.wifiDevice, configuration.wifiSSID, password);
        }

    }
}
