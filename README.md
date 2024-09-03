# BitfinexTradingBot

A robust C++ trading bot for Bitfinex exchange. Features include order placement, modification, cancellation, orderbook retrieval, and position management. Built with security and performance in mind.

## Features

- Place, modify, and cancel orders on Bitfinex
- Retrieve orderbook data
- Manage trading positions
- Secure API credential handling

## Prerequisites

- C++11 or later
- CMake 3.10 or later
- libcurl
- OpenSSL
- nlohmann-json (header-only library)

## Setup

1. Clone the repository:
   ```
   git clone https://github.com/yourusername/BitfinexTradingBot.git
   cd BitfinexTradingBot
   ```

2. Download the `json.hpp` file:
   - Visit the [nlohmann/json GitHub repository](https://github.com/nlohmann/json)
   - Navigate to the `single_include/nlohmann` directory
   - Download the `json.hpp` file
   - Place the downloaded `json.hpp` file in your project's `include` directory

3. Install other dependencies:
   - On Ubuntu/Debian:
     ```
     sudo apt-get install libcurl4-openssl-dev libssl-dev
     ```
   - On macOS (using Homebrew):
     ```
     brew install curl openssl
     ```
   - On Windows, consider using [vcpkg](https://github.com/Microsoft/vcpkg) to manage dependencies:
     ```
     vcpkg install curl openssl
     ```

## Building the Project

1. Create a build directory and run CMake:
   ```
   mkdir build && cd build
   cmake ..
   ```

2. Build the project:
   ```
   make
   ```

## Configuration

1. Copy the `.env.example` file to `.env`:
   ```
   cp .env.example .env
   ```

2. Edit the `.env` file and add your Bitfinex API credentials:
   ```
   BITFINEX_API_KEY=your_api_key_here
   BITFINEX_API_SECRET=your_api_secret_here
   ```

## Usage

Run the executable generated in the build directory:

```
./BitfinexTradingBot
```