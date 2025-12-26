#include "../headers/SFMLWindowDrawer.hpp"
#include <typeinfo>
#include <iostream>
#include <sstream>
#include <omp.h>
#include <thread>
#include <chrono>
#include <iomanip>
#include <mpi.h>
#include <fstream>
#include <numeric>

int maxThreads = std::thread::hardware_concurrency();

struct FractalState
{
    double c_real;
    double c_imag;
    double view_x_min;
    double view_x_max;
    double view_y_min;
    double view_y_max;
    int max_iter;
    int poly_degrees;
    int command;
};

SFMLWindowDrawer::SFMLWindowDrawer(unsigned int width, unsigned int height, const std::string &title, std::unique_ptr<fractal::FractalService::Stub> stub)
    : stub_(std::move(stub)),
      current_c(-0.8, 0.156),
      current_poly_degree(2),
      current_max_iterations(100),
      view_x_min(-2.0), view_x_max(2.0),
      view_y_min(-2.0), view_y_max(2.0),
      c_change_step(0.001),
      drift_velocity(0.0, 0.0),
      velocity_step(0.005),
      needsRecalculation(true)
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        window.create(sf::VideoMode(width, height), title);

        if (!fractalTexture.create(width, height))
        {
            std::cerr << "Error: Could not create sf::Texture" << std::endl;
            window.close();
        }
        fractalSprite.setTexture(fractalTexture);
        // font loader
        if (!font.loadFromFile("arial.ttf"))
        {
            std::cerr << "Error: Could not load font 'arial.ttf'!" << std::endl;
            std::cerr << "Please copy 'arial.ttf' from C:\\Windows\\Fonts into your project directory." << std::endl;
            window.close();
        }
        setupUI();
    }

    fractalImage.create(width, height, sf::Color::Black);
    sequentialCalc = new SequentialCalculator();
    parallelCalc = new ParallelCalculator();
    calculator = sequentialCalc; // Start with sequential
}

SFMLWindowDrawer::~SFMLWindowDrawer()
{
    delete sequentialCalc;
    delete parallelCalc;
}

void SFMLWindowDrawer::setupUI()
{
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
    textControls.setString("CONTROLS:\n | [A]: Toggle Automation | [P]: Cycle Poly | [S]: Toggle Parallel | [M]: Toggle MPI\n[Arrows]: C-Value | [T/G]: Threads | [D]: Default Threads | [H]: Schedule | [Esc]: Exit");

    textParallel.setFont(font);
    textParallel.setCharacterSize(18);
    textParallel.setFillColor(sf::Color::White);
    textParallel.setPosition(10, 90);
};

void SFMLWindowDrawer::run()
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0)
    {
        while (window.isOpen())
        {
            processEvents();
            update();
            render();
        }
        FractalState killSignal;
        killSignal.command = 1;
        MPI_Bcast(&killSignal, sizeof(FractalState), MPI_BYTE, 0, MPI_COMM_WORLD);
    }
    else
    {
        while (true)
        {
            FractalState state;
            MPI_Bcast(&state, sizeof(FractalState), MPI_BYTE, 0, MPI_COMM_WORLD);
            if (state.command == 1)
                break;

            current_c = std::complex<double>(state.c_real, state.c_imag);
            view_x_min = state.view_x_min;
            view_x_max = state.view_x_max;
            view_y_min = state.view_y_min;
            view_y_max = state.view_y_max;
            current_max_iterations = state.max_iter;
            current_poly_degree = state.poly_degrees;

            int n_ranks;
            MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);
            parallelCalc->calculate_distributed(rank, n_ranks, fractalImage, current_c,
                                                current_max_iterations, current_poly_degree,
                                                view_x_min, view_x_max, view_y_min, view_y_max);
        }
    }
}

