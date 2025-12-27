FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update || (sleep 5 && apt-get update) && \
    apt-get install -y --fix-missing --no-install-recommends \
    build-essential \
    libgrpc++-dev \
    protobuf-compiler-grpc \
    libprotobuf-dev \
    libsfml-dev \
    libopenmpi-dev \
    iproute2\
    ca-certificates && \
    rm -rf /var/lib/apt/lists/*


COPY . /app
WORKDIR /app

RUN protoc -I . --cpp_out=. --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` fractal.proto

RUN mpicxx -O3 \
    src/main_server.cpp \
    src/JuliaSetCalculator.cpp \
    src/ParallelCalculator.cpp \
    fractal.pb.cc \
    fractal.grpc.pb.cc \
    -I. -I./headers \
    -lsfml-graphics -lsfml-window -lsfml-system \
    -lgrpc++ -lgrpc++_reflection -lprotobuf -lpthread -fopenmp \
    -o fractal_server


EXPOSE 50051

CMD ["./fractal_server"]
