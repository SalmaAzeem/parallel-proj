#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "fractal.grpc.pb.h"
#include "headers/ParallelCalculator.hpp"
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include<thread>
#include<unistd.h>
#include <chrono>

using fractal::JuliaRequest;
using fractal::JuliaResponse;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

std::unique_ptr<grpc::Server> g_server;

class FractalServiceImpl final : public fractal::FractalService::Service
{
    ParallelCalculator calculator;
    std::string server_id_;

    public:
        FractalServiceImpl(const std::string& server_id) : server_id_(server_id) {}
    bool timeout_state = false;

    Status CalculateJulia(ServerContext *context, const JuliaRequest *request, JuliaResponse *response) override
    {
        auto md = context->client_metadata();
        auto it_shutdown = md.find("x-shutdown-worker");
        auto it = md.find("x-simulate-unavailability");
        if (it != md.end() && timeout_state)
        {
            std::cerr << "[SERVER] Simulating UNAVAILABLE" << std::endl;
            timeout_state = false;
            return Status(grpc::StatusCode::UNAVAILABLE, "simulated-unavailable");
        }
        timeout_state = true;
        sf::Image image;
        image.create(request->width(), request->height());

        double calc_time_sec = calculator.calculate_polynomial(
            image,
            std::complex<double>(request->c_real(), request->c_imag()),
            request->max_iterations(),
            request->poly_degree(),
            request->x_min(), request->x_max(),
            request->y_min(), request->y_max());

        const sf::Uint8 *pixelPtr = image.getPixelsPtr();
        size_t totalBytes = request->width() * request->height() * 4;
        response->set_rgba_data(pixelPtr, totalBytes);
        response->set_calculation_time_ms(calc_time_sec * 1000.0);
        response->set_server_id(server_id_);

        return Status::OK;
    }

       Status Shutdown(ServerContext *,
                    const fractal::ShutdownRequest *,
                    fractal::ShutdownResponse *response) override
    {
        response->set_message("Server shutting down");

        std::thread([]
                    {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            g_server->Shutdown(); })
            .detach();

        return Status::OK;
    }
};

int main()
{
    std::cout<<"delaying .....\n";
    sleep(5);
    std::cout<<"server starting\n";
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    std::string server_address("0.0.0.0:50051");
    
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    std::string server_id = std::string(hostname) + ":50051";
    
    FractalServiceImpl service(server_id);
    ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    g_server = builder.BuildAndStart();
    std::cout << "Fractal Server running on " << server_address << " (id: " << server_id << ")" << std::endl;

    g_server->Wait(); 
    std::cout << "Server stopped" << std::endl;
    return 0;
}
