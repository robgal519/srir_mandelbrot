#include "Application.h"

int main(){
    const char *params[] = {"MandelbrotClient", "/tmp/.req", "/tmp/.resp"};
    try {
        Application application(params[0], params[1], params[2]);
        application.start();
    }
    catch(const std::exception& exception){
        std::cerr << "Exception thrown! " << exception.what() << std::endl;
        return -1;
    }
}

