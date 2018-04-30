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
#include <memory>
#include "TCPClient.h"

using boost::asio::ip::tcp;

TCPClient::TCPClient(boost::asio::io_service& io_service, tcp::resolver::iterator& endpoint_iterator,
                     int backendNumber, unsigned int buf_size):
            io_service (io_service), timer(io_service), backend_number(backendNumber), buf_size(buf_size) {
    this->endpoint_iterator = std::move(endpoint_iterator);
    do_connect();
}

void TCPClient::sendMessage(std::string & message) {
    if (!connected) {
        if (offline_messages.size() < buf_size) {
            offline_messages.push_back(message);
        }
        if (offline_messages.size() % 10 == 0) {
            std::cerr << "Backend " << backend_number << ", " << offline_messages.size()  << " of " << buf_size
                      << " (max) messages in offline buffer" << std::endl;
        }
    } else {
        io_service.post(
                [this, message]() {
                    // This is run on the io_service and therefore cannot race!
                    bool stillWriting = !messages.empty();
                    messages.push_back(message);
                    if (!stillWriting) {
                        do_write();
                    }
                });
    }
}

void TCPClient::do_connect() {
    // re-create socket for new connection
    socket = std::move(std::make_unique<tcp::socket>(io_service));

    boost::asio::async_connect(*socket, endpoint_iterator,
                               [this](boost::system::error_code ec, tcp::resolver::iterator) {
                                   if (!ec) {
                                       connected = true;
                                       std::cerr << "Backend " << backend_number << ", connected" << std::endl;
                                       io_service.post(
                                               [this]() {
                                                   // This is run on the io_service and therefore cannot race!
                                                   while (!offline_messages.empty()) {
                                                       messages.push_back(offline_messages.front());
                                                       offline_messages.pop_front();
                                                   }
                                                   if (!messages.empty()) {
                                                       do_write();
                                                   }
                                               });
                                   } else {
                                       std::cerr << "Backend " << backend_number << ", coudln't connect to backend: "
                                                 << ec.message() << std::endl;
                                       socket->close();
                                       reconnect_later();
                                   }
                               });
}

void TCPClient::do_write() {
    if (!connected)
        return;
    boost::asio::async_write(*socket,
                             boost::asio::buffer(messages.front().data(),
                                                 messages.front().length()),
                             [this](boost::system::error_code ec, std::size_t) {
                                 if (!ec && connected) {
                                     // Remove successfully sent message from queue and
                                     // recursively call this function to send the next one
                                     messages.pop_front();
                                     if (!messages.empty()) {
                                         do_write();
                                     }
                                 }
                                 else {
                                     std::cerr << "Backend " << backend_number << ", error during write: "
                                               << ec.message() << std::endl;
                                     connected = false;
                                     socket->close();
                                     reconnect_later();
                                 }
                             });
}

void TCPClient::reconnect_later() {
    if (!connected) {
        // Reconnect after 5 second
        std::cerr << "Backend " << backend_number << ", reconnecting in 5 seconds" << std::endl;
        timer.expires_from_now(boost::posix_time::seconds(5));
        timer.async_wait(std::bind(&TCPClient::do_connect, this));
    }
}
