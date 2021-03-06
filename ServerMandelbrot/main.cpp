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

#include <upcxx/upcxx.hpp>
#include "colors.hpp"

int proc_id, num_procs;

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
    upcxx::init();

    proc_id = upcxx::rank_me();
    num_procs = upcxx::rank_n();
    std::string req = "/tmp/.req";
    std::string resp = "/tmp/.resp";
    int requestPipe;
    int responsePipe;
    std::chrono::time_point<std::chrono::steady_clock> begin_time;

    upcxx::dist_object<upcxx::global_ptr<Request>> global_request(upcxx::new_<Request>());

//////////////////////////////////
    if (proc_id == 0) {
        mkfifo(req.c_str(), 0777);
        mkfifo(resp.c_str(), 0777);

        requestPipe = open(req.c_str(), O_RDONLY);
        if (requestPipe == -1) {
            throw -1;
        }
        responsePipe = open(resp.c_str(), O_WRONLY);
        if (responsePipe == -1) {
            throw -1;
        }

    }
    bool connectionOK = true;
    while (connectionOK) {
        Request request;
        int status;
        if (proc_id == 0) {
            std::cout << "Server: // Request  // Reading request.." << std::endl;
            status = read(requestPipe, &request, sizeof(Request));
            begin_time = std::chrono::steady_clock::now();
            if (status == -1) {
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
            upcxx::rput(request, *global_request);
        }
        // wait for node 0 to receive the request
        upcxx::barrier();
        request = upcxx::rget(global_request.fetch(0).wait()).wait();


        int width = request.windowWidth;
        int height = request.windowHeight;
        connectionOK = request.connectionOk;
        if (!connectionOK)
            continue;

        // calculate number of lines to process by this thread
        uint16_t linesForThread = 0;
        if (proc_id == num_procs - 1) {
            linesForThread = height - (num_procs - 2) * (height / (num_procs - 1));
        } else {
            linesForThread = height / (num_procs - 1);
        }
        upcxx::dist_object<upcxx::global_ptr<char>> global_line(upcxx::new_array<char>(width * linesForThread * 3));

        std::pair<double, double> start = {request.leftTopX, request.leftTopY};
        std::pair<double, double> end = {request.rightBottomX, request.rightBottomY};
        std::vector<char> result;
        double surface = (end.first - start.first) * (start.second - end.second);

        int iterations = 300 / sqrt(surface);
        if (iterations > 2000)
            iterations = 2000;
        if (proc_id != 0) {
            char *block_of_lines = global_line->local();
            int y_from_picture_top = (height / (num_procs - 1)) * (proc_id - 1);
            for (int y = 0;
                 y < linesForThread; y++) {
                for (int x = 0; x < width; x++) {
                    double x_val = (1.0 * x / width) * (end.first - start.first) + start.first;
                    double y_val =
                            start.second - (1.0 * (y + y_from_picture_top) / height) * (start.second - end.second);

                    colors::RGB rgb = colors::RGBColor(inMandelbrot(x_val, y_val, iterations), iterations);
                    block_of_lines[y * width * 3 + x * 3] = rgb.R;
                    block_of_lines[y * width * 3 + x * 3 + 1] = rgb.G;
                    block_of_lines[y * width * 3 + x * 3 + 2] = rgb.B;
                }

            }
        }
        upcxx::barrier();
        if (proc_id == 0) {
            for (int proc = 1; proc < num_procs; proc++) {
                int linesForThread;
                if (proc == num_procs - 1) {
                    linesForThread = height - (num_procs - 2) * (height / (num_procs - 1));
                } else {
                    linesForThread = height / (num_procs - 1);
                }
                std::vector<char> block_of_lines(linesForThread * width * 3);
                upcxx::rget(
                        global_line.fetch(proc).wait(),
                        block_of_lines.data(),
                        linesForThread * width * 3).wait();
                result.insert(std::end(result), std::begin(block_of_lines), std::end(block_of_lines));
            }
            std::cout << "Server: // Response // Sending response.." << std::endl;
            std::chrono::duration<double> diff = std::chrono::steady_clock::now() - begin_time;
            std::cout << "Server: // Response // Calculations took " << (diff.count()) << " seconds." << std::endl;
            status = write(responsePipe, result.data(), result.size());
            if (status == -1) {
                std::cout << "Server: // Response // Sending response failed. Errno: " << status << std::endl;
                throw -1;
            }
            std::cout << "Server: // Response // Response sent successfully. Amount of bytes sent: " << status
                      << std::endl;
        }
        upcxx::barrier();
        upcxx::delete_array(*global_line);
    }
    upcxx::finalize();
}
