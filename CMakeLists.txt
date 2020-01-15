cmake_minimum_required(VERSION 3.15)
project(aawireless CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(base_directory ${CMAKE_CURRENT_SOURCE_DIR})
set(sources_directory ${base_directory}/src)
set(proto_directory ${base_directory}/proto)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${base_directory}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${base_directory}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${base_directory}/bin)
set(EXECUTABLE_OUTPUT_PATH ${base_directory}/bin)

set(Boost_NO_BOOST_CMAKE 1)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

add_definitions(-DBOOST_ALL_DYN_LINK)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} -Wall -pedantic -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-g -O3")

### TODO: Should not be done this way? ###
set(AASDK_LIBRARIES /usr/local/lib/libaasdk.so)
set(AASDK_INCLUDE_DIRS /usr/local/include/aasdk)
set(AASDK_PROTO_LIBRARIES /usr/local/lib/libaasdk_proto.so)
set(AASDK_PROTO_INCLUDE_DIRS /usr/local/include/aasdk_proto)
###

find_package(Boost REQUIRED COMPONENTS system log random OPTIONAL_COMPONENTS unit_test_framework)
find_package(libusb-1.0 REQUIRED)
find_package(Qt5 COMPONENTS Bluetooth Network DBus)
find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(KF5NetworkManagerQt REQUIRED)
find_package(KF5BluezQt REQUIRED)
#find_package(libbluetooth REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
        ${Boost_INCLUDE_DIRS}
        ${LIBUSB_1_INCLUDE_DIRS}
        ${PROTOBUF_INCLUDE_DIR}
        ${AASDK_PROTO_INCLUDE_DIRS}
        ${AASDK_INCLUDE_DIRS}
        ${sources_directory})

file(GLOB_RECURSE source_files ${sources_directory}/*.h ${sources_directory}/*.cpp)

add_executable(aawireless ${proto_headers} ${proto_sources} ${source_files})

target_link_libraries(aawireless
        ${Boost_LIBRARIES}
        ${LIBUSB_1_LIBRARIES}
        ${PROTOBUF_LIBRARIES}
        Qt5::Bluetooth
        Qt5::Network
        Qt5::DBus
        KF5::NetworkManagerQt
        KF5::BluezQt
        ${AASDK_PROTO_LIBRARIES}
        ${AASDK_LIBRARIES})
