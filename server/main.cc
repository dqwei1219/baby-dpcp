#include <iostream>
#include <csignal>
#include <memory>
#include <spdlog/spdlog.h>
#include "DatabaseServer.h"

std::unique_ptr<DatabaseServer> server_ptr;

void signalHandler(int signum) {
    spdlog::info("Interrupt signal ({}) received. Shutting down server...", signum);
    if (server_ptr) server_ptr->stop();
    exit(signum);
}

int main() {
    spdlog::set_level(spdlog::level::info);

    const std::string auth_token = "your_secret_token";
    const int port = 8080;

    server_ptr = std::make_unique<DatabaseServer>(auth_token);

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    spdlog::info("Server started on http://localhost:{}", port);
    std::cout << "Press Ctrl+C to shutdown" << std::endl;

    server_ptr->start(port);

    return 0;
}
