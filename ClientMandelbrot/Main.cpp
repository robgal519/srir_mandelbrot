//
// Created by glaeqen on 07/04/19.
//


#include "Application.h"

int main(){
    int argc = 3;
    const char *argv[] = {"MandelbrotClient", "/tmp/.req", "/tmp/.resp"};
    if(argc != 3) {
        std::cout << "Pass 2 args";
        return 1;
    }
    try {
        Application application(argv[0], argv[1], argv[2]);
        application.start();
    }
    catch(const std::exception& exception){
        std::cerr << "Exception thrown! " << exception.what() << std::endl;
        return -1;
    }
}

