#include "headers/SFMLWindowDrawer.hpp"
#include <grpcpp/grpcpp.h>
#include "fractal.grpc.pb.h"
#include <mpi.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    std::string server_list = "ipv4:127.0.0.1:50051,127.0.0.1:50052";

    std::string service_config =
        "{"
        "  \"methodConfig\": [{"
        "    \"name\": [{}],"
        "    \"retryPolicy\": {"
        "      \"maxAttempts\": 3,"
        "      \"initialBackoff\": \"0.1s\","
        "      \"maxBackoff\": \"1s\","
        "      \"backoffMultiplier\": 2,"
        "      \"retryableStatusCodes\": [\"UNAVAILABLE\"]"
        "    }"
        "  }]"
        "}";

    grpc::ChannelArguments args;
    args.SetLoadBalancingPolicyName("round_robin");

    args.SetServiceConfigJSON(service_config);

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
