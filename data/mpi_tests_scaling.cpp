#include <mpi.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <complex>
#include "../headers/ParallelCalculator.hpp"
#include "../headers/SequentialCalculator.hpp"

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, n_ranks;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);

    SequentialCalculator seqCalc;
    ParallelCalculator parCalc;
    parCalc.setNumThreads(1);

    std::vector<int> sizes = {256, 512, 1024, 2048, 4096};
    const std::complex<double> c = {-0.8, 0.156};
    const int max_iter = 100;
    const int poly = 2;
    const double x_min = -2.0, x_max = 2.0, y_min = -2.0, y_max = 2.0;

    // Write header if file does not exist
    std::ifstream infile("mpi_scaledsize.csv");
    bool file_exists = infile.good();
    infile.close();
    if (rank == 0 && !file_exists) {
        std::ofstream file("mpi_scaledsize.csv", std::ios::app);
        file << "ImageSize,Schedule,Threads,Sequential,Parallel,Speedup,Efficiency\n";
        file.close();
    }
    for (int size : sizes) {
        sf::Image img;
        if (rank == 0) img.create(size, size);
        double t_seq = 0.0;
        if (rank == 0) {
            t_seq = seqCalc.calculate_polynomial(img, c, max_iter, poly, x_min, x_max, y_min, y_max);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        double start = MPI_Wtime();
        parCalc.calculate_distributed(rank, n_ranks, img, c, max_iter, poly, x_min, x_max, y_min, y_max);
        double t_par = MPI_Wtime() - start;
        if (rank == 0) {
            double speedup = t_seq / t_par;
            double eff = (speedup / n_ranks) * 100.0;
            std::ofstream file("mpi_scaledsize.csv", std::ios::app);
            file << size << ",static," << n_ranks << "," << t_seq << "," << t_par << "," << speedup << "," << eff << "\n";
            file.close();
            std::cout << "Scaling Size: " << size << " | Ranks: " << n_ranks << " | Speedup: " << speedup << "\n";
        }
    }
    MPI_Finalize();
    return 0;
}
