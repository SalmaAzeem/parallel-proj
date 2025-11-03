#include <iostream>
#include <fstream>
#include "../headers/SequentialCalculator.hpp"
#include "../headers/ParallelCalculator.hpp"

int main() {
  std::ofstream file("results.csv");
  file << "ImageSize,Schedule,Threads,Sequential,Parallel,Speedup,Efficiency\n";

  SequentialCalculator sequentialCalc;
  ParallelCalculator parallelCalc;

  int sizes[] = {256, 512, 1024, 2048, 4096};
  const std::complex<double>& c = {-0.8, 0.156};
  const int max_iterations = 100;
  const int poly_degree = 2;
  const double x_min = -2.0, y_min = -2.0, x_max = 2.0, y_max = 2.0;

  for (int size: sizes) {
    sf::Image img;
    img.create(size, size);

    double sequential_elapsed_time = sequentialCalc.calculate_polynomial(img, c, max_iterations, poly_degree, x_min, x_max, y_min, y_max);

    std::string schedules[] = {"static", "dynamic", "guided"};
    int threads[] = {1, 2, 4, 8, 16};

    for (const auto& sch: schedules) {
      parallelCalc.setSchedule(sch);
      for (int t : threads) {
        parallelCalc.setNumThreads(t);
        double parallel_elapsed_time = parallelCalc.calculate_polynomial(img, c, max_iterations, poly_degree, x_min, x_max, y_min, y_max);
        double speedup = sequential_elapsed_time / parallel_elapsed_time;
        double effeciency = (speedup / t) * 100;
        file << size << "," << sch << "," << t << "," << sequential_elapsed_time << "," << parallel_elapsed_time << "," << speedup << "," << effeciency << "\n";
      }
    }
  }

  

  file.close();
}