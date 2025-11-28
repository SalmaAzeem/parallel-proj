#ifndef WINDOWDRAWER_HPP
#define WINDOWDRAWER_HPP

#include <SFML/Graphics.hpp>
#include "JuliaSetCalculator.hpp"
#include "SequentialCalculator.hpp"
#include "ParallelCalculator.hpp"
#include <string>
#include <complex>

enum class CalcMode {
    SEQUENTIAL,
    PARALLEL_OMP,
    DISTRIBUTED_MPI
};

class SFMLWindowDrawer {
public:
    SFMLWindowDrawer(unsigned int width, unsigned int height, const std::string& title);
    ~SFMLWindowDrawer();
    void run();

private:
    void processEvents();
    void update();
    void render();

    void recalculateFractal();
    
    void setupUI();
    void updateUI();
    
    // SFML objects
    sf::RenderWindow window;
    sf::Event event;
    sf::Image fractalImage;
    sf::Texture fractalTexture;
    sf::Sprite fractalSprite;

    SequentialCalculator* sequentialCalc;
    ParallelCalculator* parallelCalc;
    JuliaSetCalculator* calculator;

    CalcMode currentMode;

    // Fractal parameters
    std::complex<double> current_c;
    int current_poly_degree;
    int current_max_iterations;
    double view_x_min, view_x_max, view_y_min, view_y_max;
    double c_change_step;

    // State management
    bool needsRecalculation;

    sf::Font font;
    sf::Text textTitle;
    sf::Text textParams;
    sf::Text textTheme;
    sf::Text textParallel;
    sf::Text textControls;
};

#endif