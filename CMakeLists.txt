cmake_minimum_required(VERSION 3.7)
include(GNUInstallDirs)
project(TCPDup)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCE_FILES main.cpp TCPClient.cpp TCPClient.h TCPServerSession.cpp TCPServerSession.h TCPServer.cpp TCPServer.cpp TCPServer.h)

FIND_PACKAGE( Boost 1.60 REQUIRED COMPONENTS program_options system thread regex)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

add_executable(tcpdup ${SOURCE_FILES})
install (TARGETS tcpdup RUNTIME DESTINATION bin)
TARGET_LINK_LIBRARIES(tcpdup LINK_PUBLIC ${Boost_LIBRARIES} )

# systemd unit file
configure_file (tcpdup.service ${CMAKE_CURRENT_BINARY_DIR}/tcpdup.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tcpdup.service DESTINATION /lib/systemd/system)
install(FILES tcpdup DESTINATION /etc/default)
