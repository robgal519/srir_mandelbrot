#pragma once

#include "Exception.h"
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>

struct Request {
    bool connectionOk;
    int windowWidth;
    int windowHeight;
    double leftTopX;
    double leftTopY;
    double rightBottomX;
    double rightBottomY;
};

class DataRequestNamedPipe {
public:
    virtual ~DataRequestNamedPipe() {
        std::cout << "Request: Closing named pipe " << path << std::endl;
        close(fileDescriptor);
        std::cout << "Request: Closed named pipe " << path << std::endl;
        unlink(path.c_str());
        std::cout << "Request: Removing FIFO inode " << path << std::endl;
    }

    explicit DataRequestNamedPipe(const std::string &path): path(path) {
        mkfifo(path.c_str(), 0777); // Ignore failure
        std::cout << "Request: Opening named pipe " << path << " , waiting for writer.." << std::endl;
        fileDescriptor = open(path.c_str(), O_WRONLY);
        if (fileDescriptor == -1) {
            throw CannotOpenNamedPipeException(path);
        }
        std::cout << "Request: Opened named pipe " << path << " !" << std::endl;
    }

    void sendRequest(const Request &request) {
        int status = write(fileDescriptor, &request, sizeof(Request));
        if (status == -1) {
            throw CannotWriteToNamedPipeException(path);
        }
    }

    const std::string path;
    int fileDescriptor;
};
