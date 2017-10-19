#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include "TCPClient.h"
#include "TCPServer.h"

using boost::asio::ip::tcp;
namespace po = boost::program_options;

int main(int argc, const char* const* argv) {
    // Not to be confused with option_description which is something completely different
    po::options_description desc("Options");
    desc.add_options()
            ("help", "print this message")
            ("backend", po::value<std::vector<std::string>>()->multitoken(),
             "backend to use (\"x.x.x.x:port\")")
            ("buf", po::value<unsigned int>()->default_value(8192), "Number of measurements to buffer in case of connectivity issues")
            ("port", po::value<unsigned short>()->default_value(12345), "port to bind to")
            ("bind", po::value<std::string>()->default_value("127.0.0.1"), "IP to bind to")
            ;
    po::variables_map options;
    po::store(po::parse_command_line(argc, argv, desc), options);
    po::notify(options);

    // Check options
    if (options.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    // Check if at least one backend was specified
    if (!options.count("backend")) {
        std::cerr << "You need to specify at least one backend!" << std::endl;
        std::cout << desc << std::endl;
        return -1;
    }

    // Check if the bind address makes sense
    std::string bindAddr = options["bind"].as<std::string>();
    // https://stackoverflow.com/a/25969006
    boost::regex e("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    if (!boost::regex_match(bindAddr, e)) {
        std::cerr << "Bind address has invalid format: " << bindAddr << std::endl;
        return -1;
    }

    boost::asio::io_service ioservice;
    tcp::resolver resolver(ioservice);

    // Open TCP server socket
    tcp::endpoint tcp_endpoint{boost::asio::ip::address::from_string(options["bind"].as<std::string>()),
                                options["port"].as<unsigned short>()};

    // Open TCP client connection to each backend
    std::vector<std::string> backends = options["backend"].as<std::vector<std::string>>();
    auto clientList = std::list<std::shared_ptr<TCPClient>>();
    std::cout << "Number of backends: " << backends.size() << std::endl;

    for (int i = 0; i < backends.size(); i++) {
        std::string backend = backends[i];

        auto index = backend.find(':');
        if (index > 0 && index<backend.size()-1) {
            auto host = backend.substr(0, index);
            auto port = backend.substr(index+1);
            std::cout << "Backend " << i << ": Host: " << host << ", Port: " << port << std::endl;

            tcp::resolver::query query(tcp::v4(), host, port);
            tcp::resolver::iterator iterator = resolver.resolve(query);
            clientList.push_back(std::make_shared<TCPClient>(ioservice, iterator, i, options["buf"].as<unsigned int>()));
        } else {
            std::cout << "Backend " << i << " has invalid format (" << backend << "), ignored!" << std::endl;
        }
    }

    TCPServer server(ioservice, tcp_endpoint, clientList);
    ioservice.run();
}