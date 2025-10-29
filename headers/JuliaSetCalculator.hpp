#ifndef JULIASETCALCULATOR_HPP
#define JULIASETCALCULATOR_HPP

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Color.hpp>
#include <complex>

class JuliaSetCalculator {
public:
    JuliaSetCalculator(int theme = 1) : Theme(theme) {};
    void calculate_polynomial(sf::Image& image, const std::complex<double>& c_constant, 
        int max_iterations, int poly_degree,
        double view_x_min, double view_x_max, double view_y_min, double view_y_max);
    void setTheme(int theme);
    int getTheme() const { return Theme; }
private:
    int Theme; 
    double map(double value, double in_min, double in_max, double out_min, double out_max);
    sf::Color PixelArtist(int n, int max_iterations);
};

#endif