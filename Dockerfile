FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN sed -i 's|http://archive.ubuntu.com/ubuntu/|http://archive.ubuntu.com/ubuntu/|g' /etc/apt/sources.list && \
    echo 'Acquire::Retries "5";' > /etc/apt/apt.conf.d/80-retries

RUN apt-get update && \
    apt-get install -y --fix-missing --no-install-recommends \
    ca-certificates \
    build-essential \
    libgrpc++-dev \
    protobuf-compiler-grpc \
    libprotobuf-dev \
    libsfml-dev \
    libopenmpi-dev && \
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
    -lgrpc++ -lprotobuf -fopenmp \
    -o fractal_server

EXPOSE 50051

CMD ["./fractal_server"]