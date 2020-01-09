//
// Created by chiel on 29-12-19.
//

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "Configuration.h"

namespace aawireless {
    namespace configuration {
        Configuration::Configuration(const std::string &file) {
            boost::property_tree::ptree iniConfig;
            boost::property_tree::ini_parser::read_ini(file, iniConfig);

            wifiDevice = iniConfig.get<std::string>("Wifi.Device");
            wifiIpAddress = iniConfig.get<std::string>("Wifi.IpAddress");
            wifiPort = iniConfig.get<std::uint16_t>("Wifi.Port");
            wifiSSID = iniConfig.get<std::string>("Wifi.SSID");
            //wifiPassphrase = iniConfig.get<std::string>("Wifi.Passphrase"); // TODO: implement for hardcoded passphrase instead of generated
        }
    }
}
