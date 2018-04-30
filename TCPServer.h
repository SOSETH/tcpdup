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
