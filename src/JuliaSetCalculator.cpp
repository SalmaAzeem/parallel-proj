#include "../headers/JuliaSetCalculator.hpp"
#include <cmath>
#include <stdlib.h>
#include <omp.h>
#include<iostream>


double JuliaSetCalculator::map(double value, double in_min, double in_max, double out_min, double out_max) {
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
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
