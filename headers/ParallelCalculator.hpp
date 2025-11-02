#ifndef PARALLELCALCULATOR_HPP
#define ParallelCALCULATOR_HPP

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Color.hpp>
#include "JuliaSetCalculator.hpp"
#include <complex>
#include <string>

class ParallelCalculator:public JuliaSetCalculator {
public:
    ParallelCalculator();
    double calculate_polynomial (sf::Image& image, const std::complex<double>& c_constant, 
        int max_iterations, int poly_degree,
        double view_x_min, double view_x_max, double view_y_min, double view_y_max);
    
    void setSchedule(const std::string& schedule);
    std::string getSchedule() const;
    void setNumThreads(int threads);
    int getNumThreads() const;
private:
    std::string scheduleType;
    int numThreads;

    void calculate_pixel(unsigned int px, unsigned int py, sf::Image& image, const std::complex<double>& c_constant, 
        int max_iterations, int poly_degree,
        double view_x_min, double view_x_max, double view_y_min, double view_y_max);
};

#endif