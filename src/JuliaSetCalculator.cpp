#include "../headers/JuliaSetCalculator.hpp"
#include <cmath>
#include <stdlib.h>

double JuliaSetCalculator::map(double value, double in_min, double in_max, double out_min, double out_max) {
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
// this function doesnt return anything, it simply sets the pixel color based on the number of iterations
void JuliaSetCalculator::calculate_polynomial(sf::Image& image, const std::complex<double>& c_constant, 
    int max_iterations, int poly_degree,
    double view_x_min, double view_x_max, double view_y_min, double view_y_max) {
        // unsigned because they have more range for image dimensions since removing the sign bit allows for an extra bit of magnitude :D 
    unsigned int width = image.getSize().x;
    unsigned int height = image.getSize().y;
        // Iterate over each pixel in the image
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
}

void JuliaSetCalculator::setTheme(int themeID) {
    if (themeID > 0 && themeID <= 4) {
        this->Theme = themeID;
    } else {
        this->Theme = 1;
    }
}

sf::Color JuliaSetCalculator::PixelArtist(int n, int max_iterations) {
    if (n == max_iterations) {
        return sf::Color::Black;
    }

    // Theme ROULETTE :D
    switch (this->Theme) {
        
        // Theme 1: RGB
        case 1: 
        {
            sf::Uint8 r = static_cast<sf::Uint8>((n * 10) % 255);
            sf::Uint8 g = static_cast<sf::Uint8>((n * 7) % 255);
            sf::Uint8 b = static_cast<sf::Uint8>((n * 4) % 255);
            return sf::Color(r, g, b);
        }

        // Theme 2: Blue-Purpleish
        case 2:
        {
            sf::Uint8 r = static_cast<sf::Uint8>(100 + (n * 5) % 155);
            sf::Uint8 g = static_cast<sf::Uint8>((n * 2) % 100);
            sf::Uint8 b = static_cast<sf::Uint8>(200 + (n * 10) % 55);
            return sf::Color(r, g, b);
        }

        // Theme 3: Orange Scheme
        case 3:
        {
            sf::Uint8 r = static_cast<sf::Uint8>(200 + (n * 5) % 55);
            sf::Uint8 g = static_cast<sf::Uint8>(100 + (n * 8) % 155);
            sf::Uint8 b = static_cast<sf::Uint8>((n * 2) % 50);
            return sf::Color(r, g, b);
        }

        // Theme 4: Grayscale
        case 4:
        {
            sf::Uint8 gray = static_cast<sf::Uint8>((n * 8) % 255);
            return sf::Color(gray, gray, gray);
        }

        default:
        {
            sf::Uint8 r = static_cast<sf::Uint8>((n * 10) % 255);
            sf::Uint8 g = static_cast<sf::Uint8>((n * 7) % 255);
            sf::Uint8 b = static_cast<sf::Uint8>((n * 4) % 255);
            return sf::Color(r, g, b);
        }
    }
}
