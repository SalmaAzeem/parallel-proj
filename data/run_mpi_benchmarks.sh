#!/bin/bash

cd "$(dirname "$0")"

echo "Compiling MPI tests..."

mpic++ mpi_tests_fixed.cpp \
    ../src/ParallelCalculator.cpp \
    ../src/SequentialCalculator.cpp \
    ../src/JuliaSetCalculator.cpp \
    -I../headers \
    -fopenmp \
    -lsfml-graphics -lsfml-window -lsfml-system \
    -o mpi_tests_fixed

mpic++ mpi_tests_scaling.cpp \
    ../src/ParallelCalculator.cpp \
    ../src/SequentialCalculator.cpp \
    ../src/JuliaSetCalculator.cpp \
    -I../headers \
    -fopenmp \
    -lsfml-graphics -lsfml-window -lsfml-system \
    -o mpi_tests_scaling

echo "Running MPI benchmarks..."

for np in 1 2 3 4 5 6; do
    echo "Running mpi_tests_fixed with $np processes..."
    mpirun -np $np ./mpi_tests_fixed

    echo "Running mpi_tests_scaling with $np processes..."
    mpirun -np $np ./mpi_tests_scaling

    echo "---"
done
