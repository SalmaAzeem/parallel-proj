#include "headers/SFMLWindowDrawer.hpp"
#include <iostream>
#include <mpi.h>

int main(int argc, char **argv)
{

    MPI_Init(&argc, &argv);

    try
    {
        SFMLWindowDrawer app(1280, 720, "Julia Sets Viewer");
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
        return -1;
    }

    MPI_Finalize();
    return 0;
}