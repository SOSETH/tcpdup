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
#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include <sys/prctl.h>
#include <seccomp.h>

#include "TCPClient.h"
#include "TCPServer.h"

using boost::asio::ip::tcp;
namespace po = boost::program_options;

int main(int argc, const char* const* argv) {
    // https://stackoverflow.com/a/25969006
    boost::regex ip_regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

    // Not to be confused with option_description which is something completely different
    po::options_description desc("Options");
    desc.add_options()
            ("help", "print this message")
            ("backend", po::value<std::vector<std::string>>()->multitoken(),
             "backend to use (\"x.x.x.x:port\")")
            ("buf", po::value<unsigned int>()->default_value(8192), "Number of measurements to buffer in case of connectivity issues")
            ("port", po::value<unsigned short>()->default_value(12345), "port to bind to")
            ("bind", po::value<std::string>()->default_value("127.0.0.1"), "IP to bind to")
            ("seccomp-dbg", "Trap instead of exit on seccomp violation (debug only!)")
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
    if (!boost::regex_match(bindAddr, ip_regex)) {
        std::cerr << "Bind address has invalid format: " << bindAddr << std::endl;
        return -1;
    }

    // Init seccomp filter
    scmp_filter_ctx ctx;
    if (options.count("seccomp-dbg")) {
        ctx = seccomp_init(SCMP_ACT_TRAP); // default action: trap
    } else {
        ctx = seccomp_init(SCMP_ACT_KILL); // default action: kill
        prctl(PR_SET_NO_NEW_PRIVS, 1);     // disallow setting permissive seccomp rules afterwars
        prctl(PR_SET_DUMPABLE, 0);         // disallow changing settings via ptrace from another process
    }

    // Basics
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(munmap), 0);
    //seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mprotect), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(epoll_wait), 0);
    // boost::asio
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(poll), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(getsockopt), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(accept), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(epoll_ctl), 0);
    // ioctl is needed to enable async io on a newly established connection
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(ioctl), 1, SCMP_A1(SCMP_CMP_EQ, FIONBIO));
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(recvmsg), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(sendmsg), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(close), 0);
    // timers
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(timerfd_settime), 0);
    // reconnect
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(socket), 0); // TODO: restrict if possible
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(connect), 0); // TODO: restrict if possible

    // Init ASIO
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
    /*
     * Boost ASIO's timers will open /etc/localtime (once!). open() is very difficult to restrict using seccomp's BPF,
     * therefore we simply execute a timer before enabling the seccomp rules.
     * */
    boost::asio::deadline_timer timer(ioservice);
    timer.expires_from_now(boost::posix_time::seconds(1));
    timer.async_wait([ctx] (const boost::system::error_code&) {
        seccomp_load(ctx);
    });
    ioservice.run();
}
