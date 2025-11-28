#include "../headers/ParallelCalculator.hpp"
#include <cmath>
#include <stdlib.h>
#include <omp.h>
#include <mpi.h>
#include <vector>
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


void ParallelCalculator::apply_blur(std::vector<sf::Uint8>& buffer, int width, int start_row, int end_row) {
    std::vector<sf::Uint8> read_buffer = buffer;

    for (int y = start_row; y < end_row; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int center_idx = (y * width + x) * 4;

            int r = 0, g = 0, b = 0;
            int count = 0;

            int offsets[] = {0, -width, width, -1, 1};

            for (int k = 0; k < 5; ++k) {
                int neighbor_idx = center_idx + (offsets[k] * 4);
                r += read_buffer[neighbor_idx];
                g += read_buffer[neighbor_idx + 1];
                b += read_buffer[neighbor_idx + 2];
                count++;
            }

            buffer[center_idx]     = r / count;
            buffer[center_idx + 1] = g / count;
            buffer[center_idx + 2] = b / count;
        }
    }
}

void ParallelCalculator::calculate_distributed(int rank, int n_ranks, sf::Image& image,
    const std::complex<double>& c_constant,
    int max_iterations, int poly_degree,
    double view_x_min, double view_x_max, double view_y_min, double view_y_max) {

    unsigned int width = (rank == 0) ? image.getSize().x : 0;
    unsigned int height = (rank == 0) ? image.getSize().y : 0;
    MPI_Bcast(&width, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    int rows_per_rank = height / n_ranks;
    int remainder = height % n_ranks;
    int my_rows = rows_per_rank + (rank < remainder ? 1 : 0);
    int my_start_y = rank * rows_per_rank + (rank < remainder ? rank : remainder);
    int my_end_y = my_start_y + my_rows;

    std::vector<sf::Uint8> local_buffer((my_rows + 2) * width * 4);
    int pixel_offset = width * 4;

    for (unsigned int py = my_start_y; py < my_end_y; ++py) {
        for (unsigned int px = 0; px < width; ++px) {
            double x0 = map(px, 0, width, view_x_min, view_x_max);
            double y0 = map(py, 0, height, view_y_min, view_y_max);
            std::complex<double> z(x0, y0);
            int iteration = 0;
            while (iteration < max_iterations) {
                std::complex<double> z_next;
                if (poly_degree == 2) z_next = z * z + c_constant;
                else z_next = std::pow(z, poly_degree) + c_constant;
               
                if (std::norm(z_next) > 4.0) break;
                z = z_next;
                iteration++;
            }
            sf::Color c = PixelArtist(iteration, max_iterations);
            local_buffer[pixel_offset++] = c.r;
            local_buffer[pixel_offset++] = c.g;
            local_buffer[pixel_offset++] = c.b;
            local_buffer[pixel_offset++] = c.a;
        }
    }
   
    MPI_Request requests[4] = {MPI_REQUEST_NULL, MPI_REQUEST_NULL, MPI_REQUEST_NULL, MPI_REQUEST_NULL};
    int req_count = 0;

    sf::Uint8* my_top_row    = &local_buffer[width * 4];
    sf::Uint8* my_bottom_row = &local_buffer[my_rows * width * 4];
    sf::Uint8* top_halo      = &local_buffer[0];
    sf::Uint8* bottom_halo   = &local_buffer[(my_rows + 1) * width * 4];

    if (rank > 0) {
        MPI_Irecv(top_halo, width * 4, MPI_UNSIGNED_CHAR, rank - 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
        MPI_Isend(my_top_row, width * 4, MPI_UNSIGNED_CHAR, rank - 1, 1, MPI_COMM_WORLD, &requests[req_count++]);
    }

    if (rank < n_ranks - 1) {
        MPI_Irecv(bottom_halo, width * 4, MPI_UNSIGNED_CHAR, rank + 1, 1, MPI_COMM_WORLD, &requests[req_count++]);
        MPI_Isend(my_bottom_row, width * 4, MPI_UNSIGNED_CHAR, rank + 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
    }

    if (my_rows > 2) {
        apply_blur(local_buffer, width, 2, my_rows);
    }

    MPI_Waitall(req_count, requests, MPI_STATUSES_IGNORE);

    apply_blur(local_buffer, width, 1, 2);
    apply_blur(local_buffer, width, my_rows, my_rows + 1);

    sf::Uint8* send_ptr = &local_buffer[width * 4];
    int send_count = my_rows * width * 4;

    if (rank == 0) {
        std::vector<int> recv_counts(n_ranks);
        std::vector<int> displs(n_ranks);
        int current_disp = 0;
        for (int i = 0; i < n_ranks; ++i) {
            int r_rows = (height / n_ranks) + (i < (height % n_ranks) ? 1 : 0);
            recv_counts[i] = r_rows * width * 4;
            displs[i] = current_disp;
            current_disp += recv_counts[i];
        }
        std::vector<sf::Uint8> final_pixels(width * height * 4);
       
        MPI_Gatherv(send_ptr, send_count, MPI_UNSIGNED_CHAR,
                    final_pixels.data(), recv_counts.data(), displs.data(), MPI_UNSIGNED_CHAR,
                    0, MPI_COMM_WORLD);

        image.create(width, height, final_pixels.data());
    } else {
        MPI_Gatherv(send_ptr, send_count, MPI_UNSIGNED_CHAR,
                    NULL, NULL, NULL, MPI_UNSIGNED_CHAR,
                    0, MPI_COMM_WORLD);
    }
}