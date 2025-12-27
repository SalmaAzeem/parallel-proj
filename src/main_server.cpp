#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "fractal.grpc.pb.h"
#include "headers/ParallelCalculator.hpp"

using fractal::JuliaRequest;
using fractal::JuliaResponse;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class FractalServiceImpl final : public fractal::FractalService::Service
{
    ParallelCalculator calculator;

    Status CalculateJulia(ServerContext *context, const JuliaRequest *request, JuliaResponse *response) override
    {
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
};

int main()
{
    std::cout<<"delaying .....\n";
    sleep(5);
    std::cout<<"server starting\n";
    std::string server_address("0.0.0.0:50051");
    FractalServiceImpl service;

    ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Fractal Server (Replica) running on " << server_address << std::endl;
    server->Wait();
    return 0;
}
