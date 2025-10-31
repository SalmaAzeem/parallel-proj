#include "../headers/SFMLWindowDrawer.hpp"
#include <typeinfo>
#include <iostream>
#include <sstream>

SFMLWindowDrawer::SFMLWindowDrawer(unsigned int width, unsigned int height, const std::string& title)
    : window(sf::VideoMode(width, height), title),
      current_c(-0.8, 0.156),
      current_poly_degree(2),
      current_max_iterations(100),
      view_x_min(-2.0), view_x_max(2.0),
      view_y_min(-2.0), view_y_max(2.0),
      c_change_step(0.001),
      needsRecalculation(true)
{
    fractalImage.create(width, height, sf::Color::Black);
    sequentialCalc = new SequentialCalculator();
    parallelCalc = new ParallelCalculator();
    calculator = sequentialCalc; // Start with sequential
    if (!fractalTexture.create(width, height)) {
        std::cerr << "Error: Could not create sf::Texture!" << std::endl;
        window.close();
    }
    
    fractalSprite.setTexture(fractalTexture);

    // font loader
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error: Could not load font 'arial.ttf'!" <<std::endl;
        std::cerr << "Please copy 'arial.ttf' from C:\\Windows\\Fonts into your project directory." << std::endl;
        window.close();
    }
    setupUI();
}

SFMLWindowDrawer::~SFMLWindowDrawer() {
    delete sequentialCalc;
    delete parallelCalc;
}

void SFMLWindowDrawer::setupUI() {
    // Set up all the text objects
    textTitle.setFont(font);
    textTitle.setCharacterSize(24);
    textTitle.setFillColor(sf::Color::White);
    textTitle.setPosition(10, 10);

    textParams.setFont(font);
    textParams.setCharacterSize(18);
    textParams.setFillColor(sf::Color::White);
    textParams.setPosition(10, 40);

    textTheme.setFont(font);
    textTheme.setCharacterSize(18);
    textTheme.setFillColor(sf::Color::White);
    textTheme.setPosition(10, 65);

    textControls.setFont(font);
    textControls.setCharacterSize(14);
    textControls.setFillColor(sf::Color(180, 180, 180));
    unsigned int windowHeight = window.getSize().y;
    textControls.setPosition(10, windowHeight - 70);
    textControls.setString("CONTROLS:\n[1-4]: Theme | [P]: Cycle Poly | [S]: Toggle Parallel\n[Arrows]: Change C-Value | [Esc]: Exit");
};

void SFMLWindowDrawer::run() {
    while (window.isOpen()) {
        processEvents();
        update();
        render();
    }
}

void SFMLWindowDrawer::processEvents() {
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }

        if (event.type == sf::Event::KeyPressed) {
            bool c_changed = false;
            switch (event.key.code) {
                // Theme controls
                case sf::Keyboard::Num1:    case sf::Keyboard::Numpad1: calculator->setTheme(1); needsRecalculation = true; break;
                case sf::Keyboard::Num2:    case sf::Keyboard::Numpad2: calculator->setTheme(2); needsRecalculation = true; break;
                case sf::Keyboard::Num3:    case sf::Keyboard::Numpad3: calculator->setTheme(3); needsRecalculation = true; break;
                case sf::Keyboard::Num4:    case sf::Keyboard::Numpad4: calculator->setTheme(4); needsRecalculation = true; break;
                
                // Polynomial control
                case sf::Keyboard::P:
                    current_poly_degree++;
                    if (current_poly_degree > 4) current_poly_degree = 2; // Cycle 2, 3, 4
                    needsRecalculation = true;
                    break;

                case sf::Keyboard::S:
                    if (calculator == sequentialCalc) {
                        calculator = parallelCalc;
                        std::cout << "Switched to Parallel Calculator" << std::endl;
                    } else {
                        calculator = sequentialCalc;
                        std::cout << "Switched to Sequential Calculator" << std::endl;
                    }
                    needsRecalculation = true;
                    break;

                // C-value controls
                case sf::Keyboard::Up:    current_c.imag(current_c.imag() + c_change_step); c_changed = true; break;
                case sf::Keyboard::Down:  current_c.imag(current_c.imag() - c_change_step); c_changed = true; break;
                case sf::Keyboard::Left:  current_c.real(current_c.real() - c_change_step); c_changed = true; break;
                case sf::Keyboard::Right: current_c.real(current_c.real() + c_change_step); c_changed = true; break;


                
                case sf::Keyboard::Escape: window.close(); break;
                default: break;
            }
            if (c_changed) needsRecalculation = true;
        }
    }
}

void SFMLWindowDrawer::update() {
    // We only recalculate if a setting changed
    if (needsRecalculation) {
        recalculateFractal();
        needsRecalculation = false;
    }
    
    // We update the UI text *every* frame
    updateUI();
}

void SFMLWindowDrawer::updateUI() {
    // This uses string streams to build the text
    std::stringstream ss_params;
    ss_params << "C = " << current_c.real() << (current_c.imag() >= 0 ? " + " : " - ")
              << std::abs(current_c.imag()) << "i  |  Poly: " << current_poly_degree;
    textParams.setString(ss_params.str());

    std::stringstream ss_theme;
    ss_theme << "Theme: " << calculator->getTheme();
    textTheme.setString(ss_theme.str());

    // Update title to show current mode
    std::string mode = "Sequential";
    if (typeid(*calculator) == typeid(ParallelCalculator)) {
        mode = "Parallel";
    }
    std::stringstream ss_title;
    ss_title << "Julia Set Explorer (" << mode << ")";
    textTitle.setString(ss_title.str());
}

void SFMLWindowDrawer::render() {
    window.clear();
    window.draw(fractalSprite); // Draw the fractal first

    // --- Draw all the UI text on top ---
    window.draw(textTitle);
    window.draw(textParams);
    window.draw(textTheme);
    window.draw(textControls);

    window.display();
}

void SFMLWindowDrawer::recalculateFractal() {
    std::cout << "Recalculating fractal..." << std::endl;
    calculator->calculate_polynomial(
        fractalImage, 
        current_c, 
        current_max_iterations, 
        current_poly_degree, 
        view_x_min, view_x_max, 
        view_y_min, view_y_max
    );
    fractalTexture.update(fractalImage);
    std::cout << "Calculation complete!" << std::endl;
}