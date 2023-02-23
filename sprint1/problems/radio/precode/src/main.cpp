#include "audio.h"

#include "sender.h"
#include "receiver.h"

#include <iostream>

using namespace std::literals;

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Wrong number of parameters!" << std::endl;
        return 1;
    }
    
    int port = std::stoi(std::string(argv[2]));

    if (std::string(argv[1]) == "client")
    {
        Sender sender("127.0.0.1", port);
        sender.RecordAndSendDatagram();
    }
    else if (std::string(argv[1]) == "server")
    { 
        Receiver receiver("127.0.0.1", port);
        receiver.OnReceive();
    }
    else
    {
        std::cout << "Unknown mode!" << std::endl;
        return 1;
    }
    
    // Recorder recorder(ma_format_u8, 1);
    // Player player(ma_format_u8, 1);

    // while (true) {
    //     std::string str;

    //     std::cout << "Press Enter to record message..." << std::endl;
    //     std::getline(std::cin, str);

    //     auto rec_result = recorder.Record(65000, 1.5s);
    //     std::cout << "Recording done" << std::endl;
        
    //     std::cout << "Record results: " << "frames: " << rec_result.frames << std::endl;

    //     player.PlayBuffer(rec_result.data.data(), rec_result.frames, 1.5s);
    //     std::cout << "Playing done" << std::endl;
    // }

    return 0;
}
