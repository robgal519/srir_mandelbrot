#include "Exception.h"

const char *Exception::what() const noexcept {
    return message.c_str();
}

Exception::Exception(const std::string &message) : message(message) {};

