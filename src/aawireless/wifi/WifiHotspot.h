//
// Created by chiel on 08-01-20.
//

#ifndef AAWIRELESS_WIFIHOTSPOT_H
#define AAWIRELESS_WIFIHOTSPOT_H

#include <string>
#include <aawireless/configuration/Configuration.h>
#include <boost/asio/io_context.hpp>

namespace aawireless {
    namespace wifi {
        class WifiHotspot {
        public:
            WifiHotspot(boost::asio::io_context &ioService,
                    aawireless::configuration::Configuration &configuration,
                    std::string password);

            void start();
        private:
            boost::asio::io_context &ioService;
            aawireless::configuration::Configuration &configuration;
            std::string password;
        };
    }
}


#endif //AAWIRELESS_WIFIHOTSPOT_H
