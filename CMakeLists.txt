cmake_minimum_required(VERSION 3.7)
project(TCPDup)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCE_FILES main.cpp TCPClient.cpp TCPClient.h TCPServerSession.cpp TCPServerSession.h TCPServer.cpp TCPServer.cpp TCPServer.h)

FIND_PACKAGE( Boost 1.60 REQUIRED COMPONENTS program_options system thread regex)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

add_executable(TCPDup ${SOURCE_FILES})

TARGET_LINK_LIBRARIES( TCPDup LINK_PUBLIC ${Boost_LIBRARIES} )