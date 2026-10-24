#include "Config.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {

std::string trim(const std::string& value) {
    const auto first = std::find_if_not(
        value.begin(), value.end(),
        [](unsigned char character) {
            return std::isspace(character);
        });

    const auto last = std::find_if_not(
        value.rbegin(), value.rend(),
        [](unsigned char character) {
            return std::isspace(character);
        }).base();

    if (first >= last) {
        return "";
    }

    return std::string(first, last);
}

bool parseBoolean(const std::string& value) {
    if (value == "true" || value == "1" || value == "yes") {
        return true;
    }

    if (value == "false" || value == "0" || value == "no") {
        return false;
    }

    throw std::invalid_argument("Invalid Boolean value: " + value);
}

} // namespace

Config Config::loadFromFile(const std::string& fileName) {
    std::ifstream inputFile(fileName);

    if (!inputFile.is_open()) {
        throw std::runtime_error(
            "Unable to open configuration file: " + fileName);
    }

    Config config;
    std::string line;
    int lineNumber = 0;

    while (std::getline(inputFile, line)) {
        ++lineNumber;
        line = trim(line);

        if (line.empty() || line.front() == '#') {
            continue;
        }

        const std::size_t separator = line.find('=');

        if (separator == std::string::npos) {
            throw std::runtime_error(
                "Missing '=' on configuration line " +
                std::to_string(lineNumber));
        }

        const std::string key = trim(line.substr(0, separator));
        const std::string value = trim(line.substr(separator + 1));

        if (key == "initial_servers") {
            config.initialServers = std::stoi(value);
        } else if (key == "total_cycles") {
            config.totalCycles = std::stoi(value);
        } else if (key == "initial_queue_per_server") {
            config.initialQueuePerServer = std::stoi(value);
        } else if (key == "min_processing_time") {
            config.minProcessingTime = std::stoi(value);
        } else if (key == "max_processing_time") {
            config.maxProcessingTime = std::stoi(value);
        } else if (key == "lower_queue_per_server") {
            config.lowerQueuePerServer = std::stoi(value);
        } else if (key == "upper_queue_per_server") {
            config.upperQueuePerServer = std::stoi(value);
        } else if (key == "scale_cooldown") {
            config.scaleCooldown = std::stoi(value);
        } else if (key == "minimum_servers_per_balancer") {
            config.minimumServersPerBalancer = std::stoi(value);
        } else if (key == "new_request_probability") {
            config.newRequestProbability = std::stod(value);
        } else if (key == "blocked_traffic_rate") {
            config.blockedTrafficRate = std::stod(value);
        } else if (key == "random_seed") {
            config.randomSeed =
                static_cast<std::uint32_t>(std::stoul(value));
        } else if (key == "verbose") {
            config.verbose = parseBoolean(value);
        } else if (key == "blocked_prefix") {
            config.blockedPrefix = value;
        } else if (key == "log_file") {
            config.logFile = value;
        } else {
            throw std::runtime_error(
                "Unknown configuration key on line " +
                std::to_string(lineNumber) + ": " + key);
        }
    }

    config.validate();
    return config;
}

void Config::validate() const {
    if (initialServers < 2) {
        throw std::invalid_argument(
            "At least two initial servers are required.");
    }

    if (totalCycles <= 0 || initialQueuePerServer <= 0) {
        throw std::invalid_argument(
            "Cycles and initial queue multiplier must be positive.");
    }

    if (minProcessingTime <= 0 ||
        maxProcessingTime < minProcessingTime) {
        throw std::invalid_argument(
            "Invalid processing-time range.");
    }

    if (lowerQueuePerServer <= 0 ||
        upperQueuePerServer <= lowerQueuePerServer) {
        throw std::invalid_argument(
            "Invalid queue scaling thresholds.");
    }

    if (scaleCooldown < 0 || minimumServersPerBalancer < 1) {
        throw std::invalid_argument(
            "Invalid server-scaling configuration.");
    }

    if (initialServers < minimumServersPerBalancer * 2) {
        throw std::invalid_argument(
            "Initial servers cannot support both load balancers.");
    }

    if (newRequestProbability < 0.0 ||
        newRequestProbability > 1.0 ||
        blockedTrafficRate < 0.0 ||
        blockedTrafficRate > 1.0) {
        throw std::invalid_argument(
            "Probability values must be between 0.0 and 1.0.");
    }

    if (blockedPrefix.empty() || logFile.empty()) {
        throw std::invalid_argument(
            "Blocked prefix and log-file path cannot be empty.");
    }
}