//
// Created by chiel on 29-12-19.
//

#ifndef AAWIRELESS_DATABASE_H
#define AAWIRELESS_DATABASE_H

namespace aawireless {
    namespace database {
        class Database {
        public:
            Database(const std::string &file);

            void setLastBluetoothDevice(std::string address);
            std::string getLastBluetoothDevice();

            void load();
            void save();

        private:
            const std::string file;

            std::string lastBluetoothDevice;
        };
    }
}


#endif //AAWIRELESS_DATABASE_H