void SFMLWindowDrawer::processEvents()
{
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
        }

        if (event.type == sf::Event::KeyPressed)
        {
            bool c_changed = false;
            switch (event.key.code)
            {
            // Theme controls
            case sf::Keyboard::Num1:
            case sf::Keyboard::Numpad1:
                calculator->setTheme(1);
                needsRecalculation = true;
                break;
            case sf::Keyboard::Num2:
            case sf::Keyboard::Numpad2:
                calculator->setTheme(2);
                needsRecalculation = true;
                break;
            case sf::Keyboard::Num3:
            case sf::Keyboard::Numpad3:
                calculator->setTheme(3);
                needsRecalculation = true;
                break;
            case sf::Keyboard::Num4:
            case sf::Keyboard::Numpad4:
                calculator->setTheme(4);
                needsRecalculation = true;
                break;

            // Polynomial control
            case sf::Keyboard::P:
                current_poly_degree++;
                if (current_poly_degree > 4)
                    current_poly_degree = 2; // Cycle 2, 3, 4
                needsRecalculation = true;
                break;

            case sf::Keyboard::S:
                if (calculator == sequentialCalc)
                {
                    calculator = parallelCalc;
                    std::cout << "Switched to Parallel Calculator" << std::endl;
                }
                else
                {
                    calculator = sequentialCalc;
                    std::cout << "Switched to Sequential Calculator" << std::endl;
                }
                needsRecalculation = true;
                break;

            case sf::Keyboard::A:
                isAutomated = !isAutomated;
                std::cout << "[INFO] Automation: " << (isAutomated ? "Enabled" : "Disabled") << std::endl;
                break;
            case sf::Keyboard::M:
                if (currentMode != CalcMode::DISTRIBUTED_MPI)
                {
                    currentMode = CalcMode::DISTRIBUTED_MPI;
                    calculator = parallelCalc;
                    std::cout << "Switched to MPI Calculator" << std::endl;
                }
                else
                {
                    currentMode = CalcMode::SEQUENTIAL;
                    calculator = sequentialCalc;
                    std::cout << "Switched to Sequential Calculator" << std::endl;
                }
                needsRecalculation = true;
                break;

            // Parallel controls
            case sf::Keyboard::H:
                if (calculator == parallelCalc)
                {
                    std::string currentSchedule = parallelCalc->getSchedule();
                    if (currentSchedule == "static")
                    {
                        parallelCalc->setSchedule("dynamic");
                    }
                    else if (currentSchedule == "dynamic")
                    {
                        parallelCalc->setSchedule("guided");
                    }
                    else
                    {
                        parallelCalc->setSchedule("static");
                    }
                    needsRecalculation = true;
                }
                break;
            case sf::Keyboard::T: // Increase threads
                if (calculator == parallelCalc)
                {
                    int currentThreads = parallelCalc->getNumThreads();
                    if (currentThreads < maxThreads)
                    {
                        parallelCalc->setNumThreads((currentThreads + 1) % maxThreads);
                        needsRecalculation = true;
                    }
                }
                break;
            case sf::Keyboard::G: // Decrease threads
                if (calculator == parallelCalc)
                {
                    int currentThreads = parallelCalc->getNumThreads();
                    if (currentThreads > 0)
                    { // Allow setting to 0 (default)
                        parallelCalc->setNumThreads((currentThreads - 1) % maxThreads);
                        needsRecalculation = true;
                    }
                }
                break;
            case sf::Keyboard::D: // Set threads to default
                if (calculator == parallelCalc)
                {
                    parallelCalc->setNumThreads(0);
                    needsRecalculation = true;
                }
                break;
            // C-velocity controls
            case sf::Keyboard::Up:
                drift_velocity.imag(drift_velocity.imag() + velocity_step);
                break;
            case sf::Keyboard::Down:
                drift_velocity.imag(drift_velocity.imag() - velocity_step);
                break;
            case sf::Keyboard::Left:
                drift_velocity.real(drift_velocity.real() - velocity_step);
                break;
            case sf::Keyboard::Right:
                drift_velocity.real(drift_velocity.real() + velocity_step);
                break;
            case sf::Keyboard::Space:
                drift_velocity = std::complex<double>(0, 0);
                needsRecalculation = true;
                break;

            case sf::Keyboard::Escape:
                window.close();
                break;
            default:
                break;
            }
            if (c_changed)
                needsRecalculation = true;
        }
    }
}

void SFMLWindowDrawer::update()
{
    static auto last_auto_update = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_auto_update).count() > 50) 
    {
        double step_real = 0.0015; 
        double step_imag = 0.0008;

        current_c = std::complex<double>(
            current_c.real() + step_real, 
            current_c.imag() + step_imag
        );

        if (std::abs(current_c.real()) > 2.0) step_real *= -1;
        if (std::abs(current_c.imag()) > 2.0) step_imag *= -1;

        needsRecalculation = true;
        last_auto_update = now;
    }

    if (needsRecalculation)
    {
        static sf::Clock renderThrottle;
        if (renderThrottle.getElapsedTime().asMilliseconds() > 33) { // ~30 FPS cap
            recalculateFractal();
            renderThrottle.restart();
            needsRecalculation = false;
        }
    }

    updateUI();
}

