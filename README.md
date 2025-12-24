# Julia Sets

## Prerequisite
- **VS Code C/C++ Extension:**
---

## Environment Setup

### 1. Install MSYS2

MSYS2 provides a Unix-like environment and a package manager (`pacman`), which we'll utilize to make our lives easier when installing the compiler and libraries.

- **Download:** Go to the official [MSYS2 website](https://www.msys2.org).  
- **Install:** Run the downloaded installer (`.exe`).
  - Keep the default installation path (usually `C:\msys64`).
  - Avoid paths with spaces or special characters.

#### Initial Update

After installation, an MSYS2 terminal might open. If not, open **"MSYS2 UCRT64"** from the Start menu, do not use any other MSYS2 version for now.

Run:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc
```
### 2. Install SFML
Following the same steps as above, install SFML using `pacman`:

```bash
pacman -S mingw-w64-ucrt-x86_64-sfml
```

### 3. Configure Environemnt Variables
To ensure that your system can locate the SFML DLLs at runtime, you need to add the SFML `bin` directory to your system's `PATH` environment variable. You can easily locate in the `C:\msys64\ucrt64\bin` directory.

### 4. Run MPI
```bash
 mpic++ main.cpp src/*.cpp -Iheaders -lsfml-graphics -lsfml-window -lsfml-system -fopenmp -o mpi
```

```bash
mpirun -np 4 --oversubscribe ./mpi
```

### 5. Run gRPC
```bash
docker-compose up --build --scale fractal-worker=2
```

in another terminal
```bash
sudo apt update && sudo apt install -y libsfml-dev libgrpc++-dev protobuf-compiler-grpc libopenmpi-dev
```
```bash
protoc -I . --cpp_out=. --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` fractal.proto
```
```bash
mpicxx -O3 \
    src/main_client.cpp \
    src/SFMLWindowDrawer.cpp \
    src/JuliaSetCalculator.cpp \
    src/ParallelCalculator.cpp \
    src/SequentialCalculator.cpp \
    fractal.pb.cc \
    fractal.grpc.pb.cc \
    -I. -I./headers \
    -lsfml-graphics -lsfml-window -lsfml-system \
    `pkg-config --libs grpc++ protobuf` \
    -fopenmp \
    -o fractal_client
```
```bash
./fractal_client
```
---
### You're now set up to compile and run C++ projects using SFML in VS Code on Windows!

To run anything you'll need to simply hold `CTRL + SHIFT + B` and then run the .exe file that will pop up in your directory. Make sure the tasks.json is updated regularly as you add new files to the project, as it is not done automatically unless you use the C++ runner extension. However, using it could lead to forced modifications if not configured properly, which will break the build process.

