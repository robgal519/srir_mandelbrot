//
// Created by glaeqen on 06/04/19.
//

#ifndef MANDELBROTCLIENT_EXCEPTION_H
#define MANDELBROTCLIENT_EXCEPTION_H

#include <exception>
#include <string>

class Exception : public std::exception {
public:
    explicit Exception(const std::string &message);
    const char *what() const noexcept override;

private:
    const std::string message;
};

struct CannotOpenNamedPipeException : Exception {
    explicit CannotOpenNamedPipeException(const std::string &message) : Exception(message) {}
};
struct CannotWriteToNamedPipeException : Exception {
    explicit CannotWriteToNamedPipeException(const std::string &message) : Exception(message) {}
};
struct CannotReadFromNamedPipeException : Exception {
    explicit CannotReadFromNamedPipeException(const std::string &message) : Exception(message) {}
};


#endif //MANDELBROTCLIENT_EXCEPTION_H
