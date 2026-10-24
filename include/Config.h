#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <string>

/**
 * @brief Stores configurable settings for the CloudForge simulation.
 *
 * Settings are loaded from a text file so queue behavior, scaling thresholds,
 * request generation, firewall testing, and logging can be changed without
 * recompiling the program.
 */
struct Config {
    int initialServers = 20;
    int totalCycles = 100000;
    int initialQueuePerServer = 100;

    int minProcessingTime = 5;
    int maxProcessingTime = 50;

    int lowerQueuePerServer = 50;
    int upperQueuePerServer = 80;
    int scaleCooldown = 100;
    int minimumServersPerBalancer = 1;

    double newRequestProbability = 0.20;
    double blockedTrafficRate = 0.05;

    std::uint32_t randomSeed = 412;
    bool verbose = false;

    std::string blockedPrefix = "203.0.113.";
    std::string logFile = "logs/cloudforge.log";

    /**
     * @brief Loads settings from a key-value configuration file.
     * @param fileName Configuration-file path.
     * @return Populated and validated configuration.
     */
    static Config loadFromFile(const std::string& fileName);

    /**
     * @brief Validates all configuration values.
     * @throws std::invalid_argument if any value is invalid.
     */
    void validate() const;
};

#endif