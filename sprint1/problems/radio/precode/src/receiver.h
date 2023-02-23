#pragma once

#include "audio.h"

#include <boost/asio.hpp>
#include <array>
#include <iostream>
#include <string>
#include <string_view>
#include <chrono>

namespace net = boost::asio;
using net::ip::udp;
using namespace std::literals;

constexpr size_t MAX_BUFFER_SIZE_RECEIVE = 65000;

class Receiver
{
public:
    Receiver(std::string&& address, int port)
        : address_(std::move(address))
        , port_(port)
        , player_(ma_format::ma_format_u8, 1) {}

    void OnReceive()
    {
        try
        {
            udp::socket socket(io_context_, udp::endpoint(net::ip::make_address(address_), port_));
            std::cout << "Server is listenning..." << std::endl;
            
            for (;;)
            {
                std::array<char, MAX_BUFFER_SIZE_RECEIVE> recv_buf;
                udp::endpoint remote_endpoint;
                auto size = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);
                if (size != 0)
                {
                    std::cout << "Received " << size << " bytes." << std::endl;
                    player_.PlayBuffer(recv_buf.data(), size, 1.5s);
                }
            }
        }
        catch(const std::exception& e)
        {
            std::cout << e.what() << '\n';
        }
        
    }
private:
    std::string address_;
    int port_;
    boost::asio::io_context io_context_;
    Player player_;

};