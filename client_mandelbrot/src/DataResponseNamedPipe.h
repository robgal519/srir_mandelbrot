#pragma once

#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <fcntl.h>
#include "Exception.h"
#include "Pixel.h"

class DataResponseNamedPipe {
public:
    virtual ~DataResponseNamedPipe() {
        std::cout << "Response: Closing named pipe " << path << std::endl;
        close(fileDescriptor);
        std::cout << "Response: Closed named pipe " << path << std::endl;
        unlink(path.c_str());
        std::cout << "Response: Removing FIFO inode " << path << std::endl;
    }

    explicit DataResponseNamedPipe(const std::string &path): path(path){
        mkfifo(path.c_str(), 0777); // Ignore failure
        std::cout << "Response: Opening named pipe " << path << " , waiting for writer.." << std::endl;
        fileDescriptor = open(path.c_str(), O_RDONLY);
        if (fileDescriptor == -1) {
            throw CannotOpenNamedPipeException(path);
        }
        std::cout << "Response: Opened named pipe " << path << " !" << std::endl;
    }

    void readResponse(std::vector<Pixel> &image, int width, int height) {
        if (image.size() != width * height) {
            image.resize(width * height);
        }

        int bytesToBeRead = image.size() * sizeof(Pixel);
        int bytesReadInIteration = 0;
        for (int bytesRead = 0; bytesRead < bytesToBeRead; bytesRead += bytesReadInIteration) {
            bytesReadInIteration = read(fileDescriptor, (uint8_t *)(&*image.begin()) + bytesRead, bytesToBeRead);
            if (bytesReadInIteration == -1) {
                throw CannotReadFromNamedPipeException(path);
            }
        }
    }

    const std::string path;
    int fileDescriptor;
};
