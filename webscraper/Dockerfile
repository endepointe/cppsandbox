FROM ubuntu:latest
RUN apt-get update && \
    apt-get install -y \
    curl \
    cmake \ 
    g++ \
    libcurl4-openssl-dev \
    zlib1g-dev

WORKDIR /app

COPY main.cpp /app/main.cpp

RUN g++ -std=c++17 main.cpp -o scraper -lcurl -lz -lpthread
