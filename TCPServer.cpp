//
// Created by maxf on 18.10.17.
//

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include "TCPServer.h"

using boost::asio::ip::tcp;

TCPServer::TCPServer(boost::asio::io_service& io_service, const tcp::endpoint& endpoint,
              std::list<std::shared_ptr<TCPClient>>& clients) :
            io_service(io_service), acceptor(io_service, endpoint), clients(clients) {
    start_accept();
}

void TCPServer::start_accept() {
    std::shared_ptr<TCPServerSession> new_session(new TCPServerSession(io_service, clients));
    acceptor.async_accept(new_session->socket(),
                          boost::bind(&TCPServer::handle_accept, this, new_session,
                                      boost::asio::placeholders::error));
}

void TCPServer::handle_accept(std::shared_ptr<TCPServerSession> session,
                              const boost::system::error_code &ec) {
    if (!ec) {
        session->start();
    } else {
        std::cerr << "Error while accepting connection: " << ec.message() << std::endl;
    }

    start_accept();
}