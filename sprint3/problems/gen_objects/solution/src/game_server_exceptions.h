#pragma once

#include <string>
#include <exception>

namespace server_exceptions {

// Custom exceptions

class BaseException : public std::exception {
public:
    BaseException(const std::string& message, const std::string& code)
        : message_(message)
        , code_ (code) {}
    const std::string& message() const noexcept { return message_; }
    const std::string& code() const noexcept { return code_; }

private:
    std::string message_;
    std::string code_;
};

class InvalidArgumentException : public BaseException {
public:
    InvalidArgumentException(const std::string& message = "") 
        : BaseException(message, "invalidArgument") {}
};

class InvalidNameException : public BaseException {
public:
    InvalidNameException(const std::string& message = "") 
        : BaseException(message, "invalidArgumentName") {}
};

class InvalidMapException : public BaseException {
public:
    InvalidMapException(const std::string& message = "") 
        : BaseException(message, "invalidArgumentMap") {}
};

class ParseException : public BaseException {
public:
    ParseException(const std::string& message = "") 
        : BaseException(message, "invalidArgumentParse") {}
};

class InvalidDirectionException : public BaseException {
public:
    InvalidDirectionException(const std::string& message = "") 
        : BaseException(message, "invalidArgumentDirection") {}
};

class InvalidEndpointException : public BaseException {
public:
    InvalidEndpointException(const std::string& message = "") 
        : BaseException(message, "invalidEndpoint") {}
};

class InvalidTokenException : public BaseException {
public:
    InvalidTokenException(const std::string& message = "") 
        : BaseException(message, "invalidToken") {}
};

class UnknownTokenException : public BaseException {
public:
    UnknownTokenException(const std::string& message = "") 
        : BaseException(message, "unknownToken") {}
};

}