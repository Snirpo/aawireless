cmake_minimum_required(VERSION 3.15)
project(aawireless CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(base_directory ${CMAKE_CURRENT_SOURCE_DIR})
set(sources_directory ${base_directory}/src)
set(include_directory ${base_directory}/include)
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

find_package(Boost REQUIRED COMPONENTS system log OPTIONAL_COMPONENTS unit_test_framework)
find_package(libusb-1.0 REQUIRED)
find_package(Qt5 COMPONENTS Bluetooth Network)
find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
        ${Qt5Bluetooth_INCLUDE_DIRS}
        ${Qt5Network_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${LIBUSB_1_INCLUDE_DIRS}
        ${PROTOBUF_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
        ${include_directory})

#link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

file(GLOB_RECURSE proto_files ${proto_directory}/*.proto)
protobuf_generate_cpp(proto_sources proto_headers ${proto_files})
file(GLOB_RECURSE source_files ${sources_directory}/*.cpp ${include_directory}/*.hpp ${proto_headers} ${proto_sources})

add_executable(aawireless ${source_files})

target_link_libraries(aawireless
        ${Boost_LIBRARIES}
        ${Qt5Bluetooth_LIBRARIES}
        ${Qt5Network_LIBRARIES}
        ${LIBUSB_1_LIBRARIES}
        ${PROTOBUF_LIBRARIES})
