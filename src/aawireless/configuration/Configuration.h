//
// Created by chiel on 29-12-19.
//

#ifndef AAWIRELESS_CONFIGURATION_H
#define AAWIRELESS_CONFIGURATION_H

namespace aawireless {
    namespace configuration {
        class Configuration {
        public:
            Configuration(const std::string &file);

            std::string wifiDevice;
            std::string wifiIpAddress;
            uint16_t wifiPort;
            std::string wifiBSSID;
            std::string wifiSSID;
            std::string wifiPassphrase;
            std::string wifiHotspotScript;
        };
    }
}


#endif //AAWIRELESS_CONFIGURATION_H
