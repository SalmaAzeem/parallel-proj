#include "headers/SFMLWindowDrawer.hpp"
#include <grpcpp/grpcpp.h>
#include "fractal.grpc.pb.h"
#include <mpi.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    std::string server_list = "localhost:50051,localhost:50052";
    grpc::ChannelArguments args;
    args.SetLoadBalancingPolicyName("round_robin");

    auto channel = grpc::CreateCustomChannel(
        server_list,
        grpc::InsecureChannelCredentials(),
        args);

    auto stub = fractal::FractalService::NewStub(channel);

    SFMLWindowDrawer drawer(800, 600, "Julia Set Explorer (gRPC Replicas)", std::move(stub));

    drawer.run();

    MPI_Finalize();
    return 0;
}
