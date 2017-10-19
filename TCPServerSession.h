//
// Created by maxf on 18.10.17.
//

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
