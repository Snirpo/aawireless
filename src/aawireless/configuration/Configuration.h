//
// Created by chiel on 29-12-19.
//

#ifndef AAWIRELESS_CONFIGURATION_H
#define AAWIRELESS_CONFIGURATION_H

namespace aawireless {
    namespace configuration {
        class Configuration {
        public:
            Configuration(std::string &file);

            std::string ipAddress;
        };
    }
}


#endif //AAWIRELESS_CONFIGURATION_H
