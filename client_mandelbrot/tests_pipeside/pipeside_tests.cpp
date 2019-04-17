//
// Created by glaeqen on 06/04/19.
//
#include <DataRequestNamedPipe.h>


void testingFunction (){
    std::string req = "/tmp/.req";
    std::string resp = "/tmp/.resp";
    mkfifo(req.c_str(), 0777);
    mkfifo(resp.c_str(), 0777);

    std::cout << "T: Opening req R" << std::endl;
    int requestPipe = open(req.c_str(), O_RDONLY);
    if(requestPipe == -1){
        throw CannotOpenNamedPipeException("Cannot open " + req);
    }
    std::cout << "T: Opened req R" << std::endl;
    std::cout << "T: Opening resp W" << std::endl;
    int responsePipe = open(resp.c_str(), O_WRONLY);
    if(responsePipe == -1){
        throw CannotOpenNamedPipeException("Cannot open " + resp);
    }
    std::cout << "T: Opened resp W" << std::endl;
    Request request;
    int status;
    int requests = 0;
    int responses = 0;
    while(true) {
        std::cout << "T: Reading request.." << std::endl;
        status = read(requestPipe, &request, sizeof(Request));
        if(status == -1){
            throw CannotReadFromNamedPipeException("Cannot read from " + req);
        }
        requests++;
        std::cout << "T: Request " << requests << " read." << std::endl;
        std::cout << "T: Sending response.." << std::endl;
        for(int i=0; i<request.windowHeight*request.windowWidth; i++){
            status = write(responsePipe, &request.color, sizeof(request.color));
            if(status == -1){
                throw CannotWriteToNamedPipeException("Cannot write to " + resp);
            }
        }
        responses++;
        std::cout << "T: Response " << responses << " sent." << std::endl;
    }
}

int main(){
    try{
        testingFunction();
    }
    catch(const std::exception& exception){
        std::cerr << "Exception thrown! " << exception.what() << std::endl;
        return -1;
    }
}

