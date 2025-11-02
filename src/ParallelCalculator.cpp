#include "../headers/ParallelCalculator.hpp"
#include <cmath>
#include <stdlib.h>
#include <omp.h>
#include<iostream>

ParallelCalculator::ParallelCalculator() : JuliaSetCalculator(), scheduleType("static"), numThreads(0) {}

void ParallelCalculator::setSchedule(const std::string& schedule) {
    scheduleType = schedule;
}

std::string ParallelCalculator::getSchedule() const {
    return scheduleType;
}

void ParallelCalculator::setNumThreads(int threads) {
    numThreads = threads;
}

int ParallelCalculator::getNumThreads() const {
    return numThreads;
}

void ParallelCalculator::calculate_pixel(unsigned int px, unsigned int py, sf::Image& image, const std::complex<double>& c_constant, 
    int max_iterations, int poly_degree,
    double view_x_min, double view_x_max, double view_y_min, double view_y_max) {
    
    unsigned int width = image.getSize().x;
    unsigned int height = image.getSize().y;

    double x0 = map(px, 0, width, view_x_min, view_x_max);
    double y0 = map(py, 0, height, view_y_min, view_y_max);
    std::complex<double> z(x0, y0);

    int iteration = 0;
    while (iteration < max_iterations) {
        std::complex<double> z_next;
        switch (poly_degree) {
            case 2:  z_next = z * z + c_constant; break;
            case 3:  z_next = z * z * z + c_constant; break;
            case 4:  z_next = z * z * z * z + c_constant; break;
            default: z_next = std::pow(z, poly_degree) + c_constant; break;
        }

        if (std::norm(z_next) > 4.0) {
            break;
        }
        z = z_next;
        iteration++;
    }
    sf::Color color = PixelArtist(iteration, max_iterations);
    image.setPixel(px, py, color);
}

// this function doesnt return anything, it simply sets the pixel color based on the number of iterations
double ParallelCalculator::calculate_polynomial(sf::Image& image, const std::complex<double>& c_constant, 
    int max_iterations, int poly_degree,
    double view_x_min, double view_x_max, double view_y_min, double view_y_max) {
        // unsigned because they have more range for image dimensions since removing the sign bit allows for an extra bit of magnitude :D 

    if (numThreads > 0) {
        omp_set_num_threads(numThreads);
    } else {
        omp_set_num_threads(omp_get_max_threads());
    }

    unsigned int width = image.getSize().x;
    unsigned int height = image.getSize().y;
        // Iterate over each pixel in the image
    long double start_time = omp_get_wtime();

    if (scheduleType == "dynamic") {
        #pragma omp parallel for collapse(2) schedule(dynamic)
        for (unsigned int px = 0; px < width; ++px) {
            for (unsigned int py = 0; py < height; ++py) {
                calculate_pixel(px, py, image, c_constant, max_iterations, poly_degree, view_x_min, view_x_max, view_y_min, view_y_max);
            }
        }
    } else if (scheduleType == "guided") {
        #pragma omp parallel for collapse(2) schedule(guided)
        for (unsigned int px = 0; px < width; ++px) {
            for (unsigned int py = 0; py < height; ++py) {
                calculate_pixel(px, py, image, c_constant, max_iterations, poly_degree, view_x_min, view_x_max, view_y_min, view_y_max);
            }
        }
    } else { // Default to static
        if (scheduleType != "static") {
            std::cerr << "Warning: Unknown schedule type '" << scheduleType << "'. Defaulting to 'static'." << std::endl;
            scheduleType = "static";
        }
        #pragma omp parallel for collapse(2) schedule(static)
        for (unsigned int px = 0; px < width; ++px) {
            for (unsigned int py = 0; py < height; ++py) {
                calculate_pixel(px, py, image, c_constant, max_iterations, poly_degree, view_x_min, view_x_max, view_y_min, view_y_max);
            }
        }
    }

    long double end_time = omp_get_wtime();
    long double elapsed_time = end_time - start_time;
    std::cout<<"Calculation took "<< elapsed_time <<" seconds\n";
    return elapsed_time;
}
