//
// Created by maxf on 18.10.17.
//

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <deque>
#include <iostream>
#include "TCPServerSession.h"
#include "TCPClient.h"

using boost::asio::ip::tcp;

TCPServerSession::TCPServerSession(boost::asio::io_service& io_service,
                     std::list<std::shared_ptr<TCPClient>>& clients)
            : my_socket(io_service), clients(clients) {
}

boost::asio::ip::tcp::socket& TCPServerSession::socket() {
    return my_socket;
}

void TCPServerSession::start() {
    boost::asio::async_read_until(my_socket,
                                  buffer,
                                  '\n',
                                  boost::bind(&TCPServerSession::do_read, shared_from_this(),
                                              boost::asio::placeholders::error));
}

void TCPServerSession::do_read(const boost::system::error_code &error) {
    if (!error) {
        std::istream is(&buffer);
        std::string line;
        std::getline(is, line);
        line += "\n";
	for (const auto client : clients) {
            client->sendMessage(line);
        }
        boost::asio::async_read_until(my_socket,
                                      buffer,
                                      '\n',
                                      boost::bind(&TCPServerSession::do_read, shared_from_this(),
                                                  boost::asio::placeholders::error));
    }
    else {
        std::cerr << "Error during read from client, closing connection!" << std::endl;
        my_socket.close();
    }
}
