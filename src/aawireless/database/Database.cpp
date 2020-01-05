//
// Created by chiel on 29-12-19.
//

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "Database.h"

namespace aawireless {
    namespace database {
        Database::Database(const std::string &file) : file(file) {
            load();
        }

        void Database::load() {
            boost::property_tree::ptree iniConfig;
            try {
                boost::property_tree::ini_parser::read_ini(file, iniConfig);
            } catch (...) {}
            lastBluetoothDevice = iniConfig.get<std::string>("Bluetooth.LastDevice", std::string());
        }

        void Database::save() {
            boost::property_tree::ptree iniConfig;
            boost::property_tree::ini_parser::read_ini(file, iniConfig);
            iniConfig.put("Bluetooth.LastDevice", lastBluetoothDevice);
            boost::property_tree::ini_parser::write_ini(file, iniConfig);
        }

        void Database::setLastBluetoothDevice(std::string address) {
            lastBluetoothDevice = address;
        }

        std::string Database::getLastBluetoothDevice() {
            return lastBluetoothDevice;
        }
    }
}
