#ifdef WIN32
#include <sdkddkver.h>
#endif

#include "seabattle.h"

#include <atomic>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <string_view>

namespace net = boost::asio;
using net::ip::tcp;
using namespace std::literals;

void PrintFieldPair(const SeabattleField& left, const SeabattleField& right) {
    auto left_pad = "  "s;
    auto delimeter = "    "s;
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
    for (size_t i = 0; i < SeabattleField::field_size; ++i) {
        std::cout << left_pad;
        left.PrintLine(std::cout, i);
        std::cout << delimeter;
        right.PrintLine(std::cout, i);
        std::cout << std::endl;
    }
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
}

template <size_t sz>
static std::optional<std::string> ReadExact(tcp::socket& socket) {
    boost::array<char, sz> buf;
    boost::system::error_code ec;

    net::read(socket, net::buffer(buf), net::transfer_exactly(sz), ec);

    if (ec) {
        return std::nullopt;
    }

    return {{buf.data(), sz}};
}

static bool WriteExact(tcp::socket& socket, std::string_view data) {
    boost::system::error_code ec;

    net::write(socket, net::buffer(data), net::transfer_exactly(data.size()), ec);

    return !ec;
}

class SeabattleAgent {
public:
    SeabattleAgent(const SeabattleField& field)
        : my_field_(field) {
    }

    void StartGame(tcp::socket& socket, bool my_initiative) {
        while (true) {
            if (my_initiative) {
                MyTurn(socket);
                if (!IsGameEnded()) 
                    OtherTurn(socket);
                else
                    break; 
            } else {
                OtherTurn(socket);
                if (!IsGameEnded()) 
                    MyTurn(socket);
                else
                    break;
            }
        }
    }

private:
    static std::optional<std::pair<int, int>> ParseMove(const std::string_view& sv) {
        if (sv.size() != 2) return std::nullopt;

        int p1 = sv[0] - 'A', p2 = sv[1] - '1';

        if (p1 < 0 || p1 > 8) return std::nullopt;
        if (p2 < 0 || p2 > 8) return std::nullopt;

        return {{p1, p2}};
    }

    static std::string MoveToString(std::pair<int, int> move) {
        char buff[] = {static_cast<char>(static_cast<char>(static_cast<char>(move.first) + 'A')), static_cast<char>(static_cast<char>(static_cast<char>(move.second) + '1'))};
        return {buff, 2};
    }

    void PrintFields() const {
        PrintFieldPair(my_field_, other_field_);
    }

    bool IsGameEnded() const {
        return my_field_.IsLoser() || other_field_.IsLoser();
    }

    void MyTurn(tcp::socket& socket) {
        PrintFields();

        std::cout << "Your turn: ";
        std::string turn;
        std::cin >> turn;
        auto myMove = ParseMove(turn);
        while (!myMove) {
            std::cout << "Wrong format! Pass 1 digit and 1 number! Try again: ";
            turn.clear();
            std::cin >> turn;
            myMove = ParseMove(turn);
        }
        
        if (!WriteExact(socket, turn)) {
            std::cout << "Couldn't write data to socket!" << std::endl;
            return;
        } else {
            std::cout << "Data was successfully written to socket!" << std::endl;
        }

        std::optional<std::string> result;
        while (!result.has_value()) {
            result = ReadExact<1>(socket);
        }

        if (IsMyShotSuccessfull(std::move(result), std::move(myMove))) {
            if (other_field_.IsLoser()) {
                std::cout << "Congratulation! You won the seabattle!" << std::endl;
            } else {
                std::cout << "Congratulation, continue!" << std::endl;
                MyTurn(socket);
            }
        }

    }

