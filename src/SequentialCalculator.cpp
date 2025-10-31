#include "../headers/SequentialCalculator.hpp"
#include <cmath>
#include <stdlib.h>
#include <omp.h>
#include<iostream>



// this function doesnt return anything, it simply sets the pixel color based on the number of iterations
void SequentialCalculator::calculate_polynomial(sf::Image& image, const std::complex<double>& c_constant, 
    int max_iterations, int poly_degree,
    double view_x_min, double view_x_max, double view_y_min, double view_y_max) {
        // unsigned because they have more range for image dimensions since removing the sign bit allows for an extra bit of magnitude :D 
    unsigned int width = image.getSize().x;
    unsigned int height = image.getSize().y;
        // Iterate over each pixel in the image
    long double start_time = omp_get_wtime();

    // #pragma omp parallel for collapse(2)
    for (unsigned int px = 0; px < width; ++px) {
        for (unsigned int py = 0; py < height; ++py) {
            double x0 = map(px, 0, width, view_x_min, view_x_max);
            double y0 = map(py, 0, height, view_y_min, view_y_max);
            std::complex<double> z(x0, y0);

            int iteration = 0;
            while (iteration < max_iterations) {
                std::complex<double> z_next = std::pow(z, poly_degree) + c_constant;
                if (std::norm(z_next) > 4.0) {
                    break;
                }
                z = z_next;
                iteration++;
            }
            sf::Color color = PixelArtist(iteration, max_iterations);
            image.setPixel(px, py, color);
        }
    }

    long double end_time = omp_get_wtime();
    std::cout<<"Calculation took "<< end_time-start_time<<" seconds\n";
}
