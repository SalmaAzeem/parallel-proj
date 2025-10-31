#ifndef PARALLELCALCULATOR_HPP
#define ParallelCALCULATOR_HPP

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Color.hpp>
#include "JuliaSetCalculator.hpp"
#include <complex>

class ParallelCalculator:public JuliaSetCalculator {
public:
    ParallelCalculator() :JuliaSetCalculator() {};
    void calculate_polynomial (sf::Image& image, const std::complex<double>& c_constant, 
        int max_iterations, int poly_degree,
        double view_x_min, double view_x_max, double view_y_min, double view_y_max);
};

#endif