void SFMLWindowDrawer::updateUI()
{
    // This uses string streams to build the text
    std::stringstream ss_params;
    ss_params << "C = " << current_c.real() << (current_c.imag() >= 0 ? " + " : " - ")
              << std::abs(current_c.imag()) << "i  |  Poly: " << current_poly_degree
              << " | Vel: (" << drift_velocity.real() << ", " << drift_velocity.imag() << ")";
    textParams.setString(ss_params.str());

    std::stringstream ss_theme;
    ss_theme << "Theme: " << calculator->getTheme();
    textTheme.setString(ss_theme.str());

    // Update title to show current mode
    std::string mode = "Sequential";
    if (currentMode == CalcMode::PARALLEL_OMP)
    {
        mode = "Parallel (OpenMP)";
    }
    else if (currentMode == CalcMode::DISTRIBUTED_MPI)
    {
        mode = "Distributed (MPI)";
    }
    std::stringstream ss_title;
    ss_title << "Julia Set Explorer (" << mode << ")";
    textTitle.setString(ss_title.str());

    // Update parallel info text
    if (currentMode != CalcMode::SEQUENTIAL)
    {
        std::stringstream ss_parallel;
        if (currentMode == CalcMode::DISTRIBUTED_MPI)
        {
            int n_ranks;
            MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);
            ss_parallel << "MPI Ranks: " << n_ranks << " | ";
        }
        int numThreads = parallelCalc->getNumThreads();

        ss_parallel << "OMP Threads: " << (numThreads > 0 ? std::to_string(numThreads) : "Default")
                    << " of " << maxThreads << " (Max)";
        ss_parallel << " | Schedule: ";
        ss_parallel << parallelCalc->getSchedule();
        textParallel.setString(ss_parallel.str());
    }
    else
    {
        textParallel.setString(""); // Hide when in sequential mode
    }
}

void SFMLWindowDrawer::render()
{
    window.clear();
    window.draw(fractalSprite); // Draw the fractal first

    // --- Draw all the UI text on top ---
    window.draw(textTitle);
    window.draw(textParams);
    window.draw(textTheme);
    window.draw(textParallel);
    window.draw(textControls);

    window.display();
}

void SFMLWindowDrawer::recalculateFractal()
{
    static std::ofstream metrics_file("data/metrics_log.csv");
    static auto session_start = std::chrono::steady_clock::now();

    static bool header_written = false;
    if (!header_written) {
        metrics_file << "Timestamp(s),Latency(ms),Success\n";
        header_written = true;
    }

    auto start = std::chrono::steady_clock::now();

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << "[" << std::put_time(std::localtime(&now), "%H:%M:%S") << "] Sending request to replicas..." << std::endl;

    fractal::JuliaRequest request;
    request.set_c_real(current_c.real());
    request.set_c_imag(current_c.imag());
    request.set_width(window.getSize().x);
    request.set_height(window.getSize().y);
    request.set_max_iterations(current_max_iterations);
    request.set_poly_degree(current_poly_degree);
    request.set_x_min(view_x_min);
    request.set_x_max(view_x_max);
    request.set_y_min(view_y_min);
    request.set_y_max(view_y_max);

    fractal::JuliaResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub_->CalculateJulia(&context, request, &response);
    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double, std::milli> latency = end - start;
    double timestamp = std::chrono::duration<double>(end - session_start).count();
    bool is_success = status.ok();

    if (metrics_file.is_open()) {
        metrics_file << timestamp << "," 
                     << latency.count() << "," 
                     << (is_success ? 1 : 0) << "\n";
        metrics_file.flush();
    }

    if (status.ok())
    {
        const std::string &pixelData = response.rgba_data();
        fractalImage.create(request.width(), request.height(),
                            reinterpret_cast<const sf::Uint8 *>(pixelData.data()));
        fractalTexture.update(fractalImage);
        std::chrono::duration<double, std::milli> latency = end - start;
        std::cout << "[SUCCESS] Received frame. Latency: " << latency.count() << " ms" << std::endl;
    }
    else
    {
        std::cerr << "[ERROR] gRPC failed: " << status.error_message() << std::endl;
    }
}
