/*
 * Created on Wed Apr 17 2019 by Robert Gałat
 *
 * Copyright (c) 2019 AGH FiIS
 */
#include <fstream>
#include <iostream>
#include <thread>
#include <cmath>
#include <vector>

#include <mpi.h>

#include "colors.hpp"


int procid, numprocs;

inline double rSq(double Re, double Im) {
    return Re * Re + Im * Im;
}

unsigned int inMandelbrot(double x, double y) {
    int iteration = 0;
    double Re = 0;
    double Im = 0;
    constexpr int max_iterations = 80;
    while (rSq(Re, Im) <= 4 && iteration < max_iterations) {
        double Re_temp = Re * Re - Im * Im + x;
        Im = 2 * Re * Im + y;
        Re = Re_temp;
        iteration++;
    }
    return iteration;
}


int main(int argc, char *argv[]) {
    int err = MPI_Init(&argc, &argv);

    err = MPI_Comm_rank(MPI_COMM_WORLD, &procid);
    err = MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

//////////////////////////////////
    int width = 1700;
    int hight = 600;
// whole picture is in rectangle between {-2,1} and {1,-1}
    std::pair<double, double> start = {-2, 1};
    std::pair<double, double> end = {1, -1};
    std::vector<char> result;
    if (procid != 0) {

        for (int y = procid-1; y < hight; y+= (numprocs-1)) {
            //std::vector<RGB> line(width);
            char line[width * 3];
            for (int x = 0; x < width; x++) {
                double x_val = (1.0 * x / width) * (end.first - start.first) + start.first;
                double y_val = (1.0 * y / hight) * (end.second - start.second) + start.second;
                colors::RGB rgb = colors::RGBColor(inMandelbrot(x_val, y_val), 80);
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
                MPI_Recv(&line, width * 3, MPI_CHAR, y%(numprocs-1) +1, 0, MPI_COMM_WORLD, &status);
                result.insert(std::end(result), line, line + width * 3);

        }
        std::ofstream plot("data.ppm");

        plot << "P6" << std::endl;
        plot << width << " " << hight << std::endl;
        plot << "255" << std::endl;

        for (const auto &a:result)
            plot << a;
        plot.close();
    }
    MPI_Finalize();
}
