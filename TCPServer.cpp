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
