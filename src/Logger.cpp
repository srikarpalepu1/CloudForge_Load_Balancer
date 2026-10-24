#include "Logger.h"

#include <iostream>
#include <stdexcept>

Logger::Logger(const std::string& fileName, bool verbose)
    : outputFile_(fileName), verbose_(verbose) {
    if (!outputFile_.is_open()) {
        throw std::runtime_error("Unable to open log file: " + fileName);
    }
}

void Logger::event(int cycle,
                   const std::string& message,
                   const std::string& color) {
    if (!verbose_) {
        return;
    }

    const std::string formattedMessage =
        "[Cycle " + std::to_string(cycle) + "] " + message;

    outputFile_ << formattedMessage << '\n';
    std::cout << color << formattedMessage << "\033[0m\n";
}

void Logger::summary(const std::string& message) {
    outputFile_ << message << '\n';
    std::cout << message << '\n';
}

void Logger::section(const std::string& title) {
    const std::string divider(64, '=');

    outputFile_ << '\n'
                << divider << '\n'
                << title << '\n'
                << divider << '\n';

    std::cout << "\n\033[1;36m"
              << divider << '\n'
              << title << '\n'
              << divider
              << "\033[0m\n";
}

bool Logger::isVerbose() const {
    return verbose_;
}