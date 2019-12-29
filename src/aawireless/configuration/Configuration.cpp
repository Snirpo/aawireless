//
// Created by chiel on 29-12-19.
//

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "Configuration.h"

namespace aawireless {
    namespace configuration {
        Configuration::Configuration(std::string &file) {
            boost::property_tree::ptree iniConfig;
            boost::property_tree::ini_parser::read_ini(file, iniConfig);

            ipAddress = iniConfig.get<std::string>("WIFI.IpAddress", "127.0.0.1");
        }
    }
}
