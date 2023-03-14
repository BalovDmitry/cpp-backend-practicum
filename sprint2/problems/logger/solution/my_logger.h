#pragma once

#include <chrono>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <optional>
#include <mutex>
#include <thread>
#include <filesystem>
#include <iostream>

#ifdef __APPLE__ 
#define LOG_PATH "/Users/balovdmitry/Desktop/SB/cpp-backend-practicum/sprint2/problems/logger" 
#else 
#define LOG_PATH "/var/log"
#endif


using namespace std::literals;

#define LOG(...) Logger::GetInstance().Log(__VA_ARGS__)

class Logger {
    auto GetTime() const {
        if (manual_ts_) {
            return *manual_ts_;
        }

        return std::chrono::system_clock::now();
    }

    auto GetTimeStamp() const {
        const auto now = GetTime();
        const auto t_c = std::chrono::system_clock::to_time_t(now);
        return std::put_time(std::localtime(&t_c), "%F %T");
    }

    // Для имени файла возьмите дату с форматом "%Y_%m_%d"
    std::string GetFileTimeStamp() const {
        std::ostringstream buf;
        buf << GetTimeStamp();
        auto ts = buf.str();

        auto res = ts.substr(0, ts.find_first_of(" "));
        std::replace(res.begin(), res.end(), '-', '_');
        
        return res;
    }

    Logger() = default;
    Logger(const Logger&) = delete;

public:
    static Logger& GetInstance() {
        static Logger obj;
        return obj;
    }

    // Выведите в поток все аргументы.
    template<class... Ts>
    void Log(const Ts&... args) {
        std::lock_guard<std::mutex> g(m_);
        
        fileName_ = "sample_log_" + GetFileTimeStamp() + ".log";
        absPath_ = std::filesystem::weakly_canonical(std::filesystem::path(LOG_PATH) / std::filesystem::path(fileName_));

        //std::cout << "Filename: " << fileName_ << std::endl;
        //std::cout << "File path: " << absPath_ << std::endl;
        log_file_.open(absPath_, std::ios::app);
        log_file_ << GetTimeStamp() << ": ";
        ((log_file_ << std::forward<decltype(args)>(args)), ...); 
        log_file_ << '\n';
        log_file_.close();      
    }

    // Установите manual_ts_. Учтите, что эта операция может выполняться
    // параллельно с выводом в поток, вам нужно предусмотреть 
    // синхронизацию.
    void SetTimestamp(std::chrono::system_clock::time_point ts) {
        std::lock_guard<std::mutex> g(m_);
        manual_ts_ = ts;
    }

private:
    std::optional<std::chrono::system_clock::time_point> manual_ts_;
    std::string fileName_;
    std::filesystem::path absPath_;
    bool isTimeChanged_ = true;
    std::mutex m_;
    std::ofstream log_file_;
};
