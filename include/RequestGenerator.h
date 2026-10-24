#ifndef REQUEST_GENERATOR_H
#define REQUEST_GENERATOR_H

#include "Request.h"

#include <cstdint>
#include <random>
#include <string>

/**
 * @brief Generates randomized requests for the load-balancer simulation.
 *
 * The generator creates source and destination IP addresses, processing
 * times, job types, and occasional blocked-range traffic for firewall tests.
 */
class RequestGenerator {
public:
    /**
     * @brief Constructs a configurable request generator.
     * @param minProcessingTime Minimum request duration in clock cycles.
     * @param maxProcessingTime Maximum request duration in clock cycles.
     * @param blockedTrafficRate Probability of generating a blocked source IP.
     * @param seed Seed used for reproducible random generation.
     */
    RequestGenerator(int minProcessingTime,
                     int maxProcessingTime,
                     double blockedTrafficRate,
                     std::uint32_t seed);

    /**
     * @brief Creates one randomized request.
     * @param id Unique request identifier.
     * @param arrivalCycle Current simulation clock cycle.
     * @return A populated Request structure.
     */
    Request generate(std::uint64_t id, int arrivalCycle);

    /**
     * @brief Determines whether a new request should arrive.
     * @param probability Probability from 0.0 through 1.0.
     * @return True when a request should be generated.
     */
    bool shouldGenerate(double probability);

private:
    std::string generateIPAddress();
    std::string generateBlockedIPAddress();

    int minProcessingTime_;
    int maxProcessingTime_;
    double blockedTrafficRate_;
    std::mt19937 randomEngine_;
};

#endif