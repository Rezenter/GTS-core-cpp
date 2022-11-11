cmake_minimum_required(VERSION 3.21)
project(GTS-core)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "-static")
set(BUILD_ARCH "-m64")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${BUILD_ARCH} -Wall -Wextra" )
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${BUILD_ARCH}")

include_directories(${CMAKE_SOURCE_DIR} include lib)


add_executable(GTS-core src/main.cpp version.h

        src/FastADC/CAEN743.cpp src/FastADC/FastSystem.cpp
        src/FastADC/Crate.cpp include/Stoppable.h src/FastADC/Config.cpp
        src/FastADC/Storage.cpp

        include/Server/request.hpp include/Server/header.hpp src/Server/connection.cpp
        src/Server/connection_manager.cpp src/Server/reply.cpp src/Server/request_handler.cpp src/Server/mime_types.cpp
        src/Server/request_parser.cpp src/Server/server.cpp include/FastADC/Processor.h src/FastADC/Processor.cpp)
target_link_libraries(GTS-core ${CMAKE_CURRENT_SOURCE_DIR}/lib/CAENDigitizer.dll wsock32 ws2_32 boost_system boost_thread-mt)



