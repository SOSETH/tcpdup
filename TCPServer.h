//
// Created by maxf on 18.10.17.
//

#ifndef TCPDUP_TCPSERVER_H
#define TCPDUP_TCPSERVER_H

#include <boost/asio.hpp>
#include "TCPServerSession.h"

class TCPServer {
public:
    TCPServer(boost::asio::io_service& io_service, const boost::asio::ip::tcp::endpoint& endpoint,
              std::list<std::shared_ptr<TCPClient>>& clients);

private:
    void start_accept();
    void handle_accept(std::shared_ptr<TCPServerSession> session, const boost::system::error_code &ec);

private:
    boost::asio::io_service& io_service;
    boost::asio::ip::tcp::acceptor acceptor;
    std::list<std::shared_ptr<TCPClient>> clients;
};


#endif //TCPDUP_TCPSERVER_H