    void OtherTurn(tcp::socket& socket) {
        PrintFields();

        std::cout << "Waiting for turn... " << std::endl;
        
        std::optional<std::string> otherShot;
        while (!otherShot.has_value()) {
            otherShot = ReadExact<2>(socket);
        }
        std::cout << "Shot to " << otherShot.value() << std::endl;

        auto otherMove = ParseMove(otherShot.value());
        SeabattleField::ShotResult otherShotResult;
        bool isOtherSuccess = IsOtherShotSuccessfull(std::move(otherMove), otherShotResult);
        
        std::string strToSend;
        strToSend.push_back(static_cast<char>(otherShotResult));
        WriteExact(socket, strToSend);

        if (isOtherSuccess) {
            if (my_field_.IsLoser()) {
                std::cout << "You've lost the seabattle..." << std::endl;
            } else {
                OtherTurn(socket);
            }
        }
    }

private:
    bool IsMyShotSuccessfull(std::optional<std::string>&& result, std::optional<std::pair<int, int>>&& move) {
        bool ret = false;

        if (result) {
            char shotRes = result.value()[0];
            
            switch (shotRes) {
                case static_cast<char>(SeabattleField::ShotResult::HIT): {
                    other_field_.MarkHit(move.value().second, move.value().first);
                    ret = true;
                    std::cout << "You Hit (:" << std::endl;
                    break;
                }
                
                case static_cast<char>(SeabattleField::ShotResult::KILL): {
                    other_field_.MarkKill(move.value().second, move.value().first);
                    ret = true;
                    std::cout << "You Killed! (:" << std::endl;
                    break;
                }

                case static_cast<char>(SeabattleField::ShotResult::MISS): {
                    other_field_.MarkMiss(move.value().second, move.value().first);
                    std::cout << "You Missed :(" << std::endl;
                    break;
                }

                default: break;
            }
        }

        return ret;
    }

    bool IsOtherShotSuccessfull(std::optional<std::pair<int, int>>&& otherMove, SeabattleField::ShotResult& otherShotResult) {
        bool ret = false;

        auto shotRes = my_field_.Shoot(otherMove.value().second, otherMove.value().first);

        switch (shotRes) {
            case SeabattleField::ShotResult::HIT: {
                ret = true;
                std::cout << "Other Hit :(" << std::endl;
                break;
            }
            
            case SeabattleField::ShotResult::KILL: {
                ret = true;
                std::cout << "Other Killed :(" << std::endl;
                break;
            }

            case SeabattleField::ShotResult::MISS: {
                ret = false;
                std::cout << "Other Missed (:" << std::endl;
                break;
            }

            default: break;
        }

        otherShotResult = shotRes;
        return ret;
    }

    SeabattleField my_field_;
    SeabattleField other_field_;
};

void StartServer(const SeabattleField& field, unsigned short port) {
    SeabattleAgent agent(field);

    net::io_context io_context;

    tcp::acceptor acceptor(io_context, tcp::endpoint(net::ip::make_address("127.0.0.1"), port));
    std::cout << "Waiting for connection..."sv << std::endl;

    boost::system::error_code ec;
    tcp::socket socket{io_context};
    acceptor.accept(socket, ec);

    if (ec) {
        std::cout << "Can't accept connection"sv << std::endl;
        return;
    } else {
        std::cout << "Connection with client is established, start the game..." << std::endl;
    }

    agent.StartGame(socket, false);
};

void StartClient(const SeabattleField& field, const std::string& ip_str, unsigned short port) {
    SeabattleAgent agent(field);

    boost::system::error_code ec;
    auto endpoint = tcp::endpoint(net::ip::make_address(ip_str, ec), port);

    if (ec) {
        std::cout << "Wrong IP format"sv << std::endl;
        return;
    }

    net::io_context io_context;
    tcp::socket socket{io_context};
    socket.connect(endpoint, ec);

    if (ec) {
        std::cout << "Can't connect to server"sv << std::endl;
        return;
    } else {
        std::cout << "Connection with server is established." << std::endl;
    }

    agent.StartGame(socket, true);
};

int main(int argc, const char** argv) {
    if (argc != 3 && argc != 4) {
        std::cout << "Usage: program <seed> [<ip>] <port>" << std::endl;
        return 1;
    }

    std::mt19937 engine(std::stoi(argv[1]));
    SeabattleField fieldL = SeabattleField::GetRandomField(engine);

    if (argc == 3) {
        StartServer(fieldL, std::stoi(argv[2]));
    } else if (argc == 4) {
        StartClient(fieldL, argv[2], std::stoi(argv[3]));
    }
}
