/*
 * Copyright (C) 2017  Maximilian Falkenstein <mfalkenstein@sos.ethz.ch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

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
