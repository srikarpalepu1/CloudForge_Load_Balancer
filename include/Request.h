#ifndef REQUEST_H
#define REQUEST_H

#include <cstdint>
#include <string>

/**
 * @brief Represents one request entering the CloudForge load balancer.
 *
 * Each request contains an incoming IP address, destination IP address,
 * required processing time, job type, arrival cycle, and unique ID.
 */
struct Request {
    std::uint64_t id = 0;       ///< Unique request identifier.
    std::string ipIn;           ///< IP address of the requester.
    std::string ipOut;          ///< Destination IP address.
    int processingTime = 0;     ///< Number of clock cycles required.
    char jobType = 'P';         ///< P for processing or S for streaming.
    int arrivalCycle = 0;       ///< Clock cycle when the request arrived.
};

#endif