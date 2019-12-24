//
// Created by chiel on 24-12-19.
//

#ifndef AAWIRELESS_LOG_H
#define AAWIRELESS_LOG_H

#include <boost/log/trivial.hpp>

#define AW_LOG(severity) BOOST_LOG_TRIVIAL(severity) << "[AAWireless] "

#endif //AAWIRELESS_LOG_H
