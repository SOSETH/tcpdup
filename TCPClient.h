//
// Created by maxf on 18.10.17.
//

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
