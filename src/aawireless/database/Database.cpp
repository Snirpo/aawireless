//
// Created by chiel on 29-12-19.
//

#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "Database.h"

namespace aawireless {
    namespace database {
        Database::Database(const std::string &file) : file(file) {
            load();
        }

        void Database::load() {
            boost::property_tree::ptree iniConfig;
            // TODO: create directories + file if not exist
            if (boost::filesystem::exists(file)) {
                boost::property_tree::ini_parser::read_ini(file, iniConfig);
            }
            lastBluetoothDevice = iniConfig.get<std::string>("Bluetooth.LastDevice", std::string());
        }

        void Database::save() {
            boost::property_tree::ptree iniConfig;
            if (boost::filesystem::exists(file)) {
                boost::property_tree::ini_parser::read_ini(file, iniConfig);
            }
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
