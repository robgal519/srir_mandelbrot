/*
 * Created on Wed Apr 17 2019 by Robert Gałat
 *
 * Copyright (c) 2019 AGH FiIS
 */
#include <chrono>
#include <iostream>
#include <thread>
#include <cmath>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <mpi.h>

#include "colors.hpp"



int procid, numprocs;

inline double rSq(double Re, double Im) {
    return Re * Re + Im * Im;
}

unsigned int inMandelbrot(double x, double y, int max_iterations) {
    int iteration = 0;
    double Re = 0;
    double Im = 0;
    while (rSq(Re, Im) <= 4 && iteration < max_iterations) {
        double Re_temp = Re * Re - Im * Im + x;
        Im = 2 * Re * Im + y;
        Re = Re_temp;
        iteration++;
    }
    return iteration;
}

struct Request {
    bool connectionOk;
    int windowWidth;
    int windowHeight;
    double leftTopX;
    double leftTopY;
    double rightBottomX;
    double rightBottomY;
};

int main(int argc, char *argv[]) {
    int err = MPI_Init(&argc, &argv);

    err = MPI_Comm_rank(MPI_COMM_WORLD, &procid);
    err = MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    std::string req = "/tmp/.req";
    std::string resp = "/tmp/.resp";
    int requestPipe;
    int responsePipe;
    std::chrono::time_point<std::chrono::steady_clock> begin_time;

//////////////////////////////////
if(procid == 0){
    mkfifo(req.c_str(), 0777);
    mkfifo(resp.c_str(), 0777);

    requestPipe = open(req.c_str(), O_RDONLY);
    if(requestPipe == -1){
        throw -1;
    }
    responsePipe = open(resp.c_str(), O_WRONLY);
    if(responsePipe == -1) {
        throw -1;
    }

}
bool condition = true;
while(condition) {
    Request request;
    int status;
    if (procid == 0) {
        std::cout << "Server: // Request  // Reading request.." << std::endl;
        status = read(requestPipe, &request, sizeof(Request));
        begin_time = std::chrono::steady_clock::now();
        if(status == -1){
            std::cout << "Server: // Request  // Reading request failed. Errno: " << errno << std::endl;
            throw -1;
        }
        std::cout << "Server: // Request  // Request read successfully. Diagnostics data:" << std::endl;
        std::cout << "Server: // Request  // connectionOk: " << (request.connectionOk ? "yes" : "no") << std::endl;
        std::cout << "Server: // Request  // windowHeight: " << request.windowHeight << std::endl;
        std::cout << "Server: // Request  // windowWidth:  " << request.windowWidth << std::endl;
        std::cout << "Server: // Request  // leftTopX:     " << request.leftTopX << std::endl;
        std::cout << "Server: // Request  // leftTopY:     " << request.leftTopY << std::endl;
        std::cout << "Server: // Request  // rightBottomX: " << request.rightBottomX << std::endl;
        std::cout << "Server: // Request  // rightBottomY: " << request.rightBottomY << std::endl;
    }
    MPI_Bcast(&request, sizeof(Request),MPI_CHAR,0,MPI_COMM_WORLD);
    int width = request.windowWidth;
    int hight = request.windowHeight;
    condition = request.connectionOk;
    if(!condition)
        continue;
    std::pair<double, double> start = {request.leftTopX, request.leftTopY};
    std::pair<double, double> end = {request.rightBottomX, request.rightBottomY};
    std::vector<char> result;
    double surface = (end.first - start.first)*(start.second-end.second);

    int iterations = 300/sqrt(surface);
    if(iterations>2000)
        iterations = 2000;
    if (procid != 0) {
        for (int y = procid - 1; y < hight; y += (numprocs - 1)) {
            char line[width * 3];
            for (int x = 0; x < width; x++) {
                double x_val = (1.0 * x / width) * (end.first - start.first) + start.first;
                double y_val = start.second - (1.0 * y / hight) * (start.second - end.second );

                colors::RGB rgb = colors::RGBColor(inMandelbrot(x_val, y_val, iterations), iterations);
                line[x * 3] = rgb.R;
                line[x * 3 + 1] = rgb.G;
                line[x * 3 + 2] = rgb.B;
            }
            MPI_Send(line, width * 3, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        }
    } else {
        for (int y = 0; y < hight; y++) {
            char line[width * 3];
            MPI_Status status;
            MPI_Recv(&line, width * 3, MPI_CHAR, y % (numprocs - 1) + 1, 0, MPI_COMM_WORLD, &status);
            result.insert(std::end(result), line, line + width * 3);
        }
        std::cout << "Server: // Response // Sending response.." << std::endl;
        std::chrono::duration<double> diff = std::chrono::steady_clock::now() - begin_time;
        std::cout << "Server: // Response // Calculations took " << (diff.count()) << " seconds." << std::endl;
        status = write(responsePipe,result.data(),result.size() );
        if(status == -1){
            std::cout << "Server: // Response // Sending response failed. Errno: " << status << std::endl;
            throw -1;
        }
        std::cout << "Server: // Response // Response sent successfully. Amount of bytes sent: " << status << std::endl;
    }
}
    MPI_Finalize();
}
