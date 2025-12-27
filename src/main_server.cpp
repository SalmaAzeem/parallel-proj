#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "fractal.grpc.pb.h"
#include "headers/ParallelCalculator.hpp"
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include<thread>
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

        calculator.calculate_polynomial(
            image,
            std::complex<double>(request->c_real(), request->c_imag()),
            request->max_iterations(),
            request->poly_degree(),
            request->x_min(), request->x_max(),
            request->y_min(), request->y_max());

        const sf::Uint8 *pixelPtr = image.getPixelsPtr();
        size_t totalBytes = request->width() * request->height() * 4;
        response->set_rgba_data(pixelPtr, totalBytes);

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
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    std::string server_address("0.0.0.0:50051");
    FractalServiceImpl service;
    ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    // std::unique_ptr<Server> server(builder.BuildAndStart());
    // std::cout << "Fractal Server (Replica) running on " << server_address << std::endl;
    // server->Wait();

    g_server = builder.BuildAndStart();
    std::cout << "Fractal Server running on " << server_address << std::endl;

    g_server->Wait(); 
    std::cout << "Server stopped" << std::endl;
    return 0;
}
