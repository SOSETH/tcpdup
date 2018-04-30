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

#ifndef TCPDUP_TCPCLIENT_H
#define TCPDUP_TCPCLIENT_H

#include <boost/asio.hpp>
#include <deque>
#include <string>

class TCPClient {
public:
    TCPClient(boost::asio::io_service& io_service, boost::asio::ip::tcp::resolver::iterator& endpoint_iterator,
              int backendNumber, unsigned int buf_size);
    void sendMessage(std::string & message);

private:
    bool connected;
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator;
    boost::asio::io_service& io_service;
    boost::asio::deadline_timer timer;
    std::unique_ptr<boost::asio::ip::tcp::socket> socket;
    std::deque<std::string> messages, offline_messages;
    int backend_number;
    unsigned int buf_size;

    void do_connect();
    void do_write();
    void reconnect_later();
};


#endif //TCPDUP_TCPCLIENT_H
