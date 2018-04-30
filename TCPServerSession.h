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

#ifndef TCPDUP_TCPSERVERSESSION_H
#define TCPDUP_TCPSERVERSESSION_H

#include <boost/asio.hpp>
#include <list>
#include "TCPClient.h"

class TCPServerSession : public std::enable_shared_from_this<TCPServerSession>
{
public:
    TCPServerSession(boost::asio::io_service& io_service, std::list<std::shared_ptr<TCPClient>>& clients);
    boost::asio::ip::tcp::socket& socket();
    void start();
    void do_read(const boost::system::error_code &error);

private:
    boost::asio::ip::tcp::socket my_socket;
    boost::asio::streambuf buffer;
    std::list<std::shared_ptr<TCPClient>> clients;
};

#endif //TCPDUP_TCPSERVERSESSION_H
