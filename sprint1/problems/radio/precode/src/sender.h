#pragma once

#include "audio.h"
#include <boost/asio.hpp>
#include <iostream>

namespace net = boost::asio;
using net::ip::udp;
using namespace std::literals;

constexpr size_t MAX_BUFFER_SIZE_SEND = 65000;

class Sender
{
public:
    Sender(std::string&& address, int port)
        : address_(std::move(address))
        , port_(port)
        , recorder_(ma_format::ma_format_u8, 1) {}
    
    void RecordAndSendDatagram()
    {
        udp::socket socket(io_context_, udp::v4());
        boost::system::error_code ec;
        auto endpoint = udp::endpoint(net::ip::make_address(address_, ec), port_);
        std::size_t sended = 0;
        std::string str;
        
        try
        {
            for (;;)
            {
                std::cout << "Press Enter to record message or type some text to stop sending... ";
                
                std::getline(std::cin, str);
                if (str.size() > 1) break;
                
                auto rec_result = recorder_.Record(MAX_BUFFER_SIZE_SEND, 1.5s);
                std::cout << "Recording done" << std::endl;
                
                auto bufSize = rec_result.frames * recorder_.GetFrameSize();

                sended = socket.send_to(net::buffer(rec_result.data, bufSize), endpoint);
                std::cout << "Sended: " << sended  << " bytes." << std::endl;
            }
        }
        catch(const std::exception& e)
        {
            std::cout << e.what() << '\n';
            std::cout << "Sended: " << sended << " bytes." << std::endl;
        }   
    }

private:
    std::string address_;
    int port_;
    boost::asio::io_context io_context_;
    Recorder recorder_;    
};